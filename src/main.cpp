#include <array>
#include <cstdint>
#include <format>
#include <iostream>
#include <span>
#include <stack>
#include <string_view>
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
	Token        corresponding_token;
	std::int64_t forward_jump  = 0;
	std::int64_t backward_jump = 0;
};

struct Procedure {
	std::string_view         name;
	std::vector<Instruction> body;
};

Token expect(std::span<Token> &input, Token::TokenType type)
{
	if (input.empty()) {
		std::cerr << std::format("Expected token of type {}. Got empty sequence\n", static_cast<int>(type));
		std::exit(1);
	}

	if (input.front().type != type) {
		std::cerr << std::format("Expected token of type {} got type {} with content {}\n", static_cast<int>(type),
								 static_cast<int>(input.front().type), input.front().content);
		std::exit(1);
	}

	const auto token = input.front();
	input            = input.subspan(1);
	return token;
}

Procedure parse_procedure(std::span<Token> input)
{
	Procedure procedure;

	expect(input, Token::Keyword_Proc);
	const auto ident = expect(input, Token::Identifier);
	procedure.name   = ident.content;
	expect(input, Token::Keyword_Do);

	std::stack<std::int64_t> jump_stack;

	while (!input.empty()) {
		const auto &token = input.front();
		if (token.type == Token::Keyword_End && jump_stack.empty()) {
			break;
		}

		auto &inst               = procedure.body.emplace_back();
		inst.corresponding_token = token;

		input = input.subspan(1);
	}

	expect(input, Token::Keyword_End);

	return procedure;
}

int main()
{
	constexpr std::string_view input{
		R"(
		proc main do
		10 10 +
		)"
	};

	auto tokens = lex(input);
	parse_procedure(tokens);

	for (const auto &token : tokens) {
		std::cout << token.type << " " << token.content << std::endl;
	}

	return 0;
}
