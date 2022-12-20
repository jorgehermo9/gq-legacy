#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include "gq.tab.h"
#include "lib/argparse.hpp"
#include "lib/json.hpp"

using namespace std;
using json = nlohmann::json;

extern "C" int yylex();

void print_spaces(int level) {
  for (int i = 0; i < level; i++) {
    cout << "  ";
  }
}

void do_print_query(Query query, int level) {
  string key;
  if (query.query_key.info.name != NULL)
    key = *query.query_key.info.name;

  print_spaces(level);
  if (query.children == NULL) {
    cout << key << '\n';
    return;
  }

  cout << key << " {\n";
  for (auto child : *query.children) {
    do_print_query(child, level + 1);
  }
  print_spaces(level);
  cout << "}" << endl;
}

void print_query(Query query) {
  do_print_query(query, 0);
}

void print_error(Query query, string error_message) {
  if (query.query_key.info.name == NULL) {  // Root query
    cerr << RED << "Error " << RESET << "in root query declared at " << RED
         << query.query_key.info.at.line << ":" << query.query_key.info.at.col
         << RESET << ": " << error_message << endl;
    exit(1);
  }

  cerr << RED << "Error " << RESET << "in query " << CYAN
       << *query.query_key.info.name << RESET << " declared at " << RED
       << query.query_key.info.at.line << ":" << query.query_key.info.at.col
       << RESET << ": " << error_message << endl;
  exit(1);
}

void print_warning(Query query, string warning_message) {
  if (query.query_key.info.name == NULL) {  // Root query
    cerr << ORANGE << "Warning " << RESET << "in root query declared at "
         << ORANGE << query.query_key.info.at.line << ":"
         << query.query_key.info.at.col << RESET << ": " << warning_message
         << endl;
    return;
  }

  cerr << ORANGE << "Warning " << RESET << "in query " << CYAN
       << *query.query_key.info.name << RESET << " declared at " << ORANGE
       << query.query_key.info.at.line << ":" << query.query_key.info.at.col
       << RESET << ": " << warning_message << endl;
}

bool fulfill_arguments(json element, Query parent_query, string path) {
  vector<Argument>* args = parent_query.query_key.args;
  bool result = true;

  if (args == NULL) {
    return true;
  }
  for (Argument arg : *args) {
    string key = *arg.info.name;
    if (!element.contains(key)) {
      print_warning(parent_query, "Query argument " + CYAN + key + RESET +
                                      " not found in element at " + CYAN +
                                      path + RESET + "");
      result = false;
      continue;
    }

    // TODO: Check types
    switch (arg.value.type) {
      case STRING:
        if (element[key] != *arg.value.v.str) {
          result = false;
        }
        break;
      case INT:
        if (element[key] != arg.value.v.i) {
          result = false;
        }
        break;
      case FLOAT:
        if (element[key] != arg.value.v.f) {
          result = false;
        }
        break;
    }
  }
  return result;
}

json filter(Query query, json data, string path) {
  json local_data;

  // Use local_path for error printing, handle special case of error
  // in root query
  string local_path = path;
  if (query.query_key.info.name == NULL) {
    local_path = ".";
  }

  // Return all fields only if data is an object
  if (query.children == NULL && !data.is_array()) {
    return data;
  }

  if (data.is_array()) {
    local_data = json::array();

    int acc = 0;
    for (auto element : data) {
      string element_path = local_path + "[" + to_string(acc) + "]";
      acc++;

      if (!fulfill_arguments(element, query, element_path)) {
        continue;
      }

      local_data.push_back(filter(query, element, element_path));
    }

  } else if (data.is_object()) {
    local_data = json::object();

    for (auto child : *query.children) {
      string target_key = *child.query_key.info.name;
      if (!data.contains(target_key)) {
        string error_message =
            "Could not find key " + target_key + " in " + local_path + "";
        print_error(query, error_message);
      }
      local_data[target_key] =
          filter(child, data[target_key], path + "." + target_key);
    }
  } else {
    // if data is not an array or an object AND has children in the query, its
    // an error
    string error_message =
        "Query specifies fields, but " + local_path + " is not an object";
    print_error(query, error_message);
  }

  // If none could match, return empty object
  return local_data;
}

// For curl
size_t callback(const char* in, size_t size, size_t num, string* out) {
  const size_t totalBytes(size * num);
  out->append(in, totalBytes);
  return totalBytes;
}

json get_json_url(string url) {
  CURL* curl = curl_easy_init();

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  // use pointers to pass data to callback
  unique_ptr<int> httpCode(new int(404));
  unique_ptr<string> httpData(new string());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());
  curl_easy_perform(curl);
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, httpCode.get());

  curl_easy_cleanup(curl);

  if (*httpCode != 200) {
    cerr << "Error: Could not get data from " << url
         << ", response code: " << *httpCode << endl;
    exit(1);
  }
  return json::parse(*httpData);
}

int main(int argc, char* argv[]) {
  Query query;
  extern FILE* yyin;

  argparse::ArgumentParser program("gq");

  program.add_argument("-j", "--json")
      .help("json file to query")
      .default_value(string(""));
  program.add_argument("-u", "--url")
      .help("url to query")
      .default_value(string(""));
  program.add_argument("query").help("query to run on json file");

  try {
    program.parse_args(argc, argv);
  } catch (const runtime_error& err) {
    cerr << err.what() << endl;
    cerr << program;
    exit(1);
  }

  json data;
  string query_file = program.get<string>("query");
  string json_file = program.get<string>("json");
  string url = program.get<string>("url");

  if (json_file == "" && url == "") {
    // read json data from stdin
    data = json::parse(std::cin);
  } else if (json_file != "" && url != "") {
    cerr << "Error: Cannot use both url and json file" << endl;
    exit(1);
  } else if (json_file != "") {
    // read json data from file
    ifstream f(json_file);
    data = json::parse(f);
  } else if (url != "") {
    data = get_json_url(url);
  }

  yyin = fopen(query_file.c_str(), "r");
  if (yyin == NULL) {
    cerr << "Error: query file " << query_file << " not found" << endl;
    exit(1);
  }
  yyparse(&query);
  fclose(yyin);
  // use root at starting path, for error displaying...
  json result = filter(query, data, "");
  cout << result.dump(2) << endl;

  return 0;
}
