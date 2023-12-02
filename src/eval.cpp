#include "eval.h"

#include <charconv>
#include <exception>
#include <fmt/format.h>
#include <iostream>

namespace {

template<class... Ts>
struct overloaded : Ts... {
	using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

void expect_stack_size(const Token &token, Environment &env, std::size_t size)
{
	if (env.stack.size() < size) {
		std::cerr << fmt::format("{}:{} Expected stack size of at least {} but was {}\n", token.line, token.col, size,
								 env.stack.size());
		std::terminate();
	}
}

template<typename... T>
bool check_permissible_types(const Value &value, const Token &token, bool fail = true)
{
	const bool passes = (std::holds_alternative<T>(value) || ...);
	if (!passes && fail) {
		std::cerr << fmt::format("{}:{} Value holds invalid type for operation. Got {}", token.line, token.col,
								 value.index());
		std::terminate();
	}

	return passes;
}

bool eval_ops(const Token &token, Environment &env)
{
	switch (token.type) {
	case Token::Op_Add: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(overloaded{ [&](const double &a, const double &b) { env.stack.emplace_back(a + b); },
							   [&](const int &a, const int &b) { env.stack.emplace_back(a + b); },
							   [&](const double &a, const int &b) { env.stack.emplace_back(a + b); },
							   [&](const int &a, const double &b) { env.stack.emplace_back(a + b); },
							   [&](const char *a, const int &b) { env.stack.emplace_back(a + b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line,
															token.col);
								   std::terminate();
							   } },
				   first, second);

	} break;
	case Token::Op_Sub: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(overloaded{ [&](const double &a, const double &b) { env.stack.emplace_back(a - b); },
							   [&](const int &a, const int &b) { env.stack.emplace_back(a - b); },
							   [&](const double &a, const int &b) { env.stack.emplace_back(a - b); },
							   [&](const int &a, const double &b) { env.stack.emplace_back(a - b); },
							   [&](const char *a, const int &b) { env.stack.emplace_back(a - b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line,
															token.col);
								   std::terminate();
							   } },
				   first, second);

	} break;
	case Token::Op_Mul: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(overloaded{ [&](const double &a, const double &b) { env.stack.emplace_back(a * b); },
							   [&](const int &a, const int &b) { env.stack.emplace_back(a * b); },
							   [&](const double &a, const int &b) { env.stack.emplace_back(a * b); },
							   [&](const int &a, const double &b) { env.stack.emplace_back(a * b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line,
															token.col);
								   std::terminate();
							   } },
				   first, second);

	} break;
	case Token::Op_Div: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(overloaded{ [&](const double &a, const double &b) { env.stack.emplace_back(a / b); },
							   [&](const int &a, const int &b) { env.stack.emplace_back(a / b); },
							   [&](const double &a, const int &b) { env.stack.emplace_back(a / b); },
							   [&](const int &a, const double &b) { env.stack.emplace_back(a / b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line,
															token.col);
								   std::terminate();
							   } },
				   first, second);

	} break;
	case Token::Op_LT: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(overloaded{ [&](const double &a, const double &b) { env.stack.emplace_back(a < b); },
							   [&](const int &a, const int &b) { env.stack.emplace_back(a < b); },
							   [&](const double &a, const int &b) { env.stack.emplace_back(a < b); },
							   [&](const int &a, const double &b) { env.stack.emplace_back(a < b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line,
															token.col);
								   std::terminate();
							   } },
				   first, second);

	} break;
	case Token::Op_LE: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(overloaded{ [&](const double &a, const double &b) { env.stack.emplace_back(a <= b); },
							   [&](const int &a, const int &b) { env.stack.emplace_back(a <= b); },
							   [&](const double &a, const int &b) { env.stack.emplace_back(a <= b); },
							   [&](const int &a, const double &b) { env.stack.emplace_back(a <= b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line,
															token.col);
								   std::terminate();
							   } },
				   first, second);

	} break;
	case Token::Op_GE: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(overloaded{ [&](const double &a, const double &b) { env.stack.emplace_back(a >= b); },
							   [&](const int &a, const int &b) { env.stack.emplace_back(a >= b); },
							   [&](const double &a, const int &b) { env.stack.emplace_back(a >= b); },
							   [&](const int &a, const double &b) { env.stack.emplace_back(a >= b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line,
															token.col);
								   std::terminate();
							   } },
				   first, second);

	} break;
	case Token::Op_GT: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(overloaded{ [&](const double &a, const double &b) { env.stack.emplace_back(a > b); },
							   [&](const int &a, const int &b) { env.stack.emplace_back(a > b); },
							   [&](const double &a, const int &b) { env.stack.emplace_back(a > b); },
							   [&](const int &a, const double &b) { env.stack.emplace_back(a > b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line,
															token.col);
								   std::terminate();
							   } },
				   first, second);

	} break;
	case Token::Op_EQ: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(
			[&](const auto &a, const auto &b) {
				if constexpr (std::equality_comparable_with<decltype(a), decltype(b)>) {
					return a == b;
				}
				std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line, token.col);
				std::terminate();
				return false;
			},
			first, second);

	} break;
	case Token::Op_NE: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(
			[&](const auto &a, const auto &b) {
				if constexpr (std::equality_comparable_with<decltype(a), decltype(b)>) {
					return !(a == b);
				}
				std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line, token.col);
				std::terminate();
				return true;
			},
			first, second);

	} break;
	case Token::Op_Store: {
		expect_stack_size(token, env, 2);
		const auto variable = env.stack.back();
		env.stack.pop_back();
		const auto value = env.stack.back();
		env.stack.pop_back();

		if (!std::holds_alternative<Identifier>(variable)) {
			std::cerr << fmt::format("{}:{} Tried to store in non variable type {}\n", token.line, token.col,
									 variable.index());
			std::terminate();
		}

		const auto &ident = std::get<Identifier>(variable);

		auto cellIt = env.variables.find(ident.content);
		if (cellIt == env.variables.end()) {
			std::cerr << fmt::format("{}:{} Tried to store undeclared variable\n", token.line, token.col);
			std::terminate();
		}
		cellIt->second = value;

	} break;
	case Token::Op_Load: {
		expect_stack_size(token, env, 1);
		const auto variable = env.stack.back();
		env.stack.pop_back();

		if (!std::holds_alternative<Identifier>(variable)) {
			std::cerr << fmt::format("{}:{} Tried to store in non variable type\n", token.line, token.col);
			std::terminate();
		}

		const auto &ident = std::get<Identifier>(variable);

		auto cellIt = env.variables.find(ident.content);
		if (cellIt == env.variables.end()) {
			std::cerr << fmt::format("{}:{} Tried to load undeclared variable\n", token.line, token.col);
			std::terminate();
		}
		env.stack.push_back(cellIt->second);
	} break;
	default:
		return false;
	}
	return true;
}

