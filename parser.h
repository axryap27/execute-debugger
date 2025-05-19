/*parser.h*/

//
// Recursive-descent parsing functions for nuPython programming language.
// The parser is responsible for checking if the input follows the syntax
// ("grammar") rules of nuPython. If successful, a copy of the tokens is
// returned so the program can be analyzed and executed.
//
// Author: Prof. Joe Hummel
// Northwestern University
// CS 211
//

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // true, false

#include "tokenqueue.h"


//
// parser_parse
//
// Given an input stream, uses the scanner to obtain the tokens
// and then checks the syntax of the input against the BNF rules
// for the subset of Python we are supporting. 
//
// Returns NULL if a syntax error was found; in this case 
// an error message was output. Returns a pointer to a list
// of tokens -- a Token Queue -- if no syntax errors were 
// detected. This queue contains the complete input in token
// form for analysis and execution.
//
// NOTE: it is the callers responsibility to free the resources
// used by the Token Queue.
//
struct TokenQueue* parser_parse(FILE* input);
