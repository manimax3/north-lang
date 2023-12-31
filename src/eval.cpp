#include "eval.h"

#include <charconv>
#include <exception>
#include <fmt/format.h>
#include <iostream>

bool eval_builtin(std::string_view name, Environment &environ, const Token &token);

using VariablesMap = std::unordered_map<std::string_view, Value>;

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

bool eval_ops(const Token &token, Environment &env, VariablesMap &variables)
{
	switch (token.type) {
	case Token::Op_Add: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(overloaded{ [&](const double &a, const double &b) { env.stack.emplace_back(a + b); },
							   [&](const Integral &a, const Integral &b) { env.stack.emplace_back(a + b); },
							   [&](const double &a, const Integral &b) { env.stack.emplace_back(a + b); },
							   [&](const Integral &a, const double &b) { env.stack.emplace_back(a + b); },
							   [&](char *a, const Integral &b) { env.stack.emplace_back(a + b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op {} on stack\n",
															token.line, token.col, token.type);
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
							   [&](const Integral &a, const Integral &b) { env.stack.emplace_back(a - b); },
							   [&](const double &a, const Integral &b) { env.stack.emplace_back(a - b); },
							   [&](const Integral &a, const double &b) { env.stack.emplace_back(a - b); },
							   [&](char *a, const Integral &b) { env.stack.emplace_back(a - b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op {} on stack\n",
															token.line, token.col, token.type);
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
							   [&](const Integral &a, const Integral &b) { env.stack.emplace_back(a * b); },
							   [&](const double &a, const Integral &b) { env.stack.emplace_back(a * b); },
							   [&](const Integral &a, const double &b) { env.stack.emplace_back(a * b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op {} on stack\n",
															token.line, token.col, token.type);
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
							   [&](const Integral &a, const Integral &b) { env.stack.emplace_back(a / b); },
							   [&](const double &a, const Integral &b) { env.stack.emplace_back(a / b); },
							   [&](const Integral &a, const double &b) { env.stack.emplace_back(a / b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op {} on stack\n",
															token.line, token.col, token.type);
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
							   [&](const Integral &a, const Integral &b) { env.stack.emplace_back(a < b); },
							   [&](const double &a, const Integral &b) { env.stack.emplace_back(a < b); },
							   [&](const Integral &a, const double &b) { env.stack.emplace_back(a < b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op {} on stack\n",
															token.line, token.col, token.type);
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
							   [&](const Integral &a, const Integral &b) { env.stack.emplace_back(a <= b); },
							   [&](const double &a, const Integral &b) { env.stack.emplace_back(a <= b); },
							   [&](const Integral &a, const double &b) { env.stack.emplace_back(a <= b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op {} on stack\n",
															token.line, token.col, token.type);
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
							   [&](const Integral &a, const Integral &b) { env.stack.emplace_back(a >= b); },
							   [&](const double &a, const Integral &b) { env.stack.emplace_back(a >= b); },
							   [&](const Integral &a, const double &b) { env.stack.emplace_back(a >= b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op {} on stack\n",
															token.line, token.col, token.type);
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
							   [&](const Integral &a, const Integral &b) { env.stack.emplace_back(a > b); },
							   [&](const double &a, const Integral &b) { env.stack.emplace_back(a > b); },
							   [&](const Integral &a, const double &b) { env.stack.emplace_back(a > b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op {} on stack\n",
															token.line, token.col, token.type);
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
					env.stack.emplace_back(a == b);
					return;
				}
				std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line, token.col);
				std::terminate();
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
					env.stack.emplace_back(!(a == b));
					return;
				}
				std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line, token.col);
				std::terminate();
			},
			first, second);

	} break;
	case Token::Op_Store: {
		expect_stack_size(token, env, 2);
		const auto variable = env.stack.back();
		env.stack.pop_back();
		const auto value = env.stack.back();
		env.stack.pop_back();

		if (std::holds_alternative<Identifier>(variable)) {
			const auto &ident  = std::get<Identifier>(variable);
			auto        cellIt = variables.find(ident.content);
			if (cellIt == variables.end()) {
				std::cerr << fmt::format("{}:{} Tried to store undeclared variable\n", token.line, token.col);
				std::terminate();
			}
			cellIt->second = value;
		} else if (std::holds_alternative<char *>(variable)) {
			char *ptr = std::get<char *>(variable);
			std::visit(overloaded{ [&](const Integral &ivalue) { *ptr = static_cast<char>(ivalue & 0xFF); },
								   [&](const std::string_view &str) { std::memcpy(ptr, str.data(), str.size()); },

								   [&](const auto &) {
									   std::cerr << fmt::format("{}:{} Tried to store incompatible type {} to memory\n",
																token.line, token.col, value.index());
									   std::terminate();
								   } },
					   value);
		} else {
			std::cerr << fmt::format("{}:{} Tried to store in non variable type {}\n", token.line, token.col,
									 variable.index());
			std::terminate();
		}
	} break;
	case Token::Op_Load: {
		expect_stack_size(token, env, 1);
		const auto variable = env.stack.back();
		env.stack.pop_back();

		if (std::holds_alternative<Identifier>(variable)) {
			const auto &ident = std::get<Identifier>(variable);

			auto cellIt = variables.find(ident.content);
			if (cellIt == variables.end()) {
				std::cerr << fmt::format("{}:{} Tried to load undeclared variable\n", token.line, token.col);
				std::terminate();
			}
			env.stack.push_back(cellIt->second);
		} else if (std::holds_alternative<char *>(variable)) {
			const auto *ptr = std::get<char *>(variable);
			env.stack.emplace_back(static_cast<Integral>(*ptr));
		} else {
			std::cerr << fmt::format("{}:{} Tried to load from non variable type\n", token.line, token.col);
			std::terminate();
		}
	} break;
	case Token::Op_LS: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(overloaded{ [&](const Integral &a, const Integral &b) { env.stack.emplace_back(a << b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line,
															token.col);
								   std::terminate();
							   } },
				   first, second);

	} break;
	case Token::Op_RS: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(overloaded{ [&](const Integral &a, const Integral &b) { env.stack.emplace_back(a >> b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line,
															token.col);
								   std::terminate();
							   } },
				   first, second);

	} break;
	case Token::Op_BitOr: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(overloaded{ [&](const Integral &a, const Integral &b) { env.stack.emplace_back(a | b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line,
															token.col);
								   std::terminate();
							   } },
				   first, second);

	} break;
	case Token::Op_BitAnd: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(overloaded{ [&](const Integral &a, const Integral &b) { env.stack.emplace_back(a & b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line,
															token.col);
								   std::terminate();
							   } },
				   first, second);

	} break;
	case Token::Op_BitXor: {
		expect_stack_size(token, env, 2);
		const auto second = env.stack.back();
		env.stack.pop_back();
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(overloaded{ [&](const Integral &a, const Integral &b) { env.stack.emplace_back(a ^ b); },
							   [&](const auto &, const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line,
															token.col);
								   std::terminate();
							   } },
				   first, second);

	} break;
	case Token::Op_BitNot: {
		expect_stack_size(token, env, 1);
		const auto first = env.stack.back();
		env.stack.pop_back();

		std::visit(overloaded{ [&](const Integral &a) { env.stack.emplace_back(~a); },
							   [&](const auto &) {
								   std::cerr << fmt::format("{}:{} Unexpected data types for op on stack\n", token.line,
															token.col);
								   std::terminate();
							   } },
				   first);

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
	VariablesMap variables;
	for (std::size_t ic = 0; ic < proc.body.size(); ++ic) {
		const auto &inst  = proc.body.at(ic);
		const auto &token = inst.corresponding_token;
		switch (token.type) {
		case Token::Numeric: {
			if (token.content.find('.') != std::string_view::npos) {
				double value = 0.;
				std::from_chars(token.content.data(), token.content.data() + token.content.size(), value);
				env.stack.emplace_back(value);
			} else {
				Integral value = 0.;
				std::from_chars(token.content.data(), token.content.data() + token.content.size(), value);
				env.stack.emplace_back(value);
			}
			break;
		}
		case Token::StringLiteral: {
			env.stack.emplace_back(token.content);
			break;
		}
		case Token::Identifier: {
			if (const auto *mproc = select_procedure(env.loaded_modules, token.content); mproc) {
				eval_proc(*mproc, env);
			} else if (eval_builtin(token.content, env, token)) {
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
		case Token::Keyword_Else:
			ic = inst.forward_jump;
			continue;
			break;
		case Token::Keyword_End: {
			const auto &branch_inst = proc.body.at(inst.backward_jump);

			switch (branch_inst.corresponding_token.type) {
			case Token::Keyword_If:
			case Token::Keyword_Else:
				break;
			case Token::Keyword_While: {
				const auto &do_inst = proc.body.at(branch_inst.forward_jump);
				ic                  = do_inst.backward_jump;
				break;
			}
			default:
				std::cerr << fmt::format("{}:{} Unexpected preceding branching instruction T:{}\n", token.line,
										 token.col, branch_inst.corresponding_token.type);
				std::terminate();
				break;
			}

			break;
		}
		case Token::Keyword_Break: {
			const auto &branch_inst = proc.body.at(inst.backward_jump);

			switch (branch_inst.corresponding_token.type) {
			case Token::Keyword_While: {
				const auto &do_inst = proc.body.at(branch_inst.forward_jump);
				ic                  = do_inst.forward_jump;
				break;
			}
			default:
				std::cerr << fmt::format("{}:{} Unexpected preceding branching instruction T:{}\n", token.line,
										 token.col, branch_inst.corresponding_token.type);
				std::terminate();
				break;
			}

			break;
		}
		case Token::Keyword_Return:
			return;
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

			const auto &ident        = std::get<Identifier>(variable);
			variables[ident.content] = true;

			break;
		}

		default:
			if (!eval_ops(token, env, variables)) {
				std::cerr << fmt::format("{}:{} Unexpected token recieved {}\n", token.line, token.col, token.type);
				std::terminate();
			}
			break;
		}
	}
}
