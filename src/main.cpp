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

void print_query(Query query, int level) {
  string key;
  if (query.key != NULL)
    key = *query.key;

  print_spaces(level);
  if (query.children == NULL) {
    cout << key << '\n';
    return;
  }

  cout << key << " {\n";
  for (auto child : *query.children) {
    print_query(*child, level + 1);
  }
  print_spaces(level);
  cout << "}" << endl;
}

json filter(Query query, json data, string path) {
  json local_data;

  if (query.children == NULL) {
    return data;
  }

  if (data.is_array()) {
    for (auto element : data) {
      local_data.push_back(filter(query, element, path));
    }
  } else if (data.is_object()) {
    for (auto child : *query.children) {
      string target_key = *child->key;
      if (!data.contains(target_key)) {
        cerr << "Error: could not find key '" << target_key << "' in path '"
             << path << "' of json file" << endl;
        exit(1);
      }
      local_data[target_key] =
          filter(*child, data[target_key], path + "." + target_key);
    }
  } else {
    // if data is not an array or an object AND has children in the query, its
    // an error
    cerr << "Error: could not find key '" << *query.key << "' in path '" << path
         << "' of json file" << endl;
    exit(1)
  }
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
        exit(1)
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

  json result = filter(query, data, "root");
  cout << result.dump(2) << endl;

  return 0;
}
