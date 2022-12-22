#include <iostream>
#include <string>
#include "lib/json.hpp"
#include "types.hpp"
using namespace std;
using json = nlohmann::json;

void print_error(Query query, string error_message);
void print_warning(Query query, string warning_message);
string get_type_error_message(Type type, json field, string path, string key);
string get_operation_error_message(Argument arg);
string get_modifier_error_message(Argument arg);