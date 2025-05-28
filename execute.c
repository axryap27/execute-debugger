/*execute.c*/

//
// Execution engine for nuPython programs. This file contains
// the execute() function and helper functions that interpret
// and execute nuPython statements from a program graph and allocated RAM memory.
// Supports assignment statements, function calls (print),
// binary expressions, and pointer operations.
//
// Aarya Patel
// Northwestern University
// CS 211
// Spring 2025
// 
// Starter code: Prof. Joe Hummel
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // true, false
#include <string.h>
#include <assert.h>
#include <math.h>

#include "programgraph.h"
#include "ram.h"
#include "execute.h"


//
// Private functions
//

//
// Helper functions for binary operations
//

//
// execute_int_operation
//
// Performs binary operations on two integers
//
static bool execute_int_operation(int lhs, int rhs, int operator_type, struct RAM_VALUE* result, int line)
{
    result->value_type = RAM_TYPE_INT;
    
    if (operator_type == OPERATOR_PLUS) {
        result->types.i = lhs + rhs;
    }
    else if (operator_type == OPERATOR_MINUS) {
        result->types.i = lhs - rhs;
    }
    else if (operator_type == OPERATOR_ASTERISK) {
        result->types.i = lhs * rhs;
    }
    else if (operator_type == OPERATOR_POWER) {
        result->types.i = 1;
        for (int i = 0; i < rhs; i++) {
            result->types.i *= lhs;
        }
    }
    else if (operator_type == OPERATOR_MOD) {
        if (rhs == 0) {
            printf("**SEMANTIC ERROR: mod by 0 (line %d)\n", line);
            return false;
        }
        result->types.i = lhs % rhs;
    }
    else if (operator_type == OPERATOR_DIV) {
        if (rhs == 0) {
            printf("**SEMANTIC ERROR: divide by 0 (line %d)\n", line);
            return false;
        }
        result->types.i = lhs / rhs;
    }
    else {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
        return false;
    }
    return true;
}

//
// execute_real_operation  
//
// Performs binary operations on two reals
//
static bool execute_real_operation(double lhs, double rhs, int operator_type, struct RAM_VALUE* result, int line)
{
    result->value_type = RAM_TYPE_REAL;
    
    if (operator_type == OPERATOR_PLUS) {
        result->types.d = lhs + rhs;
    }
    else if (operator_type == OPERATOR_MINUS) {
        result->types.d = lhs - rhs;
    }
    else if (operator_type == OPERATOR_ASTERISK) {
        result->types.d = lhs * rhs;
    }
    else if (operator_type == OPERATOR_POWER) {
        result->types.d = pow(lhs, rhs);
    }
    else if (operator_type == OPERATOR_MOD) {
        if (rhs == 0.0) {
            printf("**SEMANTIC ERROR: mod by 0 (line %d)\n", line);
            return false;
        }
        result->types.d = fmod(lhs, rhs);
    }
    else if (operator_type == OPERATOR_DIV) {
        if (rhs == 0.0) {
            printf("**SEMANTIC ERROR: divide by 0 (line %d)\n", line);
            return false;
        }
        result->types.d = lhs / rhs;
    }
    else {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
        return false;
    }
    return true;
}

//
// execute_string_operation
//
// Performs binary operations on two strings (only + for concatenation)
// Manually allocates the necessary memory for the longer, new string
// 
//
static bool execute_string_operation(char* lhs, char* rhs, int operator_type, struct RAM_VALUE* result, int line)
{
    if (operator_type != OPERATOR_PLUS) {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
        return false;
    }
    
    // String concatenation
    int len1 = strlen(lhs);
    int len2 = strlen(rhs);
    char* concatenated = malloc(len1 + len2 + 1);
    
    if (concatenated == NULL) {
        printf("**SEMANTIC ERROR: memory allocation failed (line %d)\n", line);
        return false;
    }
    
    strcpy(concatenated, lhs);
    strcat(concatenated, rhs);
    
    result->value_type = RAM_TYPE_STR;
    result->types.s = concatenated;
    return true;
}

//
// retrieve_value
//
// Given an element (integer literal or variable) and
// memory, retrieves the integer value and stores it
// in the result pointer. Used as a helper for binary
// expression evaluation. If a semantic error occurs
// (e.g. undefined variable, non-integer type), an 
// error message is output and the function returns false.
// Enhanced version that can retrieve any type of value (int, real, string, boolean)
//

