#include <iostream>
#include <string>
#include "lib/json.hpp"
#include "types.hpp"

using namespace std;
using json = nlohmann::json;

json filter(Query query, json data);