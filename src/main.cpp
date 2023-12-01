#include <array>
#include <charconv>
#include <cstdint>
#include <fmt/core.h>
#include <iostream>
#include <span>
#include <stack>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

struct Token {
	enum TokenType {
		Invalid = 0,
		Numeric,
		StringLiteral,
		Identifier,
		Keyword_Dup,
		Keyword_Swap,
		Keyword_Drop,
		Keyword_Do,
		Keyword_While,
		Keyword_If,
		Keyword_Proc,
		Keyword_End,
		Keyword_Variable,
		Keyword_Over,
		Keyword_Return,
		Op_Begin_,
		Op_Add,
		Op_Sub,
		Op_Mul,
		Op_Div,
		Op_Store,
		Op_Load,
		Op_End_,
		Count
	};

	TokenType        type = Invalid;
	int              line = 0, col = 0;
	std::string_view content;
};

constexpr inline std::array<std::pair<std::string_view, Token::TokenType>, Token::Count> token_names{
	std::pair{ "dup", Token::Keyword_Dup },
	std::pair{ "swap", Token::Keyword_Swap },
	std::pair{ "drop", Token::Keyword_Drop },
	std::pair{ "do", Token::Keyword_Do },
	std::pair{ "while", Token::Keyword_While },
	std::pair{ "if", Token::Keyword_If },
	std::pair{ "proc", Token::Keyword_Proc },
	std::pair{ "end", Token::Keyword_End },
	std::pair{ "variable", Token::Keyword_Variable },
	std::pair{ "over", Token::Keyword_Over },
	std::pair{ "+", Token::Op_Add },
	std::pair{ "-", Token::Op_Sub },
	std::pair{ "*", Token::Op_Mul },
	std::pair{ "/", Token::Op_Div },
	std::pair{ "!", Token::Op_Store },
	std::pair{ "?", Token::Op_Load },
};

std::vector<Token> lex(std::string_view input)
{
	std::vector<Token> result;

	for (auto it = input.begin(); it != input.end(); ++it) {
		const auto c = *it;
		Token      token;
		if (c == '"') {
			token.type         = Token::StringLiteral;
			std::size_t length = 0;
			for (auto sit = std::next(it); sit != input.end() && *sit != '"'; ++sit) {
				++length;
			}
			token.content = std::string_view{ std::next(it), length };
			result.push_back(token);
			std::advance(it, 2 + length);
		}

		else if (std::isdigit(c) != 0) {
			token.type           = Token::Numeric;
			bool        foundSep = false;
			std::size_t length   = 0;
			for (auto sit = it; sit != input.end() && std::isdigit(*sit) != 0;) {
				++length;
				sit = std::next(sit);
				if (*sit == '.' && !foundSep) {
					++length;
					sit      = std::next(sit);
					foundSep = true;
				}
			}
			token.content = std::string_view{ it, length };
			std::advance(it, length - 1);
			result.push_back(token);
		}

		else if (std::isalpha(c) != 0) {
			token.type         = Token::Identifier;
			std::size_t length = 0;
			for (auto sit = it; sit != input.end() && std::isalnum(*sit) != 0; ++sit) {
				++length;
			}
			token.content = std::string_view{ it, length };
			std::advance(it, length);

			for (const auto &[name, type] : token_names) {
				if (token.content == name) {
					token.type = type;
				}
			}

			result.push_back(token);
		}

		else if (c == '/') {
			it = std::next(it);
			if (it != input.end() && *it == '/') {
				// We have a comment
				while (it != input.end() && *it != '\n') {
					std::advance(it, 1);
				}
				continue;
			}
		}

		for (const auto &[name, type] : token_names) {
			if (name == std::string_view{ it, 1 }) {
				token.type    = type;
				token.content = { it, 1 };
				result.push_back(token);
			}
		}
	}

	return result;
}

struct Instruction {
	Token       corresponding_token;
	std::size_t forward_jump  = 0;
	std::size_t backward_jump = 0;
};

struct Procedure {
	std::string_view         name;
	std::vector<Instruction> body;
};

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
		case Token::Keyword_While: {
			jump_stack.push(inst_idx);
			break;
		}
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

struct Identifier : Token {};
using Value = std::variant<Identifier, std::string_view, double, int, char *, bool>;

struct Environment {
	std::unordered_map<std::string_view, Procedure> procedures;
	std::unordered_map<std::string_view, Value>     variables;
	std::vector<Value>                              stack;
};

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
				std::cerr << "Expected value on stack but empty\n";
				std::terminate();
			}

			const auto top = env.stack.back();
			env.stack.erase(env.stack.end() - 1);
			if (const bool *v = std::get_if<bool>(&top); v == nullptr) {
				std::cerr << "Expected boolean value on stack\n";
				std::terminate();
			} else if (!(*v)) {
				ic = inst.forward_jump;
				continue;
			}

			break;
		}
		case Token::Keyword_End: {
			const auto &do_inst     = proc.body.at(inst.backward_jump);
			const auto &branch_inst = proc.body.at(do_inst.backward_jump);

			switch (branch_inst.corresponding_token.type) {
			case Token::Keyword_If:
				break;
			case Token::Keyword_While:
				ic = do_inst.backward_jump;
				break;
			default:
				std::cerr << "Unexpected branching instruction\n";
				std::terminate();
				break;
			}

			break;
		}
		case Token::Keyword_While:
		case Token::Keyword_If:
			break;

		default:
			std::cerr << fmt::format("Unexpected token recieved {}\n", token.type);
			std::terminate();
			break;
		}
	}
}

int main()
{
	constexpr std::string_view input{
		R"(
		proc main do
		if true do "Hallo Welt" print end
		end
		)"
	};

	auto       tokens = lex(input);
	const auto proc   = parse_procedure(tokens);

	for (const auto &token : tokens) {
		std::cout << token.type << " " << token.content << std::endl;
	}

	std::cout << fmt::format("Found procedure {}\n", proc.name);
	for (std::size_t i = 0; i < proc.body.size(); ++i) {
		const auto &inst = proc.body.at(i);
		std::cout << fmt::format(" I {} Type: {} B: {} F: {}\n", i, inst.corresponding_token.content,
								 inst.backward_jump, inst.forward_jump);
	}

	Environment environ;
	eval_proc(proc, environ);

	return 0;
}
