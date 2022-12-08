#include <string>
#include <vector>
using namespace std;

#ifndef __TYPES
#define __TYPES
typedef struct Query
{
	string *key;
	vector<Query *> *children;
} Query;

#endif