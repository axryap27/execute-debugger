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

static bool execute_function_call(struct STMT* stmt, struct RAM* memory)
{
    if(stmt->stmt_type != STMT_FUNCTION_CALL){
        return false;
    }

    char* function_name = stmt->types.function_call->function_name;
    
}


void execute(struct STMT* program, struct RAM* memory)
{
    struct STMT* stmt = program;  // Start at the first statement
    
    while (stmt != NULL) {  // Traverse through the program statements:
        if (stmt->stmt_type == STMT_ASSIGNMENT) {
            stmt = stmt->types.assignment->next_stmt;  // Advance to next statement
        }
        else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
            stmt = stmt->types.function_call->next_stmt;  // Advance to next statement
        }
        else if (stmt->stmt_type == STMT_PASS) {
            stmt = stmt->types.pass->next_stmt;  // Advance to next statement
        }
        else {
            assert(stmt->stmt_type == STMT_PASS);
                  stmt->line, stmt->stmt_type;
            
            // Use the appropriate next_stmt field based on statement type
            if (stmt->stmt_type == STMT_IF_THEN_ELSE) {
                stmt = stmt->types.if_then_else->next_stmt;
            }
            else if (stmt->stmt_type == STMT_WHILE_LOOP) {
                stmt = stmt->types.while_loop->next_stmt;
            }
            else {
                // Unrecognized statement type, break to avoid infinite loop
                break;
            }
        }
    }
}

