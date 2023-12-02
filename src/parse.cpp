#include "parse.h"

#include <exception>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <span>
#include <sstream>
#include <stack>

Token expect(std::span<Token> &input, Token::TokenType type)
{
	if (input.empty()) {
		std::cerr << fmt::format("Expected token of type {}. Got empty sequence\n", static_cast<int>(type));
		std::terminate();
	}

	if (input.front().type != type) {
		std::cerr << fmt::format("Expected token of type {} got type {} with content {}\n", static_cast<int>(type),
								 static_cast<int>(input.front().type), input.front().content);
		std::terminate();
	}

	const auto token = input.front();
	input            = input.subspan(1);
	return token;
}

std::optional<Module> load_file(std::string_view filename)
{
	auto module      = new_module();
	module->filename = filename;
	{
		std::ifstream ifs;
		ifs.open(module->filename, std::ios_base::in);
		if (!ifs.is_open()) {
			std::cerr << fmt::format("Could not open file {}\n", filename);
			return std::nullopt;
		}

		std::ostringstream oss;
		oss << ifs.rdbuf();
		module->buffer = oss.str();
	}

	auto        tokens           = lex(module->buffer);
	std::size_t remaining_tokens = tokens.size();
	std::span   token_view       = tokens;

	while (remaining_tokens > 0) {
		token_view                    = token_view.last(remaining_tokens);
		auto proc                     = parse_procedure(token_view, &remaining_tokens);
		module->procedures[proc.name] = std::move(proc);
	}

	return module;
}

Procedure parse_procedure(std::span<Token> input, std::size_t *unconsomed_tokens)
{
	Procedure proc;

	expect(input, Token::Keyword_Proc);
	const auto ident = expect(input, Token::Identifier);
	proc.name        = ident.content;
	expect(input, Token::Keyword_Do);

	std::stack<std::size_t> jump_stack;

	while (!input.empty()) {
		const auto &token = input.front();
		if (token.type == Token::Keyword_End && jump_stack.empty()) {
			break;
		}
		auto &inst               = proc.body.emplace_back();
		inst.corresponding_token = token;
		const auto inst_idx      = size(proc.body) - 1;

		switch (token.type) {
		case Token::Keyword_While:
		case Token::Keyword_If: {
			jump_stack.push(inst_idx);
			break;
		}
		case Token::Keyword_Do: {
			if (jump_stack.empty()) {
				std::cerr << "Expected branching instruction preceding do instruction\n";
				std::terminate();
			}

			auto &branch_inst        = proc.body[jump_stack.top()];
			branch_inst.forward_jump = inst_idx;
			inst.backward_jump       = jump_stack.top();

			jump_stack.pop();
			jump_stack.push(inst_idx);
			break;
		}
		case Token::Keyword_End: {
			if (jump_stack.empty()) {
				std::cerr << "Expected do instruction preceding end instruction\n";
				std::terminate();
			}

			auto &branch_inst = proc.body[jump_stack.top()];
			if (branch_inst.corresponding_token.type != Token::Keyword_Do) {
				std::cerr << "Expected do token before end token\n";
				std::terminate();
			}
			branch_inst.forward_jump = inst_idx;
			inst.backward_jump       = branch_inst.backward_jump;

			jump_stack.pop();
			break;
		}
		default:
			break;
		}

		input = input.subspan(1);
	}

	if (!jump_stack.empty()) {
		std::cerr << "Unclosed braching instructions\n";
		std::terminate();
	}

	expect(input, Token::Keyword_End);

	if (unconsomed_tokens) {
		*unconsomed_tokens = input.size();
	}

	return proc;
}
