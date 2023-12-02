#pragma once

#include "lex.h"
#include "parse.h"

#include <string_view>
#include <unordered_map>
#include <vector>
#include <variant>

struct Identifier : Token {};
using Value = std::variant<Identifier, std::string_view, double, int, char *, bool>;

struct Environment {
	std::unordered_map<std::string_view, Procedure> procedures;
	std::unordered_map<std::string_view, Value>     variables;
	std::vector<Value>                              stack;
};

void eval_proc(const Procedure &proc, Environment &env);
