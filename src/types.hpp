#include <string>
#include <vector>
using namespace std;

#ifndef __TYPES
#define __TYPES

#define RED string("\033[1;31m")
#define ORANGE string("\033[1;33m")
#define GREEN string("\033[1;32m")
#define RESET string("\033[0;0m")
#define RESETBOLD string("\033[1;1m")
#define CYAN string("\033[1;36m")

enum ArgumentType { STRING, INT, FLOAT };

typedef struct Location {
  int line;
  int col;
} Location;

typedef struct ArgumentValue {
  ArgumentType type;
  union {
    string* str;
    int i;
    float f;
  } v;
} ArgumentValue;

typedef struct KeyInfo {
  string* name;
  Location at;
} KeyInfo;

typedef struct Argument {
  KeyInfo info;
  ArgumentValue value;
} Argument;

typedef struct QueryKey {
  KeyInfo info;
  vector<Argument>* args;
} QueryField;

typedef struct Query {
  QueryKey query_key;
  vector<Query>* children;
} Query;

#endif
