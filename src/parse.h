#pragma once

#include "lex.h"

#include <variant>
#include <span>

struct Instruction {
	Token       corresponding_token;
	std::size_t forward_jump  = 0;
	std::size_t backward_jump = 0;
};

struct Procedure {
	std::string_view         name;
	std::vector<Instruction> body;
};

Procedure parse_procedure(std::span<Token> input);
