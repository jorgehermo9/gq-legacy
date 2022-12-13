#include <fstream>
#include <iostream>
#include "argparse.hpp"
#include "gq.tab.h"
#include "json.hpp"
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
  if (query.key_info.name != NULL)
    key = *query.key_info.name;

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
  if (query.key_info.name == NULL) {  // Root query
    cerr << RED << "Error " << RESET << "in root query declared at " << RED
         << query.key_info.declared_line << ":" << query.key_info.declared_col
         << RESET << ": " << error_message << endl;
    exit(1);
  }

  cerr << RED << "Error " << RESET << "in query '" << CYAN
       << *query.key_info.name << RESET << "' declared at " << RED
       << query.key_info.declared_line << ":" << query.key_info.declared_col
       << RESET << ": " << error_message << endl;
  exit(1);
}
json filter(Query query, json data, string path) {
  json local_data;

  // Use local_path for error printing, handle special case of error
  // in root query
  string local_path = path;
  if (query.key_info.name == NULL) {
    local_path = ".";
  }

  if (query.children == NULL) {
    return data;
  }

  if (data.is_array()) {
    local_data = json::array();

    int acc = 0;
    for (auto element : data) {
      local_data.push_back(
          filter(query, element, path + "[" + to_string(acc) + "]"));
      acc++;
    }

  } else if (data.is_object()) {
    local_data = json::object();

    for (auto child : *query.children) {
      string target_key = *child.key_info.name;
      if (!data.contains(target_key)) {
        string error_message =
            "Could not find key '" + target_key + "' in '" + local_path + "'";
        print_error(query, error_message);
      }
      local_data[target_key] =
          filter(child, data[target_key], path + "." + target_key);
    }
  } else {
    // if data is not an array or an object AND has children in the query, its
    // an error
    string error_message =
        "Query specifies fields, but '" + local_path + "' is not an object";
    print_error(query, error_message);
  }

  // If none could match, return empty object
  return local_data;
}

int main(int argc, char* argv[]) {
  Query query;
  extern FILE* yyin;

  switch (argc) {
    case 1:
      yyin = stdin;
      yyparse(&query);
      break;
    case 2:
      yyin = fopen(argv[1], "r");
      if (yyin == NULL) {
        cerr << "Error: File not found" << endl;
        exit(1);
      }
      yyparse(&query);
      fclose(yyin);
      break;
    default:
      cerr << "Error: Too many arguments. Syntax: " << argv[0] << "input_file"
           << endl;
      exit(1);
  }

  // 	argparse::ArgumentParser program("gq");

  //   program.add_argument("square")
  //     .help("display the square of a given integer")
  //     .scan<'i', int>();

  //   try {
  //     program.parse_args(argc, argv);
  //   }
  //   catch (const std::runtime_error& err) {
  //     std::cerr << err.what() << std::endl;
  //     std::cerr << program;
  //     std::exit(1);
  //   }

  //   auto input = program.get<int>("square");
  //   std::cout << (input * input) << std::endl;

  // print query
  // print_query(query, 0);

  std::ifstream f("example.json");
  json data = json::parse(f);

  // use root at starting path, for error displaying...
  json result = filter(query, data, "");
  cout << result.dump(2) << endl;

  return 0;
}
