/*token.h*/

//
// Token definitions for nuPython programming language
//
// Author: Prof. Joe Hummel
// Northwestern University
// CS 211
//

#pragma once


//
// Token
// 
// A token in the nuPython programming language
//
struct Token
{
  int id;    // token id (see enum below)
  int line;  // line containing the token (1-based)
  int col;   // column where the token starts (1-based)
};


//
// TokenID
//
// Every token in nuPython has a unique ID number
//
enum TokenID
{
  nuPy_UNKNOWN = -1,  // a character that is not part of nuPython
  nuPy_EOS,           // end-of-stream, denoted by EOF or $
  nuPy_EOLN,          // end-of-line
  nuPy_LEFT_PAREN,    // (
  nuPy_RIGHT_PAREN,   // )
  nuPy_LEFT_BRACKET,  // [
  nuPy_RIGHT_BRACKET, // ]
  nuPy_LEFT_BRACE,    // {
  nuPy_RIGHT_BRACE,   // }
  nuPy_PLUS,          // +
  nuPy_MINUS,         // -
  nuPy_ASTERISK,      // *
  nuPy_POWER,         // **
  nuPy_PERCENT,       // %
  nuPy_SLASH,         // /
  nuPy_EQUAL,         // =
  nuPy_EQUALEQUAL,    // ==
  nuPy_NOTEQUAL,      // !=
  nuPy_LT,            // <
  nuPy_LTE,           // <=
  nuPy_GT,            // >
  nuPy_GTE,           // >=
  nuPy_AMPERSAND,     // &
  nuPy_COLON,         // :
  nuPy_INT_LITERAL,   // e.g. 123 
  nuPy_REAL_LITERAL,  // e.g. 3.14 or .5 or 89.
  nuPy_STR_LITERAL,   // e.g. "hello cs211" or 'hello cs211'
  nuPy_IDENTIFIER,    // e.g. print or sum or x
  //
  // keywords:
  //
  nuPy_KEYW_AND,      // and
  nuPy_KEYW_BREAK,    // break
  nuPy_KEYW_CONTINUE, // continue
  nuPy_KEYW_DEF,      // def
  nuPy_KEYW_ELIF,     // elif
  nuPy_KEYW_ELSE,     // else
  nuPy_KEYW_FALSE,    // False
  nuPy_KEYW_FOR,      // for
  nuPy_KEYW_IF,       // if
  nuPy_KEYW_IN,       // in
  nuPy_KEYW_IS,       // is
  nuPy_KEYW_NONE,     // None
  nuPy_KEYW_NOT,      // not
  nuPy_KEYW_OR,       // or
  nuPy_KEYW_PASS,     // pass
  nuPy_KEYW_RETURN,   // return
  nuPy_KEYW_TRUE,     // True
  nuPy_KEYW_WHILE     // while
};
