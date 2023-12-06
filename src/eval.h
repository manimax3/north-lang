#pragma once

#include "lex.h"
#include "parse.h"

#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

struct Identifier : Token {};
using Integral = std::int64_t;
using Value    = std::variant<Identifier, std::string_view, double, Integral, char *, bool, void *>;

struct Environment {
	std::vector<Value>       stack;
	std::vector<Module>      loaded_modules;
	std::vector<std::string> search_paths;
};

void eval_proc(const Procedure &proc, Environment &env);
