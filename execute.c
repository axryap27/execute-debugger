/*execute.c*/

//
// Execution engine for nuPython programs. This file contains
// the execute() function and helper functions that interpret
// and execute nuPython statements from a program graph and allocated RAM memory.
// Supports assignment statements, function calls (print),
// binary expressions, int(), float(), input() function calls
// if-then-else statements, and while loops. Extends last version
// by allowing for reals, ints, and string operations of binary expressions.
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
#include <stdbool.h> // true, false
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
// Input(), int(), floater() functions for assignment
//

//
// execute_input_function
//
// Handles the input() function call by prompting the user with
// a string literal and reading their input from stdin. The function
// expects a string literal parameter as the prompt, reads a complete
// line of user input, removes end-of-line characters, and returns
// the input as a dynamically allocated string. If a semantic error
// occurs (e.g. missing parameter, non-string parameter), an error
// message is output and the function returns false.
//
static bool execute_input_function(struct FUNCTION_CALL *func_call, struct RAM_VALUE *result, int line)
{
    struct ELEMENT *param = func_call->parameter;

    if (param == NULL || param->element_type != ELEMENT_STR_LITERAL)
    {
        printf("**SEMANTIC ERROR: input() requires a string literal (line %d)\n", line);
        return false;
    }

    // Print the prompt
    printf("%s", param->element_value);

    // Read user input
    char line_input[256];
    fgets(line_input, sizeof(line_input), stdin);

    // Remove EOL characters
    line_input[strcspn(line_input, "\r\n")] = '\0';

    // Create a copy of the input string
    int len = strlen(line_input);
    char *input_copy = malloc(len + 1);
    strcpy(input_copy, line_input);

    result->value_type = RAM_TYPE_STR;
    result->types.s = input_copy;
    return true;
}

//
// execute_int_function
//
// Handles the int() function call by converting a string variable
// to an integer value. The function expects a variable parameter
// containing a string value, retrieves the string from memory,
// and converts it to an integer using atoi(). Handles the special
// case where "0" or strings of all zeros are valid conversions
// that return 0. If a semantic error occurs (e.g. missing parameter,
// non-variable parameter, undefined variable, non-string variable,
// or invalid string conversion), an error message is output and
// the function returns false.
//
static bool execute_int_function(struct FUNCTION_CALL *func_call, struct RAM *memory, struct RAM_VALUE *result, int line)
{
    struct ELEMENT *param = func_call->parameter;

    if (param == NULL || param->element_type != ELEMENT_IDENTIFIER)
    {
        printf("**SEMANTIC ERROR: int() requires a variable (line %d)\n", line);
        return false;
    }

    // Get the variable value
    char *var_name = param->element_value;
    struct RAM_VALUE *var_value = ram_read_cell_by_name(memory, var_name);

    if (var_value == NULL)
    {
        printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, line);
        return false;
    }

    if (var_value->value_type != RAM_TYPE_STR)
    {
        printf("**SEMANTIC ERROR: int() requires a string (line %d)\n", line);
        ram_free_value(var_value);
        return false;
    }

    // Convert string to integer
    char *str_value = var_value->types.s;
    int converted = atoi(str_value);

    // Check for conversion failure (special case for "0")
    if (converted == 0 && strcmp(str_value, "0") != 0)
    {
        // Check if it's all zeros
        bool all_zeros = true;
        for (int i = 0; str_value[i] != '\0'; i++)
        {
            if (str_value[i] != '0')
            {
                all_zeros = false;
                break;
            }
        }
        if (!all_zeros)
        {
            printf("**SEMANTIC ERROR: invalid string for int() (line %d)\n", line);
            ram_free_value(var_value);
            return false;
        }
    }

    result->value_type = RAM_TYPE_INT;
    result->types.i = converted;
    ram_free_value(var_value);
    return true;
}

