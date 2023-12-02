#pragma once

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
		Keyword_Rotate,
		Keyword_Import,

		Op_Add,
		Op_Sub,
		Op_Mul,
		Op_Div,
		Op_Store,
		Op_Load,
		Op_LT,
		Op_LE,
		Op_GE,
		Op_GT,
		Op_EQ,
		Op_NE,

		Op_LS,
		Op_RS,
		Op_BitAnd,
		Op_BitOr,
		Op_BitNot,
		Op_BitXor,

		Count
	};

	TokenType        type = Invalid;
	int              line = 0, col = 0;
	std::string_view content;
};

std::vector<Token> lex(std::string_view input);
