#include "types.hpp"

string string_of_type(Type type) {
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

string string_of_operation(Operation op) {
  switch (op) {
    case CONTAINS_OP:
      return "contains";
    case NOT_CONTAINS_OP:
      return "not contains";
    case STARTS_WITH_OP:
      return "starts with";
    case NOT_STARTS_WITH_OP:
      return "not starts with";
    case ENDS_WITH_OP:
      return "ends with";
    case NOT_ENDS_WITH_OP:
      return "not ends with";
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

string string_of_modifier(OperationModifier mod) {
  switch (mod) {
    case NONE:
      return "no modifier";
    case CASE_INSENSITIVE:
      return "case insensitive";
    default:
      return "unknown";
  }
}

string join_key_path(stack<KeyInfo> key_path) {
  if (key_path.empty()) {
    return ".";
  }

  KeyInfo first_key_info = key_path.top();
  key_path.pop();
  string key_path_str = *first_key_info.name;
  while (!key_path.empty()) {
    KeyInfo key_info = key_path.top();
    key_path.pop();
    key_path_str = key_path_str + "." + *key_info.name;
  }
  return key_path_str;
}
