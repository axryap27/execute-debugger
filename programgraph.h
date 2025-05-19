/*programgraph.h*/

//
// Project: program graph data structure for nuPython
//
// Prof. Joe Hummel
// Northwestern University
// CS 211
//

#pragma once

#include <stdbool.h>     // true, false
#include "tokenqueue.h"


//
// A nuPython program is 1 or more statements:
//

//
// nuPython statement types:
//
enum STMT_TYPES
{
  STMT_ASSIGNMENT = 0,
  STMT_FUNCTION_CALL,
  STMT_IF_THEN_ELSE,
  STMT_WHILE_LOOP,
  STMT_PASS
};

struct STMT
{
  //
  // what kind of stmt do we have?
  //
  int stmt_type;  // enum STMT_TYPES
  int line;       // what line # does it start on?

  //
  // pointer to that stmt struct:
  //
  union
  {
    struct STMT_ASSIGNMENT* assignment;
    struct STMT_FUNCTION_CALL* function_call;
    struct STMT_IF_THEN_ELSE* if_then_else;
    struct STMT_WHILE_LOOP* while_loop;
    struct STMT_PASS* pass;
  } types;
};

struct STMT_ASSIGNMENT
{
  // 
  // Examples:  x = 123 
  //           *p = x + y
  //
  char* var_name;
  bool  isPtrDeref;
  struct VALUE* rhs;  // rhs = "right-hand side"

  struct STMT* next_stmt;
};

struct STMT_FUNCTION_CALL
{
  //
  // Examples: print()
  //           print("the output is")
  //
  char* function_name;
  struct ELEMENT* parameter;  // optional => could be NULL

  struct STMT* next_stmt;
};

struct STMT_IF_THEN_ELSE
{
  //
  // Example: if x<0:
  //          { ... }
  //          elif x==0:
  //          { ... }
  //          else:
  //          { ... }
  //
  struct EXPR* condition;
  struct STMT* true_path;  // next stmt if the condition is true
  struct STMT* false_path; // next stmt if the condition is false
  //
  // NOTE: the next stmt only has meaning for the initial if
  // stmt. For any nested elif statements, those will have 
  // NULL for next_stmt. We need to know which stmt follows
  // an IF statement for printing purposes, and freeing memory.
  //
  struct STMT* next_stmt;  // next stmt following if stmt
};

struct STMT_WHILE_LOOP
{
  //
  // Example: while x<10:
  //          { ... }
  //
  struct EXPR* condition;
  struct STMT* loop_body; // loop body if the condition is true
  struct STMT* next_stmt; // next stmt after the loop is over
};

struct STMT_PASS
{
  //
  // Example: pass
  //
  struct STMT* next_stmt;
};


//
// nuPython values / expressions:
//
enum VALUE_TYPES
{
  VALUE_FUNCTION_CALL = 0,
  VALUE_EXPR
};

struct VALUE
{
  //
  // what kind of value do we have?
  //
  int value_type;  // enum VALUE_TYPES

  //
  // pointer to that value struct:
  //
  union
  {
    struct FUNCTION_CALL* function_call;
    struct EXPR* expr;
  } types;
};

struct FUNCTION_CALL
{
  char* function_name;
  struct ELEMENT* parameter;  // optional => could be NULL
};

struct EXPR
{
  struct UNARY_EXPR* lhs;  // lhs = "left-hand side"

  bool   isBinaryExpr;    // true => we have operator_type and rhs

  int    operator_type;   // enum OPERATORS
  struct UNARY_EXPR* rhs; // optional => could be NULL
};

enum UNARY_EXPR_TYPES
{
  UNARY_PTR_DEREF = 0,
  UNARY_ADDRESS_OF,
  UNARY_PLUS,
  UNARY_MINUS,
  UNARY_ELEMENT
};

struct UNARY_EXPR
{
  //
  // what kind of unary expression do we have?
  //
  int expr_type;  // enum UNARY_EXPR_TYPES

  //
  // underlying element (identifier or literal):
  //
  struct ELEMENT* element;
};


//
// nuPython elements
//
enum ELEMENT_TYPES
{
  ELEMENT_IDENTIFIER = 0,
  ELEMENT_INT_LITERAL,
  ELEMENT_REAL_LITERAL,
  ELEMENT_STR_LITERAL,
  ELEMENT_TRUE,
  ELEMENT_FALSE,
  ELEMENT_NONE
};

struct ELEMENT
{
  //
  // what kind of element do we have?
  //
  int element_type;  // enum ELEMENT_TYPES

  //
  // underlying element (identifier or literal):
  //
  char* element_value;  // e.g. "x" or "123" or "3.14" or "this is a string"
};


//
// nuPython operators
//
enum OPERATORS
{
  OPERATOR_PLUS = 0,
  OPERATOR_MINUS,
  OPERATOR_ASTERISK,
  OPERATOR_POWER,
  OPERATOR_MOD,
  OPERATOR_DIV,
  OPERATOR_EQUAL,
  OPERATOR_NOT_EQUAL,
  OPERATOR_LT,
  OPERATOR_LTE,
  OPERATOR_GT,
  OPERATOR_GTE,
  OPERATOR_IS,
  OPERATOR_IN,
  OPERATOR_NO_OP  // when there is no operator_type
};


//
// Public functions:
//

//
// programgraph_build
//
// Given a legal nuPython program in the form of a list
// of tokens, builds and returns a program graph
// representing the nuPython program. This is easier
// to work with than the raw tokens. 
//
// Returns NULL if an error occurs and the program graph
// could not be built.
// 
// NOTE: the program graph may contain semantic errors, 
// e.g. type errors or calls to functions that don't exist.
// Semantic errors need to be detected during execution 
// (it could also be done using a pre-execution pass 
// through the graph).
//
struct STMT* programgraph_build(struct TokenQueue* tokens);

//
// programgraph_destroy
//
// Frees all the memory with in given program graph.
//
void programgraph_destroy(struct STMT* program);

//
// programgraph_print
//
// Prints the contents of the program graph to the console.
//
void programgraph_print(struct STMT* program);
