#pragma once

#include "lex.h"

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <variant>

struct Instruction {
	Token       corresponding_token;
	std::size_t forward_jump  = 0;
	std::size_t backward_jump = 0;
};

struct Procedure {
	std::string_view         name;
	std::vector<Instruction> body;
};

struct ModuleData {
	std::string                                     filename;
	std::string                                     buffer;
	std::unordered_map<std::string_view, Procedure> procedures;
};

using Module = std::unique_ptr<ModuleData>;
inline Module new_module()
{
	return std::make_unique<ModuleData>();
}

Procedure             parse_procedure(std::span<Token> input, std::size_t *unconsomed_tokens = nullptr);
std::optional<Module> load_file(std::string_view filename);