static bool retrieve_value(struct ELEMENT* element, struct RAM* memory, struct RAM_VALUE* result, int line)
{
    if (element->element_type == ELEMENT_INT_LITERAL) {
        result->value_type = RAM_TYPE_INT;
        result->types.i = atoi(element->element_value);
        return true;
    } 
    else if (element->element_type == ELEMENT_REAL_LITERAL) {
        result->value_type = RAM_TYPE_REAL;
        result->types.d = atof(element->element_value);
        return true;
    }
    else if (element->element_type == ELEMENT_STR_LITERAL) {
        result->value_type = RAM_TYPE_STR;
        int len = strlen(element->element_value);
        char* str_copy = malloc(len + 1);
        strcpy(str_copy, element->element_value);
        result->types.s = str_copy;
        return true;
    }
    else if (element->element_type == ELEMENT_TRUE) {
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = 1;
        return true;
    }
    else if (element->element_type == ELEMENT_FALSE) {
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = 0;
        return true;
    }
    else if (element->element_type == ELEMENT_IDENTIFIER) {
        char* var_name = element->element_value;
        struct RAM_VALUE* value = ram_read_cell_by_name(memory, var_name);
        
        if (value == NULL) {
            printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, line);
            return false;
        }
        if (value->value_type == RAM_TYPE_STR) {
            result->value_type = RAM_TYPE_STR;
            int len = strlen(value->types.s);
            char* str_copy = malloc(len + 1);
            strcpy(str_copy, value->types.s);
            result->types.s = str_copy;
        }
        // Copy the pointer if not a string
        else{
            *result = *value;
        }
        ram_free_value(value);
        return true;
    }
    
    printf("**SEMANTIC ERROR: unsupported element type in expression (line %d)\n", line);
    return false;
}

//
// execute_binary_expression
//
// Given a binary expression, memory, and result pointer,
// evaluates the expression and stores the integer result.
// Supports +, -, *, /, %, ** operators with integer 
// literals and variables as operands. If a semantic 
// error occurs (e.g. undefined variable, divide by 0),
// an error message is output, execution stops, and 
// the function returns false.
// Extended to operate on reals, ints, and strings
//
static bool execute_binary_expression(struct EXPR* expr, struct RAM* memory, struct RAM_VALUE* result, int line)
{
    if (!expr->isBinaryExpr) {
        return retrieve_value(expr->lhs->element, memory, result, line);
    }
    
    struct RAM_VALUE lhs_value, rhs_value;
    
    if (!retrieve_value(expr->lhs->element, memory, &lhs_value, line)) {
        return false;
    }
    if (!retrieve_value(expr->rhs->element, memory, &rhs_value, line)) {
        return false;
    }
    
    // Both operands are integers
    if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT) {
        return execute_int_operation(lhs_value.types.i, rhs_value.types.i, expr->operator_type, result, line);
    }
    // Both operands are reals
    else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL) {
        return execute_real_operation(lhs_value.types.d, rhs_value.types.d, expr->operator_type, result, line);
    }
    // One int, one real - convert to reals
    else if ((lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_REAL) ||
             (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_INT)) {
        double lhs_real = (lhs_value.value_type == RAM_TYPE_INT) ? (double)lhs_value.types.i : lhs_value.types.d;
        double rhs_real = (rhs_value.value_type == RAM_TYPE_INT) ? (double)rhs_value.types.i : rhs_value.types.d;
        return execute_real_operation(lhs_real, rhs_real, expr->operator_type, result, line);
    }
    // Both operands are strings
    else if (lhs_value.value_type == RAM_TYPE_STR && rhs_value.value_type == RAM_TYPE_STR) {
        return execute_string_operation(lhs_value.types.s, rhs_value.types.s, expr->operator_type, result, line);
    }
    // Invalid combination
    else {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
        return false;
    }
}




//
// write_value_to_variable
//
// Given a variable name, value, and memory, writes the
// value to the specified variable. Handles both regular
// assignment and pointer-based assignment. If a semantic
// error occurs (e.g. invalid memory address for pointer
// assignment), an error message is output and the 
// function returns false.
//
static bool write_value_to_variable(char* var_name, bool isPtrDeref, struct RAM_VALUE ram_value, struct RAM* memory, int line)
{
    if (isPtrDeref) {
        // Pointer-based assignment (*x = value)
        struct RAM_VALUE* addr_value = ram_read_cell_by_name(memory, var_name);
        if (addr_value == NULL) {
            printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, line);
            return false;
        }
        
        if (addr_value->value_type != RAM_TYPE_INT) {
            printf("**SEMANTIC ERROR: invalid memory address for assignment (line %d)\n", line);
            return false;
        }
        
        int address = addr_value->types.i;
        ram_free_value(addr_value);
        
        if (!ram_write_cell_by_addr(memory, ram_value, address)) {
            printf("**SEMANTIC ERROR: invalid memory address for assignment (line %d)\n", line);
            return false;
        }
    } 
    else {
        // regular ram-saving assignment
        ram_write_cell_by_name(memory, ram_value, var_name);
    }
    
    return true;
}