//
// execute_float_function
//
// Handles the float() function call by converting a string variable
// to a real (double) value. The function expects a variable parameter
// containing a string value, retrieves the string from memory,
// and converts it to a real using atof(). Handles the special
// case where "0", "0.0", or strings of all zeros with optional
// decimal points are valid conversions that return 0.0. If a semantic
// error occurs (e.g. missing parameter, non-variable parameter,
// undefined variable, non-string variable, or invalid string
// conversion), an error message is output and the function returns false.
//
static bool execute_float_function(struct FUNCTION_CALL *func_call, struct RAM *memory, struct RAM_VALUE *result, int line)
{
    struct ELEMENT *param = func_call->parameter;

    if (param == NULL || param->element_type != ELEMENT_IDENTIFIER)
    {
        printf("**SEMANTIC ERROR: float() requires a variable (line %d)\n", line);
        return false;
    }

    // Get the variable value
    char *var_name = param->element_value;
    struct RAM_VALUE *var_value = ram_read_cell_by_name(memory, var_name);

    if (var_value == NULL)
    {
        printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, line);
        return false;
    }

    if (var_value->value_type != RAM_TYPE_STR)
    {
        printf("**SEMANTIC ERROR: float() requires a string (line %d)\n", line);
        ram_free_value(var_value);
        return false;
    }

    // Convert string to real
    char *str_value = var_value->types.s;
    double converted = atof(str_value);

    // Check for conversion failure (special case for "0")
    if (converted == 0.0 && strcmp(str_value, "0") != 0 && strcmp(str_value, "0.0") != 0)
    {
        // Check if it's all zeros (with possible decimal point)
        bool all_zeros = true;
        for (int i = 0; str_value[i] != '\0'; i++)
        {
            if (str_value[i] != '0' && str_value[i] != '.')
            {
                all_zeros = false;
                break;
            }
        }
        if (!all_zeros)
        {
            printf("**SEMANTIC ERROR: invalid string for float() (line %d)\n", line);
            ram_free_value(var_value);
            return false;
        }
    }

    result->value_type = RAM_TYPE_REAL;
    result->types.d = converted;
    ram_free_value(var_value);
    return true;
}

//
// execute_assignment_function_call
//
// Main dispatcher function for handling function calls that appear
// on the right-hand side of assignment statements. Identifies the
// function by name and delegates execution to the appropriate
// specialized handler: execute_input_function for input() calls,
// execute_int_function for int() calls, and execute_float_function
// for float() calls. If a semantic error occurs (e.g. unknown
// function name), an error message is output and the function
// returns false.
//
static bool execute_assignment_function_call(struct FUNCTION_CALL *func_call, struct RAM *memory, struct RAM_VALUE *result, int line)
{
    char *function_name = func_call->function_name;

    if (strcmp(function_name, "input") == 0)
    {
        return execute_input_function(func_call, result, line);
    }
    else if (strcmp(function_name, "int") == 0)
    {
        return execute_int_function(func_call, memory, result, line);
    }
    else if (strcmp(function_name, "float") == 0)
    {
        return execute_float_function(func_call, memory, result, line);
    }
    else
    {
        printf("**SEMANTIC ERROR: unknown function '%s' (line %d)\n", function_name, line);
        return false;
    }
}


//
//
// Helper functions for binary comparisons
//
//

