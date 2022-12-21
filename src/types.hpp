#include <string>
#include <vector>
using namespace std;

#ifndef __TYPES
#define __TYPES

#define REDC string("\033[1;31m")
#define YELLOWC string("\033[1;33m")
#define GREENC string("\033[1;32m")
#define RESETC string("\033[0;0m")
#define RESETBOLDC string("\033[1;1m")
#define CYANC string("\033[1;36m")

#define RED(A) REDC + A + RESETC
#define YELLOW(A) YELLOWC + A + RESETC
#define GREEN(A) GREENC + A + RESETC
#define RESETBOLD(A) RESETBOLDC + A + RESETC
#define CYAN(A) CYANC + A + RESETC

enum ArgumentType { STRING, INT, FLOAT, BOOL, NULLT };

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
    bool b;
    void* n;
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
