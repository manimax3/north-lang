#include "parse.h"

#include <exception>
#include <fmt/format.h>
#include <iostream>
#include <span>
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

Procedure parse_procedure(std::span<Token> input)
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

	return proc;
}
