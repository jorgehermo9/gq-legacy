#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include "filter.hpp"
#include "gq.tab.h"
#include "lib/argparse.hpp"
#include "lib/json.hpp"
#include "types.hpp"

using namespace std;
using json = nlohmann::json;

extern "C" int yylex();

bool quiet = false;

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
  program.add_argument("-o", "--output")
      .help("output file for filtered json")
      .default_value(string(""));
  program.add_argument("-q", "--quiet")
      .help("quiet mode (supress warnings)")
      .default_value(false)
      .implicit_value(true);


  try {
    program.parse_args(argc, argv);
  } catch (const runtime_error& err) {
    cerr << err.what() << endl;
    cerr << program;
    exit(1);
  }

  quiet = program.get<bool>("quiet");

  json data;
  string query_file = program.get<string>("query");
  string json_file = program.get<string>("json");
  string url = program.get<string>("url");

  if (json_file == "" && url == "") {
    // read json data from stdin
    data = json::parse(std::cin);
  } else if (json_file != "" && url != "") {
    cerr << RED("Error:") << " cannot use both url and json file" << endl;
    exit(1);
  } else if (json_file != "") {
    // read json data from file
    ifstream f(json_file);
    if (!f.is_open()) {
      cerr << RED("Error:") << " json file " << CYAN(json_file) << " not found"
           << endl;
      exit(1);
    }
    try {
      data = json::parse(f);
    } catch (json::parse_error& e) {
      cerr << RED("Error:") << " json file " << CYAN(json_file)
           << " is not valid, reason: " << e.what() << endl;
      exit(1);
    }
  } else if (url != "") {
    try {
      data = get_json_url(url);
    } catch (json::parse_error& e) {
      cerr << RED("Error:") << " json from url " << CYAN(url)
           << " is not valid, reason: " << e.what() << endl;
      exit(1);
    }
  }

  yyin = fopen(query_file.c_str(), "r");
  if (yyin == NULL) {
    cerr << RED("Error:") << " query file " << CYAN(query_file) << " not found"
         << endl;
    exit(1);
  }

  // Call to parser
  yyparse(&query);
  fclose(yyin);

  json result = filter(query, data);

  // If user specifies output, write to file, otherwise print to stdout
  if (program.get<string>("output") != "") {
    ofstream f(program.get<string>("output"));
    f << result.dump(2) << endl;
    f.close();
  } else {
    cout << result.dump(2) << endl;
  }

  return 0;
}
