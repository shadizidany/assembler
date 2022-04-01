/*Shadi Zidany
Julian Jubran*/
#ifndef UTILITIES_H /*returns true if this macro is not defined*/
#define UTILITIES_H /*substitutes a preprocessor macro*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NEGATIVE -1
#define FALSE 0
#define TRUE 1
#define FILE_CREATION_ERROR -4 /*file creation error*/
#define MALLOC_ERROR -2 /*memory allocation failed*/
#define SUCCESS 2 
#define MAX_MEMORY 4096 /*max stack described in the project*/
#define MEMORY_ADDRESS 100 /*start of memory addres for program loading*/
#define MAX_WORDS_IN_INST 3 /*maximum words in command (instruction)*/
#define MAX_LINE 128 /*maximum line length to read from assembly file*/
#define MAX_LINE_ALLOWED 80 /*maximum line length allowed in assembly file*/
#define MAX_BUFFER 256 /*maximum buffer length for line optimization*/
#define MAX_LABEL 32 /*maximum label name length*/
#define MAX_FILENAME 64 /*maximum filename length*/
#define MAX_FILE_FORMAT 4 /*max file format len like (.as, .ob, etc..)*/
#define FILE_FORMAT (strstr(*argv, ".as")) ? "" : ".as" /*adding file format for fprintf if ".as" does not exist*/
#define MAX_KEYWORD 6 /*maximum assembly keyword length*/
#define MAX_DATA 16383 /*maximum positive data value to fit in 15 bit*/
#define MIN_DATA -16384 /*minimum negative data value to fit in 15 bit*/
#define MAX_IMMEDIATE 2047 /*maximum positive immediate value to fit in 12 bit*/
#define MIN_IMMEDIATE -2048 /*minimum negative immediate value to fit in 12 bit*/
#define sizeofData 15 /*data number fits in 15 bit*/
#define sizeofImmediate 12 /*immediate number fits in 12 bit*/
#define MAX_CMDS_HAVING_SRC_OP 5 /*maximum commands having source operand*/
#define MAX_CMDS_HAVING_DEST_OP 14 /*maximum commands having destination (target) operand*/
#define ONE_WORD_REGISTERS (srcOpAddMode == indirectReg || srcOpAddMode == reg) &&\
	(destOpAddMode == indirectReg || destOpAddMode == reg)
/*if two operands are registers (direct or indirect) then they shared one info word*/

/*commands decimal value (opcode)*/
enum cmd { mov, cmp, add, sub, lea, clr, not, inc, dec, jmp, bne, red, prn, jsr, rts, stop };
enum bitField { absRelExt, dest = 3, src = 7, opcode = 11 }; /*bit field of an instruction word*/
enum addressMode { immediate = 1, label, indirectReg = 4, reg = 8 }; /*instruction operands type*/
enum AbsRelExt { external = 1, relocatable, absolute = 4 }; /*addressing mode*/

/*Struct for save information about external and entry variables
char * Variable Name : pointer for string that save name of variable
int external : if equal to 1 that's mean the variable is extrnal else 0 
unsigned dec_add; : to save the dec address
One-way linked list*/
typedef struct variables
{
	char * variableName;
	int external;/*if equal to 1 thats mean variable it's external  if it's 0 thats mean it's entry*/
	unsigned dec_add;
	struct variables * next;
} variables;

/*Struct ro save data (all information about .data and .string)
unsigned word : to save the word in 15 bits
unsigned dec_add: to save dec address
Two-way circular linked list*/
typedef struct data
{
	unsigned word, dec_add;
	struct data *next, *prev;
} data;

/*struct to save instruction all word and dec_address for all line from input
unsigned word[MAX_WORDS_IN_INST], dec_add[MAX_WORDS_IN_INST]: to save 
the word and the dec address for word 
the max is 3 word 
Two-way circular linked list*/

typedef struct instruction
{
	unsigned word[MAX_WORDS_IN_INST], dec_add[MAX_WORDS_IN_INST];
	struct instruction *next, *prev;
} instruction;

/*struct to save the symbol table information
char label[MAX_LABEL]: to save label 
unsigned dec_add : to save dec address 
directive : if  label is directive that will be equal to 1 else 0
external : if label is external that will be equal to 1 else 0
One-way linked list
*/
typedef struct symbol
{
	char label[MAX_LABEL];
	unsigned dec_add, directive, external;
	struct symbol *next;
} symbol;

#endif /*ends preprocessor conditional*/