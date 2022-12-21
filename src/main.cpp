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

string get_query_name(Query query) {
  if (query.query_key.info.name != NULL) {
    return "query " + CYAN("'" + *query.query_key.info.name + "'");
  } else {
    return CYAN("root query");
  }
}

void print_error(Query query, string error_message) {
  string query_name = get_query_name(query);
  cerr << RED("Error") << " in " << query_name << " declared at "
       << RED(to_string(query.query_key.info.at.line) + ":" +
              to_string(query.query_key.info.at.col))
       << ": " << error_message << endl;
  exit(1);
}

void print_warning(Query query, string warning_message) {
  string query_name = get_query_name(query);
  cerr << YELLOW("Warning") << " in " << query_name << " declared at "
       << YELLOW(to_string(query.query_key.info.at.line) + ":" +
                 to_string(query.query_key.info.at.col))
       << ": " << warning_message << endl;
}

string string_of_type(ArgumentType type) {
  switch (type) {
    case STRING:
      return "string";
    case INT:
      return "number";
    case FLOAT:
      return "number";
    case BOOL:
      return "boolean";
    case NULLT:
      return "null";
    default:
      return "unknown";
  }
}

string string_of_operation(ArgumentOperation op) {
  switch (op) {
    case CONTAINS_OP:
      return "contains";
    case NOT_CONTAINS_OP:
      return "not contains";
    case STARTS_WITH_OP:
      return "starts with";
    case NOT_STARTS_WITH_OP:
      return "not starts with";
    case LE_OP:
      return "<=";
    case LT_OP:
      return "<";
    case GE_OP:
      return ">=";
    case GT_OP:
      return ">";
    case EQ_OP:
      return "=";
    case NE_OP:
      return "!=";
    default:
      return "unknown";
  }
}

string string_type_of_json(json element) {
  if (element.is_string()) {
    return "string";
  } else if (element.is_number()) {
    return "number";
  } else if (element.is_boolean()) {
    return "boolean";
  } else if (element.is_null()) {
    return "null";
  } else if (element.is_object()) {
    return "object";
  } else if (element.is_array()) {
    return "array";
  } else {
    return "unknown";
  }
}

string get_type_error_message(ArgumentType type,
                              json field,
                              string path,
                              string key) {
  string type_str = string_of_type(type);
  string type_json = string_type_of_json(field);
  return "query argument " + CYAN(key) + " is of type " + PURPLE(type_str) +
         " but " + CYAN(path + "." + key) + " is of type " + PURPLE(type_json) +
         "; not including item";
}

string get_operation_error_message(Argument arg) {
  string op_str = string_of_operation(arg.operation);
  string type_str = string_of_type(arg.value.type);
  return "operation " + PURPLE(op_str) + " not supported in type " +
         PURPLE(type_str) + " in query argument " + CYAN(*arg.info.name);
}

bool satisfies_operation_string(Query parent_query,
                                string field,
                                Argument arg) {
  string value = *arg.value.v.str;
  string key = *arg.info.name;
  switch (arg.operation) {
    case CONTAINS_OP:
      return field.find(value) != string::npos;
    case NOT_CONTAINS_OP:
      return field.find(value) == string::npos;
    case STARTS_WITH_OP:
      return field.find(value) == 0;
    case NOT_STARTS_WITH_OP:
      return field.find(value) != 0;
    case EQ_OP:
      return field == value;
    case NE_OP:
      return field != value;
    default:
      print_error(parent_query, get_operation_error_message(arg));
      return false;
  }
}

bool satisfies_operation_int(Query parent_query, json field, Argument arg) {
  int value = arg.value.v.i;
  string key = *arg.info.name;
  switch (arg.operation) {
    case EQ_OP:
      return field == value;
    case NE_OP:
      return field != value;
    case GT_OP:
      return field > value;
    case GE_OP:
      return field >= value;
    case LT_OP:
      return field < value;
    case LE_OP:
      return field <= value;
    default:
      print_error(parent_query, get_operation_error_message(arg));
      return false;
  }
}