//
// execute_assignment
//
// Given an assignment statement and memory, executes
// the assignment by evaluating the right-hand side
// and storing the result in the specified variable.
// Handles literals, variables, binary expressions,
// and pointer-based assignments. If a semantic error
// occurs (e.g. undefined variable), an error message
// is output and the function returns false.
//
static bool execute_assignment(struct STMT* stmt, struct RAM* memory)
{
    assert(stmt->stmt_type == STMT_ASSIGNMENT);
    
    char* var_name = stmt->types.assignment->var_name;
    bool isPtrDeref = stmt->types.assignment->isPtrDeref;
    
    // Get the RHS value
    struct VALUE* rhs = stmt->types.assignment->rhs;
    struct EXPR* expr = rhs->types.expr;
    
    // Use the extended binary expression handler for ALL cases
    struct RAM_VALUE result;
    if (!execute_binary_expression(expr, memory, &result, stmt->line)) {
        return false;
    }
    
    return write_value_to_variable(var_name, isPtrDeref, result, memory, stmt->line);
}
//
// execute_function_call
//
// Given a function call statement and memory, executes
// the function call. Currently supports only the print()
// function with no parameters, string literals, integer
// literals, real literals, or variables as parameters.
// If a semantic error occurs (e.g. unknown function,
// undefined variable), an error message is output and
// the function returns false.
//
static bool execute_function_call(struct STMT* stmt, struct RAM* memory)
{
    // Check if it's actually a function call statement
    if (stmt->stmt_type != STMT_FUNCTION_CALL) {
        return false;  // Not a function call, shouldn't happen but just in case
    }
    char* function_name = stmt->types.function_call->function_name;
    
    // Check if the function is "print"
    if (strcmp(function_name, "print") == 0) {
        struct ELEMENT* param = stmt->types.function_call->parameter;
        
        if (param == NULL) {
            printf("\n");
            return true;
        }
        if (param->element_type == ELEMENT_STR_LITERAL){
            printf("%s\n", param->element_value);
        }
        else if (param->element_type == ELEMENT_INT_LITERAL) {
            int param_int = atoi(param->element_value);
            printf("%d\n", param_int);
            return true;
        }
        else if (param->element_type == ELEMENT_REAL_LITERAL){
            double param_real = atof(param->element_value);
            printf("%lf\n", param_real);
            return true;
        }
        else if (param->element_type == ELEMENT_FALSE){
            printf("False\n");
            return true;
        }        
        else if (param->element_type == ELEMENT_TRUE){
            printf("True\n");
            return true;
        }
        else if (param->element_type == ELEMENT_IDENTIFIER){
            char* var_name = param->element_value;
            struct RAM_VALUE* value = ram_read_cell_by_name(memory, var_name);

            if (value == NULL){
                printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, stmt->line);
                return false;
            }
            // check which type the variable is
            if (value->value_type == RAM_TYPE_INT) {
                printf("%d\n", value->types.i);
            }
            else if (value->value_type == RAM_TYPE_REAL) {
                printf("%f\n", value->types.d);
            }
            else if (value->value_type == RAM_TYPE_STR) {
                printf("%s\n", value->types.s);
            }
            else if (value->value_type == RAM_TYPE_BOOLEAN){
                if (value->types.i == 0){
                    printf("False\n");
                }
                else{
                    printf("True\n");
                }
            }
            return true;
        }
        return true;  
    }
    else {
        printf("**SEMANTIC ERROR: unknown function (line %d)\n", stmt->line);
        return false;  
    }
}

//
// Public functions:
//

//
// execute
//
// Given a nuPython program graph and a memory, 
// executes the statements in the program graph.
// If a semantic error occurs (e.g. type error),
// and error message is output, execution stops,
// and the function returns.
//
void execute(struct STMT* program, struct RAM* memory)
{
    struct STMT* stmt = program;
    
    while (stmt != NULL) {
        if (stmt->stmt_type == STMT_ASSIGNMENT) {
            bool success = execute_assignment(stmt, memory);
            if (!success) {
                return;  // Stop execution on error
            }
            stmt = stmt->types.assignment->next_stmt;
        }
        else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
            bool success = execute_function_call(stmt, memory);
            if (!success) {
                return;  // Stop execution on error
            }
            stmt = stmt->types.function_call->next_stmt;
        }
        else if (stmt->stmt_type == STMT_PASS) {
            // Pass does nothing, just advance
            stmt = stmt->types.pass->next_stmt;
        }
        else {
            // For project 06, we only handle these three types
            assert(stmt->stmt_type == STMT_ASSIGNMENT || 
                   stmt->stmt_type == STMT_FUNCTION_CALL || 
                   stmt->stmt_type == STMT_PASS);
        }
    }
}