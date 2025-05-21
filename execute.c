/*execute.c*/

//
// << WHAT IS THE PURPOSE OF THIS FILE??? >>
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

#include "programgraph.h"
#include "ram.h"
#include "execute.h"


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

// returns true if statement executes successfully 
static bool execute_assignment(struct STMT* stmt, struct RAM* memory)
{
    // Ensure we're dealing with an assignment statement
    assert(stmt->stmt_type == STMT_ASSIGNMENT);
    
    // Get the variable name being assigned to
    char* var_name = stmt->types.assignment->var_name;
    
    // Get the RHS value
    struct VALUE* rhs = stmt->types.assignment->rhs;
    struct EXPR* expr = rhs->types.expr;
    struct UNARY_EXPR* unary_expr = expr->lhs;
    struct ELEMENT* element = unary_expr->element;

    if (element->element_type == ELEMENT_INT_LITERAL){
        char * int_str = element->element_value;
        int value = atoi(int_str);

        struct RAM_VALUE ram_value;
        ram_value.value_type = RAM_TYPE_INT;
        ram_value.types.i = value;
        ram_write_cell_by_name(memory, ram_value, var_name);

    }
    else if (element->element_type == ELEMENT_REAL_LITERAL){
        char* real_str = element->element_value;
        double value = atof(real_str);

        struct RAM_VALUE ram_value;
        ram_value.value_type = RAM_TYPE_REAL;
        ram_value.types.d = value;
        ram_write_cell_by_name(memory, ram_value, var_name);
    }
    else if (element->element_type == ELEMENT_STR_LITERAL){
        char* str_value = element->element_value;


        struct RAM_VALUE ram_value;
        ram_value.value_type = RAM_TYPE_STR;
        ram_value.types.s = str_value;
        ram_write_cell_by_name(memory, ram_value, var_name);
    }
    
    return true;
}
    


static bool execute_function_call(struct STMT* stmt, struct RAM* memory)
{
    // Check if it's actually a function call statement
    if (stmt->stmt_type != STMT_FUNCTION_CALL) {
        return false;  // Not a function call, shouldn't happen but just in case
    }

    // Get the function name
    char* function_name = stmt->types.function_call->function_name;
    
    // Check if the function is "print"
    if (strcmp(function_name, "print") == 0) {
        struct ELEMENT* param = stmt->types.function_call->parameter;
        
        // Check if there are any parameters
        if (param == NULL) {
            // print() with no parameter - just print a newline
            printf("\n");
            return true;
        }
        // Check if the parameter is a string literal
        if (param->element_type == ELEMENT_STR_LITERAL){
            printf("%s\n", param->element_value);
        }
        else if (param->element_type == ELEMENT_INT_LITERAL) {
            // Print the string literal followed by a newline
            int param_int = atoi(param->element_value);
            printf("%d\n", param_int);
            return true;
        }
        else if (param->element_type == ELEMENT_REAL_LITERAL){
            double param_real = atof(param->element_value);
            printf("%lf\n", param_real);
            return true;
        }        
        return true;  
    }
    else {
        printf("**SEMANTIC ERROR: unknown function (line %d)\n", stmt->line);
        return false;  
    }
}

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