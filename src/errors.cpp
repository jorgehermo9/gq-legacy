#include "errors.hpp"

using namespace std;

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

string get_type_error_message(Type type, json field, string path, string key) {
  string type_str = string_of_type(type);
  string type_json = string_type_of_json(field);
  return "query argument " + CYAN(key) + " is of type " + PURPLE(type_str) +
         " but " + CYAN(path) + " is of type " + PURPLE(type_json) +
         "; not including item";
}

string get_operation_error_message(Argument arg) {
  string op_str = string_of_operation(arg.operation.op);
  string type_str = string_of_type(arg.value.type);
  string key_path = join_key_path(*arg.key_path);
  return "operation " + PURPLE(op_str) + " not supported in type " +
         PURPLE(type_str) + " in query argument " + CYAN(key_path);
}

string get_modifier_error_message(Argument arg) {
  string mod_str = string_of_modifier(arg.operation.modifier);
  string type_str = string_of_type(arg.value.type);
  string key_path = join_key_path(*arg.key_path);
  return "modifier " + PURPLE(mod_str) + " not supported with type " +
         PURPLE(type_str) + " in query argument " + CYAN(key_path);
}
