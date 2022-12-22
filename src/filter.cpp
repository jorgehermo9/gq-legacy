#include "filter.hpp"
#include "errors.hpp"
#include "lib/json.hpp"

using namespace std;
using json = nlohmann::json;

bool satisfies_operation_string(Query parent_query,
                                string field,
                                Argument arg) {
  string value = *arg.value.v.str;
  string key = *arg.info.name;

  if (arg.operation.modifier == CASE_INSENSITIVE) {
    transform(field.begin(), field.end(), field.begin(), ::tolower);
    transform(value.begin(), value.end(), value.begin(), ::tolower);
  }
  switch (arg.operation.op) {
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
  if (arg.operation.modifier == CASE_INSENSITIVE) {
    print_error(parent_query, get_modifier_error_message(arg));
  }
  int value = arg.value.v.i;
  string key = *arg.info.name;
  switch (arg.operation.op) {
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
  if (arg.operation.modifier == CASE_INSENSITIVE) {
    print_error(parent_query, get_modifier_error_message(arg));
  }
  float value = arg.value.v.f;
  string key = *arg.info.name;
  switch (arg.operation.op) {
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
  if (arg.operation.modifier == CASE_INSENSITIVE) {
    print_error(parent_query, get_modifier_error_message(arg));
  }
  bool value = arg.value.v.b;
  string key = *arg.info.name;
  switch (arg.operation.op) {
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
  if (arg.operation.modifier == CASE_INSENSITIVE) {
    print_error(parent_query, get_modifier_error_message(arg));
  }
  string key = *arg.info.name;
  switch (arg.operation.op) {
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

json do_filter(Query query, json data, string path) {
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
      local_data.push_back(do_filter(element_query, element, element_path));
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
          do_filter(child, data[target_key], path + "." + target_key);
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

json filter(Query query, json data) {
  return do_filter(query, data, "");
}