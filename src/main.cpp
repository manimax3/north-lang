#include "eval.h"
#include "lex.h"
#include "parse.h"

#include <fmt/format.h>
#include <iostream>

int main()
{
	constexpr std::string_view input{
		R"(
		proc main do
		hallo variable
		10 hallo !
		while hallo ? 0 > do
			hallo ? print
			hallo ? 1 - hallo !
		end
		end
		)"
	};

	auto tokens = lex(input);

	for (const auto &token : tokens) {
		std::cout << fmt::format("{} {} {}:{}\n", static_cast<int>(token.type), token.content, token.line, token.col);
	}

	const auto proc = parse_procedure(tokens);

	std::cout << fmt::format("Found procedure {}\n", proc.name);
	for (std::size_t i = 0; i < proc.body.size(); ++i) {
		const auto &inst = proc.body.at(i);
		std::cout << fmt::format(" I {} Type: {} B: {} F: {}\n", i, inst.corresponding_token.content,
								 inst.backward_jump, inst.forward_jump);
	}

	Environment env{};
	eval_proc(proc, env);

	return 0;
}