bool satisfies_operation_float(Query parent_query, json field, Argument arg) {
  float value = arg.value.v.f;
  string key = *arg.info.name;
  switch (arg.operation) {
    case EQ_OP:
      return field == value;
    case NE_OP:
      return field != value;
    case GT_OP:
      return field > value;
    case GE_OP:
      return field >= value;
    case LT_OP:
      return field < value;
    case LE_OP:
      return field <= value;
    default:
      print_error(parent_query, get_operation_error_message(arg));
      return false;
  }
}

bool satisfies_operation_bool(Query parent_query, bool field, Argument arg) {
  bool value = arg.value.v.b;
  string key = *arg.info.name;
  switch (arg.operation) {
    case EQ_OP:
      return field == value;
    case NE_OP:
      return field != value;
    default:
      print_error(parent_query, get_operation_error_message(arg));
      return false;
  }
}

bool satisfies_operation_null(Query parent_query, json field, Argument arg) {
  string key = *arg.info.name;
  switch (arg.operation) {
    case EQ_OP:
      return field.is_null();
    case NE_OP:
      return !field.is_null();
    default:
      print_error(parent_query, get_operation_error_message(arg));
      return false;
  }
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
      print_warning(parent_query, "query argument " + CYAN(key) +
                                      " not found in element at " + CYAN(path) +
                                      "; not including item");
      result = false;
      continue;
    }

    json field = element[key];
    switch (arg.value.type) {
      case STRING:
        if (!field.is_string()) {
          string error_message =
              get_type_error_message(arg.value.type, field, path, key);
          print_warning(parent_query, error_message);
          result = false;
          continue;
        }
        if (!satisfies_operation_string(parent_query, field, arg)) {
          result = false;
        }
        break;
      case INT:
        if (!field.is_number()) {
          string error_message =
              get_type_error_message(arg.value.type, field, path, key);
          print_warning(parent_query, error_message);
          result = false;
          continue;
        }
        if (!satisfies_operation_int(parent_query, field, arg)) {
          result = false;
        }
        break;
      case FLOAT:
        if (!field.is_number()) {
          string error_message =
              get_type_error_message(arg.value.type, field, path, key);
          print_warning(parent_query, error_message);
          result = false;
          continue;
        }
        if (!satisfies_operation_float(parent_query, field, arg)) {
          result = false;
        }
        break;
      case BOOL:
        if (!field.is_boolean()) {
          string error_message =
              get_type_error_message(arg.value.type, field, path, key);
          print_warning(parent_query, error_message);
          result = false;
          continue;
        }
        if (!satisfies_operation_bool(parent_query, field, arg)) {
          result = false;
        }
        break;
      case NULLT:
        if (!satisfies_operation_null(parent_query, field, arg)) {
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

  if (!data.is_array() && query.query_key.args != NULL) {
    print_error(query, "query arguments are only allowed in array data");
  }

  // Return all fields only if data is an object, cause arrays must be filtered
  if (query.children == NULL && !data.is_array()) {
    return data;
  }

  if (data.is_array()) {
    local_data = json::array();
    // Discard arguments in children
    Query children_query = query;
    children_query.query_key.args = NULL;

    int acc = 0;
    for (auto element : data) {
      string element_path = local_path + "[" + to_string(acc) + "]";
      acc++;

      // In arrays pass the arguments to its children if they are also arrays
      if (!element.is_array() &&
          !fulfill_arguments(element, query, element_path)) {
        continue;
      }

      Query element_query = element.is_array() ? query : children_query;
      local_data.push_back(filter(element_query, element, element_path));
    }

  } else if (data.is_object()) {
    local_data = json::object();

    for (auto child : *query.children) {
      string target_key = *child.query_key.info.name;
      string alias_key = *child.query_key.alias;
      if (!data.contains(target_key)) {
        string error_message = "Could not find key " + CYAN(target_key) +
                               " in " + CYAN(local_path);
        print_error(query, error_message);
      }

      if (local_data.contains(alias_key)) {
        string error_message = "duplicated key " + CYAN(alias_key);
        print_error(query, error_message);
      }

      local_data[alias_key] =
          filter(child, data[target_key], path + "." + target_key);
    }
  } else {
    // if data is not an array or an object AND has children in the query, its
    // an error
    string error_message =
        "Query specifies fields, but " + CYAN(local_path) + " is not an object";
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
    if (!f.is_open()) {
      cerr << "Error: json file " << json_file << " not found" << endl;
      exit(1);
    }
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
