#include "types.hpp"

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