const Procedure *select_procedure(std::span<Module> modules, std::string_view name)
{
	for (auto it = modules.rbegin(); it != modules.rend(); ++it) {
		const auto procIt = (*it)->procedures.find(name);
		if (procIt != (*it)->procedures.end()) {
			return std::addressof(procIt->second);
		}
	}

	return nullptr;
}

}

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
			if (const auto *mproc = select_procedure(env.loaded_modules, token.content); mproc) {
				eval_proc(*mproc, env);
			} else if (token.content == "true") {
				env.stack.emplace_back(true);
			} else if (token.content == "false") {
				env.stack.emplace_back(false);
			} else if (token.content == "print") {
				const auto &v = env.stack.back();
				std::visit(overloaded{ [](const Identifier &a) { std::cout << "I:" << a.content << std::endl; },
									   [](const auto &a) { std::cout << a << std::endl; } },
						   v);
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
		case Token::Keyword_Rotate:
			if (env.stack.size() < 3) {
				std::cerr << fmt::format("{}:{} Expected atleasat two elements on the stack\n", token.line, token.col);
				std::terminate();
			}
			std::swap(*(env.stack.end() - 3), *(env.stack.end() - 1));
			std::swap(*(env.stack.end() - 1), *(env.stack.end() - 2));
			break;
		case Token::Keyword_Import: {
			if (env.stack.empty()) {
				std::cerr << fmt::format("{}:{} Expected atleasat two elements on the stack\n", token.line, token.col);
				std::terminate();
			}

			const auto file_value = env.stack.back();
			env.stack.pop_back();

			if (!std::holds_alternative<std::string_view>(file_value)) {
				std::cerr << fmt::format("{}:{} Expected string literal with filename\n", token.line, token.col);
				std::terminate();
			}

			const auto &filename = std::get<std::string_view>(file_value);

			bool foundMod = false;
			for (const auto &sp : env.search_paths) {
				auto mod = load_file(fmt::format("{}/{}", sp, filename));
				if (!mod.has_value()) {
					continue;
				}

				env.loaded_modules.emplace_back(std::move(*mod));
				foundMod = true;
				break;
			}

			if (!foundMod) {
				std::cerr << fmt::format("{}:{} Failed to load module {}\n", token.line, token.col, filename);
				std::terminate();
			}

			break;
		}
		case Token::Keyword_Swap:
			if (env.stack.size() < 2) {
				std::cerr << fmt::format("{}:{} Expected atleasat two elements on the stack\n", token.line, token.col);
				std::terminate();
			}
			std::swap(*(env.stack.end() - 1), *(env.stack.end() - 2));
			break;
		case Token::Keyword_Variable: {
			if (env.stack.empty()) {
				std::cerr << fmt::format("{}:{} Expected value on stack but empty\n", token.line, token.col);
				std::terminate();
			}

			const auto variable = env.stack.back();
			env.stack.pop_back();

			if (!std::holds_alternative<Identifier>(variable)) {
				std::cerr << fmt::format("{}:{} Tried to store in non variable type\n", token.line, token.col);
				std::terminate();
			}

			const auto &ident            = std::get<Identifier>(variable);
			env.variables[ident.content] = true;

			break;
		}

		default:
			if (!eval_ops(token, env)) {
				std::cerr << fmt::format("{}:{} Unexpected token recieved {}\n", token.line, token.col, token.type);
				std::terminate();
			}
			break;
		}
	}
}
