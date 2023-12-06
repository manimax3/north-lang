#include "eval.h"

#include <array>
#include <fmt/format.h>
#include <iostream>
#include <thread>
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

template<typename T>
T get_from_stack(Environment &env, const Token &token)
{
	auto value = env.stack.back();
	env.stack.pop_back();
	if (!std::holds_alternative<T>(value)) {
		std::cerr << fmt::format("{}:{} Expected file object on the stack\n", token.line, token.col);
		std::terminate();
	}
	return std::get<T>(value);
}

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

void eval_sleep(Environment &env, const Token &token)
{
	const auto first = env.stack.back();
	env.stack.pop_back();

	if (!std::holds_alternative<Integral>(first)) {
		std::cerr << fmt::format("{}:{} Expected Integral value on the stack\n", token.line, token.col);
		std::terminate();
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(std::get<Integral>(first)));
}

void eval_fopen(Environment &env, const Token &token)
{
	const auto flags = env.stack.back();
	env.stack.pop_back();

	const auto filename = env.stack.back();
	env.stack.pop_back();

	if (!std::holds_alternative<std::string_view>(filename) || !std::holds_alternative<std::string_view>(flags)) {
		std::cerr << fmt::format("{}:{} Expected string literals on the stack\n", token.line, token.col);
		std::terminate();
	}

	auto *file = fopen(std::string{ std::get<std::string_view>(filename) }.c_str(),
					   std::string{ std::get<std::string_view>(flags) }.c_str());

	if (!file) {
		std::cerr << fmt::format("{}:{} Failed to fopen file {}\n", token.line, token.col,
								 std::get<std::string_view>(filename));
		std::terminate();
	}

	env.stack.emplace_back(static_cast<void *>(file));
}

void eval_fclose(Environment &env, const Token &token)
{
	const auto file = env.stack.back();
	env.stack.pop_back();

	if (!std::holds_alternative<void *>(file)) {
		std::cerr << fmt::format("{}:{} Expected file object on the stack\n", token.line, token.col);
		std::terminate();
	}

	auto *ffile = static_cast<FILE *>(std::get<void *>(file));
	fclose(ffile);
}

void eval_fseek(Environment &env, const Token &token)
{
	const auto whence = get_from_stack<Integral>(env, token);
	const auto offset = get_from_stack<Integral>(env, token);
	auto      *file   = get_from_stack<void *>(env, token);
	auto      *ffile  = static_cast<FILE *>(file);

	const auto result = fseek(ffile, offset, whence);
	env.stack.emplace_back(result);
}

void eval_ftell(Environment &env, const Token &token)
{
	auto *file  = get_from_stack<void *>(env, token);
	auto *ffile = static_cast<FILE *>(file);

	const auto result = ftell(ffile);
	env.stack.emplace_back(static_cast<Integral>(result)); // TODOMAX use int64
}

void eval_fread(Environment &env, const Token &token)
{
	auto *file  = get_from_stack<void *>(env, token);
	auto *ffile = static_cast<FILE *>(file);

	const auto n    = get_from_stack<Integral>(env, token);
	const auto size = get_from_stack<Integral>(env, token);
	auto      *ptr  = get_from_stack<char *>(env, token);

	const auto result = fread(ptr, static_cast<std::size_t>(size), static_cast<std::size_t>(n), ffile);
	env.stack.emplace_back(static_cast<Integral>(result)); // TODOMAX use int64
}

void eval_fwrite(Environment &env, const Token &token)
{
	auto *file  = get_from_stack<void *>(env, token);
	auto *ffile = static_cast<FILE *>(file);

	const auto n    = get_from_stack<Integral>(env, token);
	const auto size = get_from_stack<Integral>(env, token);
	auto      *ptr  = get_from_stack<char *>(env, token);

	const auto result = fwrite(ptr, static_cast<std::size_t>(size), static_cast<std::size_t>(n), ffile);
	env.stack.emplace_back(static_cast<Integral>(result)); // TODOMAX use int64
}

void eval_alloc(Environment &env, const Token &token)
{
	const auto size   = get_from_stack<Integral>(env, token);
	auto      *result = malloc(static_cast<std::size_t>(size));
	env.stack.emplace_back(static_cast<char *>(result)); // TODOMAX use int64
}

void eval_dealloc(Environment &env, const Token &token)
{
	auto *ptr = get_from_stack<char *>(env, token);
	free(ptr);
}

void eval_as_string_view(Environment &env, const Token &token)
{
	const auto size   = get_from_stack<Integral>(env, token);
	auto      *buffer = get_from_stack<char *>(env, token);
	env.stack.emplace_back(std::string_view{ buffer, static_cast<std::size_t>(size) });
}

void eval_sv_length(Environment &env, const Token &token)
{
	const auto sv = get_from_stack<std::string_view>(env, token);
	env.stack.emplace_back(static_cast<Integral>(sv.length()));
}

void eval_sv_get(Environment &env, const Token &token)
{
	const auto index = get_from_stack<Integral>(env, token);
	const auto sv    = get_from_stack<std::string_view>(env, token);

	if (index < 0 || index >= static_cast<Integral>(sv.size())) {
		std::cerr << fmt::format("{}:{} Out of bounds string_view access\n", token.line, token.col);
		std::terminate();
	}

	env.stack.emplace_back(static_cast<Integral>(sv[static_cast<std::size_t>(index)]));
}

void eval_write_int(Environment &env, const Token &token)
{
	auto      *buffer = get_from_stack<char *>(env, token);
	const auto number = get_from_stack<Integral>(env, token);
	std::memcpy(buffer, &number, sizeof(number));
}

void eval_read_int(Environment &env, const Token &token)
{
	auto    *buffer = get_from_stack<char *>(env, token);
	Integral number = 0;
	std::memcpy(&number, buffer, sizeof(number));
	env.stack.emplace_back(number);
}

constexpr inline std::array builtins{
	BuiltinItem{ "print", 1, eval_print },
	BuiltinItem{ "true", 0, +[](Environment &env, const Token &) { env.stack.emplace_back(true); } },
	BuiltinItem{ "false", 0, +[](Environment &env, const Token &) { env.stack.emplace_back(false); } },
	BuiltinItem{ "and", 2, eval_and },
	BuiltinItem{ "or", 2, eval_or },
	BuiltinItem{ "not", 1, eval_not },
	BuiltinItem{ "sleep", 1, eval_sleep },
	BuiltinItem{ "fopen", 2, eval_fopen },
	BuiltinItem{ "fclose", 1, eval_fclose },
	BuiltinItem{ "fseek", 3, eval_fseek },
	BuiltinItem{ "ftell", 1, eval_ftell },
	BuiltinItem{ "fread", 4, eval_fread },
	BuiltinItem{ "fwrite", 4, eval_fwrite },
	BuiltinItem{ "alloc", 1, eval_alloc },
	BuiltinItem{ "dealloc", 1, eval_dealloc },
	BuiltinItem{ "as_string_view", 2, eval_as_string_view },
	BuiltinItem{ "string_view_length", 1, eval_sv_length },
	BuiltinItem{ "string_view_get", 2, eval_sv_get },
	BuiltinItem{
		"size_int", 0,
		+[](Environment &env, const Token &) { env.stack.emplace_back(static_cast<Integral>(sizeof(Integral))); } },
	BuiltinItem{ "write_int", 2, eval_write_int },
	BuiltinItem{ "read_int", 1, eval_read_int },
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
