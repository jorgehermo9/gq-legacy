#include <string>
#include <vector>
using namespace std;

#ifndef __TYPES
#define __TYPES

#define RED "\033[1;31m"
#define ORANGE "\033[1;33m"
#define GREEN "\033[1;32m"
#define RESET "\033[0;0m"
#define RESETBOLD "\033[1;1m"
#define CYAN "\033[1;36m"

typedef struct KeyInfo {
  string* name;
  int declared_line;
  int declared_col;
} KeyInfo;

typedef struct Query {
  KeyInfo key_info;
  vector<Query>* children;
} Query;

#endif
