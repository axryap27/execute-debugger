/*execute.h*/

//
// Executes nuPython program, given as a Program Graph.
// 
// Prof. Joe Hummel
// Northwestern University
// CS 211
//

#pragma once

#include "programgraph.h"
#include "ram.h"

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
void execute(struct STMT* program, struct RAM* memory);
