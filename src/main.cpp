#include "eval.h"
#include "lex.h"
#include "parse.h"

#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char **argv)
{
	if (argc < 2) {
		std::cerr << "No filename supplied: ./north <filename>\n";
		return 1;
	}

	std::string buffer;
	{
		std::ifstream ifs;
		ifs.open(argv[1], std::ios_base::in);
		if (!ifs.is_open()) {
			std::cerr << fmt::format("Could not open file {}\n", argv[1]);
			return 1;
		}

		std::ostringstream oss;
		oss << ifs.rdbuf();
		buffer = oss.str();
	}

	auto tokens = lex(buffer);

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
