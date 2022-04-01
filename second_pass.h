/*Shadi Zidany
Julian Jubran*/
#ifndef SECOND_PASS_H /*returns true if this macro is not defined*/
#define SECOND_PASS_H /*substitutes a preprocessor macro*/

#include "utilities.h"

/*secondPass: reads a line from assembly file and call other function to parse it till end-of-file
parameters: no arguments
return value: 0 if object file can be created else returns total errors found*/
int secondPass(void);

/*lineParser2: looks for entry directives or instructions and call relevant functions else line is ignored
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 on success and -1 if error found*/
int lineParser2(char **ptr);

/*instCoding: coded inst word and info word and updated instructions table
parameters: cmd - command number; ptr - pointer to line tokens (sub-strings)
return value: 0 on success and -1 if error found*/
int instCoding(unsigned cmd, char **ptr);

/*isDestOperand: checks if target operand is legal to the given command and coded operand word
parameters: cmd - command number; word - pointer to operand info word; token - pointer to string (operand)
return value: operand type on success and 0 on error*/
int isDestOperand(unsigned cmd, unsigned *word, char *token);

/*isSrcOperand: checks if source operand is legal to the given command and coded operand word
parameters: cmd - command number; word - pointer to operand info word; token - pointer to string (operand)
return value: operand type on success and 0 on error*/
int isSrcOperand(unsigned cmd, unsigned *word, char *token);

/*cmdSrcOpAllowed: checks if source operand type is valid in the given command
parameters: cmd - command number; srcOpAddMode - source operand type
return value: non-zero value if valid and 0 if invalid operand*/
int cmdSrcOpAllowed(unsigned cmd, unsigned srcOpAddMode);

/*cmdDestOpAllowed: checks if target operand type is valid in the given command
parameters: cmd - command number; destOpAddMode - target operand type
return value: non-zero value if valid and 0 if invalid operand*/
int cmdDestOpAllowed(unsigned cmd, unsigned destOpAddMode);

#endif /*ends preprocessor conditional*/