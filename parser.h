/*Shadi Zidany
Julian Jubran*/
#ifndef PARSER_H /*returns true if this macro is not defined*/
#define PARSER_H /*substitutes a preprocessor macro*/

#include "utilities.h"

/*ignore: check if the given string is whitespaces, comment or longer than max allowed in assembly file
and adjust the file position to the next line if fgets() did not reads till end-of-line
parameters: line - pointer to string; file - file pointer where line reads from
			printWarningMSG - 1: print warning message (in first pass) and 0: no warning message (second pass)
return value: 1 if string is empty, comment or longer than max allowed and 0 else*/
int ignore(char *line, FILE *file, int printWarningMSG);

/*trimStr: trim whitespaces on both ends of str
parameters: str - pointer to string
return value: string pointer to the first non-space character in str*/
char *trimStr(char *str);

/*linetok: splits a given line into separated tokens (sub-strings)
parameters: token - pointer to line tokens (sub-strings); buffer - pointer to where line will be copied and optimized
line - pointer to the original string reads from file
return value: pointer to the first token (sub-string) splitted in line*/
char **linetok(char **token, char *buffer, char *line);

/*isLabelDef: check if token is a label definition
parameters: token - pointer to string to be checked
return value: 1 if token is a label definition; -1 on definition error and 0 if token is not a label definition*/
int isLabelDef(char *token);

/*iscmd: checks if token is an assembly command
parameters: token - pointer to string to be checked
return value: command decimal value on success and -1 if token is not a command*/
int iscmd(char *token);

/*isReg: checks if token is an assembly register operand
parameters: token - pointer to string to be checked
return value: register number on success and -1 if token not a register*/
int isReg(char *token);

/*isIndirectReg: checks if token is an assembly indirect register operand
parameters: token - pointer to string to be checked
return value: register number on success and -1 if token not an indirect register*/
int isIndirectReg(char *token);

/*isInt: checks if token is an integer
parameters: token - pointer to string to be checked
return value: 1 on success and 0 if token is not an integer*/
int isInt(char *token);

/*isData: check if ptr is data directive
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 if ptr is not data; -1 on data synatx error and total words needed for data coding on success*/
int isData(char **ptr);

/*isString: checks if ptr is string directive
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 if ptr is not string; -1 on string synatx error and total words needed for string coding on success*/
int isString(char **ptr);

/*isImmediate: checks if token is an immediate number
parameters: token - pointer to string to be checked
return value: 1 on success; 0 if token is not immediate and -1 if error found (value out of range or illegal char)*/
int isImmediate(char *token);

/*isKeyword: checks if token is an assembly keyword
parameters: token - pointer to string to be checked
return value: 1 if token is an assembly keyword and 0 else*/
int isKeyword(char *token);

/*immedSize: checks if immediate is in range to fit in 12 bit
parameters: immediate - number to be checked
return value: 1 on success and 0 if immediate is out of range*/
int immedSize(int immediate);

/*dataSize: checks if data is in range to fit in 15 bit
parameters: data - number to be checked
return value: 1 on success and 0 if data is out of range*/
int dataSize(int data);

/*isExtern: checks if ptr is extern directive
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 if ptr is not extern; -1 on extern synatx error and 1 on success*/
int isExtern(char **ptr);

/*isEntry: checks if ptr is entry directive
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 if ptr is not entry; -1 on entry synatx error and 1 on success*/
int isEntry(char **ptr);

#endif /*ends preprocessor conditional*/