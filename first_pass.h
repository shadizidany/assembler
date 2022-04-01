/*Shadi Zidany
Julian Jubran*/
#ifndef FIRST_PASS_H /*returns true if this macro is not defined*/
#define FIRST_PASS_H /*substitutes a preprocessor macro*/

#include "utilities.h"

/*firstPass: reads a line from assembly file and call other function to parse it till end-of-file
parameters: no arguments
return value: 0 if second pass can be executed else returns total errors found*/
int firstPass(void);

/*lineParser1: checks ptr and calls relevant functions depends on the given statement
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 on success and -1 if error found*/
int lineParser1(char **ptr);

/*directive: checks the directive statement and syntax in ptr and call relevant function to update data table
parameters: ptr - pointer to line tokens; label - indicates if label is defined or not and update symbol table
return value: 0 on success and -1 if error found*/
int directive(char **ptr, int label);

/*updateData: add all .data parameters (numbers) to data table
parameters: ptr - pointer to line tokens (sub-strings); len - total parameters in directive
return value: 0 on success or call terminate function to terminate the program if malloc failed*/
int updateData(char **ptr, int len);

/*instChecker: checks if ptr is a valid command and calls relevant functions to check operands
parameters: ptr - pointer to line tokens; label - indicates if label is defined or not and update symbol table
return value: 0 on success and -1 if error found*/
int instChecker(char **ptr, int label);

/*twoOperandsInst: checks if ptr contains two valid operands and right syntax and update instruction table
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 on success and -1 if error found*/
int twoOperandsInst(char **ptr);

/*oneOperandInst: checks if ptr contains a valid operand and right syntax and update instruction table
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 on success and -1 if error found*/
int oneOperandInst(char **ptr);

#endif /*ends preprocessor conditional*/