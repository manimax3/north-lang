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

	auto modopt = load_file(argv[1]);

	if (!modopt.has_value()) {
		std::cerr << "Failed to load module\n";
		return 1;
	}

	auto &mod = *modopt;

	//
	// std::cout << fmt::format("Found procedures {}\n", mod->procedures.size());
	// const auto &proc = mod->procedures["main"];
	// for (std::size_t i = 0; i < proc.body.size(); ++i) {
	// 	const auto &inst = proc.body.at(i);
	// 	std::cout << fmt::format(" I {} Type: {} B: {} F: {}\n", i, inst.corresponding_token.content,
	// 							 inst.backward_jump, inst.forward_jump);
	// }

	Environment env{};

	if (mod->filename.find('/') != std::string::npos) {
		const auto path = mod->filename.substr(0, mod->filename.find_last_of('/') + 1);
		env.search_paths.push_back(path);
	}

	env.loaded_modules.emplace_back(std::move(mod));
	eval_proc(env.loaded_modules[0]->procedures["main"], env);

	return 0;
}
