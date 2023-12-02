#include "eval.h"

#include <charconv>
#include <exception>
#include <fmt/format.h>
#include <iostream>

void eval_proc(const Procedure &proc, Environment &env)
{
	for (std::size_t ic = 0; ic < proc.body.size(); ++ic) {
		const auto &inst  = proc.body.at(ic);
		const auto &token = inst.corresponding_token;
		switch (token.type) {
		case Token::Numeric: {
			double value = 0.;
			std::from_chars(token.content.data(), token.content.data() + token.content.size(), value);
			env.stack.emplace_back(value);
			break;
		}
		case Token::StringLiteral: {
			env.stack.emplace_back(token.content);
			break;
		}
		case Token::Identifier: {
			if (env.procedures.contains(token.content)) {
				eval_proc(env.procedures[token.content], env);
			} else if (token.content == "true") {
				env.stack.emplace_back(true);
			} else if (token.content == "false") {
				env.stack.emplace_back(false);
			} else if (token.content == "print") {
				const auto &v = env.stack.back();
				if (auto *sv = std::get_if<std::string_view>(&v); sv != nullptr) {
					std::cout << *sv << std::endl;
				}
				env.stack.erase(env.stack.end() - 1);
			} else {
				env.stack.emplace_back(Identifier{ token });
			}
			break;
		}
		case Token::Keyword_Do: {
			if (env.stack.empty()) {
				std::cerr << fmt::format("{}:{} Expected value on stack but empty\n", token.line, token.col);
				std::terminate();
			}

			const auto top = env.stack.back();
			env.stack.erase(env.stack.end() - 1);
			if (const bool *v = std::get_if<bool>(&top); v == nullptr) {
				std::cerr << fmt::format("{}:{} Expected boolean value on stack\n", token.line, token.col);
				std::terminate();
			} else if (!(*v)) {
				ic = inst.forward_jump;
				continue;
			}

			break;
		}
		case Token::Keyword_End: {
			const auto &branch_inst = proc.body.at(inst.backward_jump);

			switch (branch_inst.corresponding_token.type) {
			case Token::Keyword_If:
				break;
			case Token::Keyword_While:
				ic = inst.backward_jump;
				break;
			default:
				std::cerr << fmt::format("{}:{} Unexpected preceding branching instruction T:{}\n", token.line,
										 token.col, branch_inst.corresponding_token.type);
				std::terminate();
				break;
			}

			break;
		}
		case Token::Keyword_While:
		case Token::Keyword_If:
			break;

		case Token::Keyword_Dup:
			if (env.stack.empty()) {
				std::cerr << fmt::format("{}:{} Expected value on stack but empty\n", token.line, token.col);
				std::terminate();
			}
			env.stack.push_back(env.stack.back());
			break;
		case Token::Keyword_Drop:
			if (env.stack.empty()) {
				std::cerr << fmt::format("{}:{} Expected value on stack but empty\n", token.line, token.col);
				std::terminate();
			}
			env.stack.erase(env.stack.end() - 1);
			break;
		case Token::Keyword_Over:
			if (env.stack.size() < 2) {
				std::cerr << fmt::format("{}:{} Expected atleasat two elements on the stack\n", token.line, token.col);
				std::terminate();
			}
			env.stack.push_back(*(env.stack.end() - 2));
			break;
		case Token::Keyword_Swap:
			if (env.stack.size() < 2) {
				std::cerr << fmt::format("{}:{} Expected atleasat two elements on the stack\n", token.line, token.col);
				std::terminate();
			}
			std::swap(*(env.stack.end() - 1), *(env.stack.end() - 2));
			break;

		default:
			std::cerr << fmt::format("{}:{} Unexpected token recieved {}\n", token.line, token.col, token.type);
			std::terminate();
			break;
		}
	}
}
