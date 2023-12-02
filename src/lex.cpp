#include "lex.h"

#include <array>
#include <cctype>
#include <string_view>
#include <vector>

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
	std::pair{ "<", Token::Op_LT },
	std::pair{ "<=", Token::Op_LE },
	std::pair{ ">=", Token::Op_GE },
	std::pair{ ">", Token::Op_GT },
	std::pair{ "==", Token::Op_EQ },
	std::pair{ "!=", Token::Op_NE },
};

std::vector<Token> lex(std::string_view input)
{
	std::vector<Token> result;

	int line_counter = 0;
	int col_counter  = 0;

	for (auto it = input.begin(); it != input.end(); ++it) {
		const auto c = *it;

		if (c == '\n') {
			col_counter = 0;
			line_counter++;
			continue;
		}

		Token token;
		token.col  = col_counter;
		token.line = line_counter;

		if (c == '"') {
			token.type         = Token::StringLiteral;
			std::size_t length = 0;
			for (auto sit = std::next(it); sit != input.end() && *sit != '"'; ++sit) {
				++length;
			}
			token.content = std::string_view{ std::next(it), length };
			result.push_back(token);
			std::advance(it, 2 + length);
			col_counter += static_cast<int>(2 + length);
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
			col_counter += static_cast<int>(length - 1);
			result.push_back(token);
		}

		else if (std::isalpha(c) != 0) {
			token.type         = Token::Identifier;
			std::size_t length = 0;
			for (auto sit = it; sit != input.end() && std::isalnum(*sit) != 0; ++sit) {
				++length;
			}
			token.content = std::string_view{ it, length };
			std::advance(it, length - 1);
			col_counter += static_cast<int>(length);

			for (const auto &[name, type] : token_names) {
				if (token.content == name) {
					token.type = type;
				}
			}
			result.push_back(token);
			continue;
		}

		else if (c == '/') {
			it = std::next(it);
			if (it != input.end() && *it == '/') {
				// We have a comment
				while (it != input.end() && *it != '\n') {
					std::advance(it, 1);
					col_counter++;
				}
				line_counter++;
				continue;
			}
		}

		else if (std::isspace(c) == 0) {
			std::size_t length = 0;
			for (auto sit = it; sit != input.end() && (std::isspace(*sit) == 0); ++sit) {
				++length;
			}
			token.content = std::string_view{ it, length };

			for (const auto &[name, type] : token_names) {
				if (token.content == name) {
					token.type = type;
					result.push_back(token);
					std::advance(it, length - 1);
					col_counter += static_cast<int>(length);
					break;
				}
			}
		}
	}

	return result;
}