//
// execute_int_comparison
//
// Performs relational comparison operations on two integer values
// and returns a boolean result. Supports all comparison operators:
// equality (==), inequality (!=), less than (<), less than or equal (<=),
// greater than (>), and greater than or equal (>=). The result is
// stored as a RAM_TYPE_BOOLEAN with integer value 1 for True and 0
// for False. If an unsupported operator type is provided, the function
// returns false to indicate an error.
//
static bool execute_int_comparison(int lhs, int rhs, int operator_type, struct RAM_VALUE *result, int line)
{
    result->value_type = RAM_TYPE_BOOLEAN;

    if (operator_type == OPERATOR_EQUAL)
    {
        if (lhs == rhs)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    else if (operator_type == OPERATOR_NOT_EQUAL)
    {
        if (lhs != rhs)
        {
            result->types.i = 1;
        }
        else
            result->types.i = 0;
    }
    // less than
    else if (operator_type == OPERATOR_LT)
    {
        if (lhs < rhs)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    // less than or equal to
    else if (operator_type == OPERATOR_LTE)
    {
        if (lhs <= rhs)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    // greater than
    else if (operator_type == OPERATOR_GT)
    {
        if (lhs > rhs)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    // greater than or equal to
    else if (operator_type == OPERATOR_GTE)
    {
        if (lhs >= rhs)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    else
    {
        return false; // Not a comparison operator
    }
    return true;
}

//
// execute_real_comparison
//
// Performs relational comparison operations on two real (double) values
// and returns a boolean result. Supports all comparison operators:
// equality (==), inequality (!=), less than (<), less than or equal (<=),
// greater than (>), and greater than or equal (>=). The result is
// stored as a RAM_TYPE_BOOLEAN with integer value 1 for True and 0
// for False. If an unsupported operator type is provided, the function
// returns false to indicate an error.
//
static bool execute_real_comparison(double lhs, double rhs, int operator_type, struct RAM_VALUE *result, int line)
{
    result->value_type = RAM_TYPE_BOOLEAN;

    if (operator_type == OPERATOR_EQUAL)
    {
        if (lhs == rhs)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    else if (operator_type == OPERATOR_NOT_EQUAL)
    {
        if (lhs != rhs)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    else if (operator_type == OPERATOR_LT)
    {
        if (lhs < rhs)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    else if (operator_type == OPERATOR_LTE)
    {
        if (lhs <= rhs)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    else if (operator_type == OPERATOR_GT)
    {
        if (lhs > rhs)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    else if (operator_type == OPERATOR_GTE)
    {
        if (lhs >= rhs)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    else
    {
        return false; // Not a comparison operator
    }
    return true;
}

//
// execute_string_comparison
//
// Performs relational comparison operations on two string values
// using case-sensitive string comparison and returns a boolean result.
// Uses strcmp() to compare strings lexicographically and supports all
// comparison operators: equality (==), inequality (!=), less than (<),
// less than or equal (<=), greater than (>), and greater than or equal (>=).
// The result is stored as a RAM_TYPE_BOOLEAN with integer value 1 for
// True and 0 for False. If an unsupported operator type is provided,
// the function returns false to indicate an error.
//
static bool execute_string_comparison(char *lhs, char *rhs, int operator_type, struct RAM_VALUE *result, int line)
{
    result->value_type = RAM_TYPE_BOOLEAN;
    int cmp_result = strcmp(lhs, rhs);

    if (operator_type == OPERATOR_EQUAL)
    {
        if (cmp_result == 0)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    else if (operator_type == OPERATOR_NOT_EQUAL)
    {
        if (cmp_result != 0)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    else if (operator_type == OPERATOR_LT)
    {
        if (cmp_result < 0)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    else if (operator_type == OPERATOR_LTE)
    {
        if (cmp_result <= 0)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    else if (operator_type == OPERATOR_GT)
    {
        if (cmp_result > 0)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    else if (operator_type == OPERATOR_GTE)
    {
        if (cmp_result >= 0)
        {
            result->types.i = 1;
        }
        else
        {
            result->types.i = 0;
        }
    }
    else
    {
        return false; // Not a comparison operator
    }
    return true;
}

//
// Helper functions for binary operations
//


//
// execute_int_operation
//
// Performs arithmetic operations on two integers (ints). Handles
// addition (+), subtraction (-), multiplication (*), exponentiation (**),
// modulo (%), and division (/). Uses pow() from math.h for exponentiation
// and fmod() for modulo operations since the standard % operator doesn't
// work with floating point values. Includes error checking for division
// and modulo by zero. Returns false if a division by zero occurs or an
// unsupported operator is encountered.
//
static bool execute_int_operation(int lhs, int rhs, int operator_type, struct RAM_VALUE *result, int line)
{
    result->value_type = RAM_TYPE_INT;

    if (operator_type == OPERATOR_PLUS)
    {
        result->types.i = lhs + rhs;
    }
    else if (operator_type == OPERATOR_MINUS)
    {
        result->types.i = lhs - rhs;
    }
    else if (operator_type == OPERATOR_ASTERISK)
    {
        result->types.i = lhs * rhs;
    }
    else if (operator_type == OPERATOR_POWER)
    {
        result->types.i = 1;
        for (int i = 0; i < rhs; i++)
        {
            result->types.i *= lhs;
        }
    }
    else if (operator_type == OPERATOR_MOD)
    {
        if (rhs == 0)
        {
            printf("**SEMANTIC ERROR: mod by 0 (line %d)\n", line);
            return false;
        }
        result->types.i = lhs % rhs;
    }
    else if (operator_type == OPERATOR_DIV)
    {
        if (rhs == 0)
        {
            printf("**SEMANTIC ERROR: divide by 0 (line %d)\n", line);
            return false;
        }
        result->types.i = lhs / rhs;
    }
    else
    {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
        return false;
    }
    return true;
}

//
// execute_real_operation
//
// Performs arithmetic operations on two real numbers (doubles). Handles
// addition (+), subtraction (-), multiplication (*), exponentiation (**),
// modulo (%), and division (/). Uses pow() from math.h for exponentiation
// and fmod() for modulo operations since the standard % operator doesn't
// work with floating point values. Includes error checking for division
// and modulo by zero. Returns false if a division by zero occurs or an
// unsupported operator is encountered.
//
static bool execute_real_operation(double lhs, double rhs, int operator_type, struct RAM_VALUE *result, int line)
{
    result->value_type = RAM_TYPE_REAL;

    if (operator_type == OPERATOR_PLUS)
    {
        result->types.d = lhs + rhs;
    }
    else if (operator_type == OPERATOR_MINUS)
    {
        result->types.d = lhs - rhs;
    }
    else if (operator_type == OPERATOR_ASTERISK)
    {
        result->types.d = lhs * rhs;
    }
    else if (operator_type == OPERATOR_POWER)
    {
        result->types.d = pow(lhs, rhs);
    }
    else if (operator_type == OPERATOR_MOD)
    {
        if (rhs == 0.0)
        {
            printf("**SEMANTIC ERROR: mod by 0 (line %d)\n", line);
            return false;
        }
        result->types.d = fmod(lhs, rhs);
    }
    else if (operator_type == OPERATOR_DIV)
    {
        if (rhs == 0.0)
        {
            printf("**SEMANTIC ERROR: divide by 0 (line %d)\n", line);
            return false;
        }
        result->types.d = lhs / rhs;
    }
    else
    {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
        return false;
    }
    return true;
}

//
// execute_string_operation
//
// Performs operations on two strings. Currently only supports concatenation
// using the + operator - all other operators result in a semantic error.
// Dynamically allocates memory for the concatenated result, copies the first
// string, then appends the second string. Includes error checking for memory
// allocation failure. Returns false if an unsupported operator is used or
// memory allocation fails.
//
static bool execute_string_operation(char *lhs, char *rhs, int operator_type, struct RAM_VALUE *result, int line)
{
    if (operator_type != OPERATOR_PLUS)
    {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
        return false;
    }

    // String concatenation
    int len1 = strlen(lhs);
    int len2 = strlen(rhs);
    char *concatenated = malloc(len1 + len2 + 1);

    if (concatenated == NULL)
    {
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

static bool retrieve_value(struct ELEMENT *element, struct RAM *memory, struct RAM_VALUE *result, int line)
{
    if (element->element_type == ELEMENT_INT_LITERAL)
    {
        result->value_type = RAM_TYPE_INT;
        result->types.i = atoi(element->element_value);
        return true;
    }
    else if (element->element_type == ELEMENT_REAL_LITERAL)
    {
        result->value_type = RAM_TYPE_REAL;
        result->types.d = atof(element->element_value);
        return true;
    }
    else if (element->element_type == ELEMENT_STR_LITERAL)
    {
        result->value_type = RAM_TYPE_STR;
        int len = strlen(element->element_value);
        char *str_copy = malloc(len + 1);
        strcpy(str_copy, element->element_value);
        result->types.s = str_copy;
        return true;
    }
    else if (element->element_type == ELEMENT_TRUE)
    {
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = 1;
        return true;
    }
    else if (element->element_type == ELEMENT_FALSE)
    {
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = 0;
        return true;
    }
    else if (element->element_type == ELEMENT_IDENTIFIER)
    {
        char *var_name = element->element_value;
        struct RAM_VALUE *value = ram_read_cell_by_name(memory, var_name);

        if (value == NULL)
        {
            printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, line);
            return false;
        }
        if (value->value_type == RAM_TYPE_STR)
        {
            result->value_type = RAM_TYPE_STR;
            int len = strlen(value->types.s);
            char *str_copy = malloc(len + 1);
            strcpy(str_copy, value->types.s);
            result->types.s = str_copy;
        }
        // Copy the pointer if not a string
        else
        {
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
// evaluates the expression and stores the integer result. Supports +, -, *, /, %, ** 
// operators with integer literals and variables as operands. 
// If a semantic error occurs (e.g. undefined variable, divide by 0),
// an error message is output, execution stops, and the function returns false.
// Extended to operate on reals, ints, and strings
//
static bool execute_binary_expression(struct EXPR *expr, struct RAM *memory, struct RAM_VALUE *result, int line)
{
    if (!expr->isBinaryExpr)
    {
        return retrieve_value(expr->lhs->element, memory, result, line);
    }

    struct RAM_VALUE lhs_value, rhs_value;

    if (!retrieve_value(expr->lhs->element, memory, &lhs_value, line))
    {
        return false;
    }
    if (!retrieve_value(expr->rhs->element, memory, &rhs_value, line))
    {
        return false;
    }

    if (expr->operator_type == OPERATOR_EQUAL || expr->operator_type == OPERATOR_NOT_EQUAL || expr->operator_type == OPERATOR_GT || expr->operator_type == OPERATOR_GTE || expr->operator_type == OPERATOR_LT || expr->operator_type == OPERATOR_LTE)
    {
        if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT)
        {
            return execute_int_comparison(lhs_value.types.i, rhs_value.types.i, expr->operator_type, result, line);
        }
        else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL)
        {
            return execute_real_comparison(lhs_value.types.d, rhs_value.types.d, expr->operator_type, result, line);
        }
        else if ((lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_INT) || (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_REAL))
        {
            if (lhs_value.value_type == RAM_TYPE_INT)
            {
                double lhs_real = (double)lhs_value.types.i;
                return execute_real_comparison(lhs_real, rhs_value.types.d, expr->operator_type, result, line);
            }
            else
            {
                double rhs_real = (double)rhs_value.types.i;
                return execute_real_comparison(lhs_value.types.d, rhs_real, expr->operator_type, result, line);
            }
        }
        else if (lhs_value.value_type == RAM_TYPE_STR && rhs_value.value_type == RAM_TYPE_STR)
        {
            return execute_string_comparison(lhs_value.types.s, rhs_value.types.s, expr->operator_type, result, line);
        }
    }

    // Both operands are integers
    if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT)
    {
        return execute_int_operation(lhs_value.types.i, rhs_value.types.i, expr->operator_type, result, line);
    }
    // Both operands are reals
    else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL)
    {
        return execute_real_operation(lhs_value.types.d, rhs_value.types.d, expr->operator_type, result, line);
    }
    // One int, one real - convert to reals
    else if ((lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_REAL) ||
             (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_INT))
    {
        double lhs_real = (lhs_value.value_type == RAM_TYPE_INT) ? (double)lhs_value.types.i : lhs_value.types.d;
        double rhs_real = (rhs_value.value_type == RAM_TYPE_INT) ? (double)rhs_value.types.i : rhs_value.types.d;
        return execute_real_operation(lhs_real, rhs_real, expr->operator_type, result, line);
    }
    // Both operands are strings
    else if (lhs_value.value_type == RAM_TYPE_STR && rhs_value.value_type == RAM_TYPE_STR)
    {
        return execute_string_operation(lhs_value.types.s, rhs_value.types.s, expr->operator_type, result, line);
    }
    // Invalid combination
    else
    {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line);
        return false;
    }
}

//
// execute_expr
//
// Evaluates any expression and returns the result. Handles both simple
// expressions (single values like variables or literals) and complex
// binary expressions (with operators). Allocates memory for the result
// and delegates the actual work to either execute_binary_expression or
// retrieve_value depending on the expression type. Cleans up allocated
// memory if an error occurs during evaluation. Returns NULL if memory
// allocation fails or expression evaluation encounters an error.
//
static struct RAM_VALUE *execute_expr(struct STMT *stmt, struct RAM *memory, struct EXPR *expr)
{
    struct RAM_VALUE *result = malloc(sizeof(struct RAM_VALUE));
    if (result == NULL)
    {
        return NULL;
    }
    if (expr->isBinaryExpr)
    {
        if (!execute_binary_expression(expr, memory, result, stmt->line))
        {
            free(result);
            return NULL;
        }
    }
    else
    {
        if (!retrieve_value(expr->lhs->element, memory, result, stmt->line))
        {
            free(result);
            return NULL;
        }
    }
    return result;
}

//
// execute_if_stmt
//
// Handles if/elif/else statements by evaluating the condition and
// determining which path to follow next. Uses execute_expr to evaluate
// the condition, then interprets the result as a boolean (0 = false,
// non-zero = true). Sets the next statement pointer to either the true
// path or false path based on the condition result. Only accepts integer
// or boolean condition results - other types result in a semantic error.
// Returns false if condition evaluation fails or produces an invalid type.
//
static bool execute_if_stmt(struct STMT *stmt, struct RAM *memory, struct STMT **next_stmt)
{
    assert(stmt->stmt_type == STMT_IF_THEN_ELSE);

    struct EXPR *condition = stmt->types.if_then_else->condition;
    struct RAM_VALUE *condition_result = execute_expr(stmt, memory, condition);

    if (condition_result == NULL)
    {
        return false;
    }
    bool condition_bool = false;
    if (condition_result->value_type == RAM_TYPE_INT || condition_result->value_type == RAM_TYPE_BOOLEAN)
    {
        condition_bool = condition_result->types.i != 0;
    }
    else
    {
        printf("**SEMANTIC ERROR: condition must evaluate to integer or boolean (line %d)\n", stmt->line);
        free(condition_result);
        return false;
    }

    free(condition_result);

    if (!condition_bool)
    {
        *next_stmt = stmt->types.if_then_else->false_path;
    }
    else
    {
        *next_stmt = stmt->types.if_then_else->true_path;
    }
    return true;
}


//
// execute_while_loop
//
// Handles while loop statements by evaluating the loop condition and
// determining whether to enter the loop body or exit the loop. Uses
// execute_expr to evaluate the condition, then interprets the result
// as a boolean (0 = false, non-zero = true). If the condition is true,
// sets next statement to the loop body; if false, sets it to the statement
// after the loop. Only accepts integer or boolean condition results.
// Returns false if condition evaluation fails or produces an invalid type.
//
static bool execute_while_loop(struct STMT *stmt, struct RAM *memory, struct STMT **next_stmt)
{
    assert(stmt->stmt_type == STMT_WHILE_LOOP);

    struct EXPR *condition = stmt->types.while_loop->condition;
    struct RAM_VALUE *condition_result = execute_expr(stmt, memory, condition);

    if (condition_result == NULL)
    {
        return false;
    }

    bool condition_bool = false;
    if (condition_result->value_type == RAM_TYPE_INT || condition_result->value_type == RAM_TYPE_BOOLEAN)
    {
        condition_bool = condition_result->types.i != 0;
    }
    else
    {
        printf("**SEMANTIC ERROR: condition must evaluate to integer or boolean (line %d)\n", stmt->line);
        free(condition_result);
        return false;
    }
    free(condition_result);

    if (condition_bool){
        *next_stmt = stmt->types.while_loop->loop_body;
    }
    else{
        *next_stmt = stmt->types.while_loop->next_stmt;
    }
    return true;
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
static bool write_value_to_variable(char *var_name, bool isPtrDeref, struct RAM_VALUE ram_value, struct RAM *memory, int line)
{
    if (isPtrDeref)
    {
        // Pointer-based assignment (*x = value)
        struct RAM_VALUE *addr_value = ram_read_cell_by_name(memory, var_name);
        if (addr_value == NULL)
        {
            printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, line);
            return false;
        }

        if (addr_value->value_type != RAM_TYPE_INT)
        {
            printf("**SEMANTIC ERROR: invalid memory address for assignment (line %d)\n", line);
            return false;
        }

        int address = addr_value->types.i;
        ram_free_value(addr_value);

        if (!ram_write_cell_by_addr(memory, ram_value, address))
        {
            printf("**SEMANTIC ERROR: invalid memory address for assignment (line %d)\n", line);
            return false;
        }
    }
    else
    {
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
static bool execute_assignment(struct STMT *stmt, struct RAM *memory)
{
    assert(stmt->stmt_type == STMT_ASSIGNMENT);

    char *var_name = stmt->types.assignment->var_name;
    bool isPtrDeref = stmt->types.assignment->isPtrDeref;

    // Get the RHS value
    struct VALUE *rhs = stmt->types.assignment->rhs;
    struct RAM_VALUE result;

    // Check to see if the RHS value is assignment or function call
    if (rhs->value_type == VALUE_FUNCTION_CALL)
    {
        if (!execute_assignment_function_call(rhs->types.function_call, memory, &result, stmt->line))
        {
            return false;
        }
    }
    else if (rhs->value_type == VALUE_EXPR)
    {
        struct EXPR *expr = rhs->types.expr;
        struct RAM_VALUE *expr_result = execute_expr(stmt, memory, rhs->types.expr);
        // Use the extended binary expression handler for ALL cases
        if (expr_result == NULL)
        {
            return false;
        }
        result = *expr_result;
        free(expr_result);
    }
    else
    {
        printf("**SEMANTIC ERROR: unsupported assignment type (line %d)\n", stmt->line);
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
static bool execute_function_call(struct STMT *stmt, struct RAM *memory)
{
    // Check if it's actually a function call statement
    if (stmt->stmt_type != STMT_FUNCTION_CALL)
    {
        return false; // Not a function call, shouldn't happen but just in case
    }
    char *function_name = stmt->types.function_call->function_name;

    // Check if the function is "print"
    if (strcmp(function_name, "print") == 0)
    {
        struct ELEMENT *param = stmt->types.function_call->parameter;

        if (param == NULL)
        {
            printf("\n");
            return true;
        }
        if (param->element_type == ELEMENT_STR_LITERAL)
        {
            printf("%s\n", param->element_value);
        }
        else if (param->element_type == ELEMENT_INT_LITERAL)
        {
            int param_int = atoi(param->element_value);
            printf("%d\n", param_int);
            return true;
        }
        else if (param->element_type == ELEMENT_REAL_LITERAL)
        {
            double param_real = atof(param->element_value);
            printf("%lf\n", param_real);
            return true;
        }
        else if (param->element_type == ELEMENT_FALSE)
        {
            printf("False\n");
            return true;
        }
        else if (param->element_type == ELEMENT_TRUE)
        {
            printf("True\n");
            return true;
        }
        else if (param->element_type == ELEMENT_IDENTIFIER)
        {
            char *var_name = param->element_value;
            struct RAM_VALUE *value = ram_read_cell_by_name(memory, var_name);

            if (value == NULL)
            {
                printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, stmt->line);
                return false;
            }
            // check which type the variable is
            if (value->value_type == RAM_TYPE_INT)
            {
                printf("%d\n", value->types.i);
            }
            else if (value->value_type == RAM_TYPE_REAL)
            {
                printf("%f\n", value->types.d);
            }
            else if (value->value_type == RAM_TYPE_STR)
            {
                printf("%s\n", value->types.s);
            }
            else if (value->value_type == RAM_TYPE_BOOLEAN)
            {
                if (value->types.i == 0)
                {
                    printf("False\n");
                }
                else
                {
                    printf("True\n");
                }
            }
            return true;
        }
        return true;
    }
    else
    {
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

void execute(struct STMT *program, struct RAM *memory)
{
    struct STMT *stmt = program;

    while (stmt != NULL)
    {
        if (stmt->stmt_type == STMT_ASSIGNMENT)
        {
            bool success = execute_assignment(stmt, memory);
            if (!success)
            {
                return;
            }
            stmt = stmt->types.assignment->next_stmt;
        }
        else if (stmt->stmt_type == STMT_FUNCTION_CALL)
        {
            bool success = execute_function_call(stmt, memory);
            if (!success)
            {
                return;
            }
            stmt = stmt->types.function_call->next_stmt;
        }
        else if (stmt->stmt_type == STMT_PASS)
        {
            stmt = stmt->types.pass->next_stmt;
        }
        else if (stmt->stmt_type == STMT_IF_THEN_ELSE)
        {
            struct STMT *next_stmt;                                   // Declare a local variable
            bool success = execute_if_stmt(stmt, memory, &next_stmt); // Pass ADDRESS
            if (!success)
            {
                return;
            }
            stmt = next_stmt; // Use the value set by the function
        }
        else if (stmt->stmt_type == STMT_WHILE_LOOP)
        {
            struct STMT *next_stmt;                                      // Declare a local variable
            bool success = execute_while_loop(stmt, memory, &next_stmt); // Pass ADDRESS
            if (!success)
            {
                return;
            }
            stmt = next_stmt; // Use the value set by the function
        }
        else
        {
            printf("**SEMANTIC ERROR: unknown statement type\n");
            return;
        }
    }
}