#include "eval.h"

#include <fmt/format.h>
#include <iostream>
#include <variant>

namespace {
template<class... Ts>
struct overloaded : Ts... {
	using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

struct BuiltinItem {
	std::string_view name;
	std::size_t      arg_count;
	void (*eval)(Environment &, const Token &);
};

void eval_print(Environment &env, const Token &)
{
	const auto &v = env.stack.back();
	std::visit(overloaded{ [](const Identifier &a) { std::cout << "I:" << a.content << std::endl; },
						   [](const auto &a) { std::cout << a << std::endl; } },
			   v);
	env.stack.erase(env.stack.end() - 1);
}

void eval_and(Environment &env, const Token &token)
{
	const auto second = env.stack.back();
	env.stack.pop_back();
	const auto first = env.stack.back();
	env.stack.pop_back();

	if (!std::holds_alternative<bool>(first) || !std::holds_alternative<bool>(second)) {
		std::cerr << fmt::format("{}:{} Expected two boolean values on the stack\n", token.line, token.col);
		std::terminate();
	}

	env.stack.emplace_back(std::get<bool>(first) && std::get<bool>(second));
}

void eval_or(Environment &env, const Token &token)
{
	const auto second = env.stack.back();
	env.stack.pop_back();
	const auto first = env.stack.back();
	env.stack.pop_back();

	if (!std::holds_alternative<bool>(first) || !std::holds_alternative<bool>(second)) {
		std::cerr << fmt::format("{}:{} Expected two boolean values on the stack\n", token.line, token.col);
		std::terminate();
	}

	env.stack.emplace_back(std::get<bool>(first) || std::get<bool>(second));
}

void eval_not(Environment &env, const Token &token)
{
	const auto first = env.stack.back();
	env.stack.pop_back();

	if (!std::holds_alternative<bool>(first)) {
		std::cerr << fmt::format("{}:{} Expected boolean values on the stack\n", token.line, token.col);
		std::terminate();
	}

	env.stack.emplace_back(!std::get<bool>(first));
}

constexpr inline std::array builtins{
	BuiltinItem{ "print", 1, eval_print },
	BuiltinItem{ "true", 0, +[](Environment &env, const Token &) { env.stack.emplace_back(true); } },
	BuiltinItem{ "false", 0, +[](Environment &env, const Token &) { env.stack.emplace_back(false); } },
	BuiltinItem{ "and", 2, eval_and },
	BuiltinItem{ "or", 2, eval_or },
	BuiltinItem{ "not", 1, eval_not },
};

}

bool eval_builtin(std::string_view name, Environment &env, const Token &token)
{
	for (const auto &builtin : builtins) {
		if (builtin.name == name) {
			if (env.stack.size() < builtin.arg_count) {
				std::cerr << fmt::format("{}:{} Builtin {} expected {} arguments but stack contained only {}\n",
										 token.line, token.col, name, builtin.arg_count, env.stack.size());
				std::terminate();
			}

			builtin.eval(env, token);
			return true;
		}
	}

	return false;
}
