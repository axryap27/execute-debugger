/*ram.h*/

//
// Random access memory (RAM) for nuPython
//
// Prof. Joe Hummel
// Northwestern University
// CS 211
//

#pragma once

#include <stdbool.h>  // true, false


//
// Definition of random access memory (RAM)
//
enum RAM_VALUE_TYPES
{
  RAM_TYPE_INT = 0,
  RAM_TYPE_REAL,
  RAM_TYPE_STR,
  RAM_TYPE_PTR,
  RAM_TYPE_BOOLEAN,
  RAM_TYPE_NONE
};

struct RAM_VALUE
{
  //
  // What type of value is stored here?
  //
  int value_type;  // enum RAM_VALUE_TYPES

  //
  // the actual value:
  //
  union
  {
    int    i; // INT, PTR, BOOLEAN
    double d; // REAL
    char*  s; // STR 
  } types;
};

struct RAM_CELL
{
  char* identifier;  // variable name for this memory cell
  struct RAM_VALUE value;
};

struct RAM
{
  struct RAM_CELL* cells;  // array of memory cells
  int num_values;  // # of values currently stored in memory
  int capacity;    // total # of cells available in memory
};


//
// Public functions:
//

//
// ram_init
//
// Returns a pointer to a dynamically-allocated memory
// for storing nuPython variables and their values. All
// memory cells are initialized to the value None. You
// take ownership of the returned memory and must call
// ram_destroy() when you are done.
//
struct RAM* ram_init(void);

//
// ram_destroy
//
// Frees the dynamically-allocated memory associated with
// the given memory. After the call returns, you cannot
// use the memory.
//
void ram_destroy(struct RAM* memory);

//
// ram_get_addr
// 
// If the given identifier (e.g. "x") has been written to 
// memory, returns the address of this value --- an integer
// in the range 0..N-1 where N is the number of values currently 
// stored in memory. Returns -1 if no such identifier exists 
// in memory. 
// 
// NOTE: a variable has to be written to memory before you can
// get its address. Once a variable is written to memory, its
// address never changes. 
//
int ram_get_addr(struct RAM* memory, char* identifier);

//
// ram_read_cell_by_addr
//
// Given a memory address (an integer in the range 0..N-1), 
// returns a COPY of the value contained in that memory cell.
// Returns NULL if the address is not valid.
//
// NOTE: this function allocates memory for the value that
// is returned. The caller takes ownership of the copy and 
// must eventually free this memory via ram_free_value().
//
// NOTE: a variable has to be written to memory before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
struct RAM_VALUE* ram_read_cell_by_addr(struct RAM* memory, int address);

// 
// ram_read_cell_by_name
//
// If the given name (e.g. "x") has been written to 
// memory, returns a COPY of the value contained in memory.
// Returns NULL if no such name exists in memory.
//
// NOTE: this function allocates memory for the value that
// is returned. The caller takes ownership of the copy and 
// must eventually free this memory via ram_free_value().
//
struct RAM_VALUE* ram_read_cell_by_name(struct RAM* memory, char* name);

//
// ram_free_value
//
// Frees the memory value returned by ram_read_cell_by_name and
// ram_read_cell_by_addr.
//
void ram_free_value(struct RAM_VALUE* value);

//
// ram_write_cell_by_addr
//
// Writes the given value to the memory cell at the given 
// address. If a value already exists at this address, that
// value is overwritten by this new value. Returns true if 
// the value was successfully written, false if not (which 
// implies the memory address is invalid).
// 
// NOTE: if the value being written is a string, it will
// be duplicated and stored.
// 
// NOTE: a variable has to be written to memory before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
bool ram_write_cell_by_addr(struct RAM* memory, struct RAM_VALUE value, int address);

//
// ram_write_cell_by_name
//
// Writes the given value to a memory cell named by the given
// name. If a memory cell already exists with this name, the
// existing value is overwritten by this new value. Returns
// true since this operation always succeeds.
// 
// NOTE: if the value being written is a string, it will
// be duplicated and stored.
// 
// NOTE: a variable has to be written to memory before its
// address becomes valid. Once a variable is written to memory,
// its address never changes. Memory does not have gaps, if
// two variables are written to memory one after another (the
// first time they are written), they will occupy memory 
// addresses N and N+1. Do not confuse memory size with memory
// capacity: if the size of memory is two, that means it contains
// two variables and their addresses are 0 and 1.
// 
bool ram_write_cell_by_name(struct RAM* memory, struct RAM_VALUE value, char* name);

//
// ram_print
//
// Prints the contents of RAM to the console, for debugging.
//
void ram_print(struct RAM* memory);

