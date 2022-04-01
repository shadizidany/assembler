/*Shadi Zidany
Julian Jubran*/
/*first_pass.c: this file are responsible for assembler first pass of two
in the first pass we check assembly language syntax and add all labels and directives to linked list
if entry directive we only check that statement is ok with no errors because second pass will deal with entry
in all instruction we check that instruction name is defined and number of operands for the given command
is appropriate to the command (second pass will check operands type) and update data and instructions counters*/

#include "parser.h"
#include "assembler.h"
#include "first_pass.h"

extern int LC; /*line counter*/
extern char *fname; /*filename*/
extern FILE *file; /*file pointer*/
extern symbol *symbolhead; /*head of symbol (label) linked list*/
extern data *datahead; /*head of data linked list*/
extern instruction *insthead; /*head of instruction linked list*/

/*firstPass: reads a line from assembly file and call other function to parse it till end-of-file
parameters: no arguments
return value: 0 if second pass can be executed else returns total errors found*/
int firstPass(void)
{
	char line[MAX_LINE] = { '\0' };
	int err = 0;
	for (LC = 1; fgets(line, MAX_LINE, file); LC++) /*fgets returns a null pointer in end-of-file*/
	{
		if (ignore(line, file, TRUE)) /*if line len is longer than max allowed a warning msg will sent to stderr*/
			continue; /*line is whitespaces, comment or longer than max allowed*/
		else
		{
			char buffer[MAX_BUFFER] = { '\0' }; /*linetok func dest string*/
			char *token[MAX_LINE_ALLOWED + 1] = { NULL }; /*linetok func dest pointers*/
			err -= lineParser1(linetok(token, buffer, line));
		}
	}
	return err;
}

/*lineParser1: checks ptr and calls relevant functions depends on the given statement
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 on success and -1 if error found*/
int lineParser1(char **ptr)
{
	int label = isLabelDef(*ptr);
	if (label < 0)
		return NEGATIVE; /*isLabelDef discovered an error in the definition*/
	if (label && !*++ptr)
	{
		fprintf(stderr, "%s: %d: error: invalid label (incomplete definition)\n", fname, LC);
		return NEGATIVE; /*label definition without directive or instruction*/
	}
	if (**ptr == '.') 
		return directive(ptr, label); /*directive found*/
	return instChecker(ptr, label); /*check for inst*/
}

/*directive: checks the directive statement and syntax in ptr and call relevant function to update data table
parameters: ptr - pointer to line tokens; label - indicates if label is defined or not and update symbol table
return value: 0 on success and -1 if error found*/
int directive(char **ptr, int label)
{
	char *str = *ptr;
	int word = 0;
	switch (*++str)
	{
	case 'd': /*.data*/
		if (!(word = isData(ptr)) || word < 0)
			break;
		if (label) /*add label to symbol table*/
		{
			if (AddNewLabelToList(&symbolhead, ptr[-1], TRUE, FALSE))
				return NEGATIVE; /* label already defined */
		}
		return updateData(++ptr, word); /*update data table*/
	case 's': /*.string*/
		if (!(word = isString(ptr)) || word < 0)
			break;
		if (label) /*add label to symbol table*/
		{
			if (AddNewLabelToList(&symbolhead, ptr[-1], TRUE, FALSE))
				return NEGATIVE; /* label already defined */
		}
		str = *++ptr;
		str[word] = '\0';
		AddNewStringToList(&datahead, ++str); /*update data table*/
		return FALSE;
	case 'e':
		if (*++str == 'n') /*.entry*/
		{
			if (!(word = isEntry(ptr)) || word < 0)
				break;
			if (label)
				fprintf(stderr, "%s: %d: warning: label '%s' declaration ignored\n", fname, LC, ptr[-1]);
			return FALSE;
		}
		if (*str == 'x') /*.extern*/
		{
			if (!(word = isExtern(ptr)) || word < 0)
				break;
			if (label)
				fprintf(stderr, "%s: %d: warning: label '%s' declaration ignored\n", fname, LC, ptr[-1]);
			return AddNewLabelToList(&symbolhead, ptr[1], FALSE, TRUE); /*add extern label to symbol table*/
		}
	}
	if (!word)
		fprintf(stderr, "%s: %d: error: undefined directive\n", fname, LC);
	return NEGATIVE;
}

/*updateData: add all .data parameters (numbers) to data table
parameters: ptr - pointer to line tokens (sub-strings); len - total parameters in directive
return value: 0 on success or call terminate function to terminate the program if malloc failed*/
int updateData(char **ptr, int len)
{
	int i, *arr = (int *)malloc(sizeof(int) * len);
	if (!arr)
		terminate(MALLOC_ERROR); /*memory allocation failed*/
	for (i = 0; *ptr; ptr++)
	{
		if (**ptr == ',')
			continue;
		if ((arr[i] = atoi(*ptr)) < 0)
			arr[i] ^= -1 << sizeofData; /* fit data in 15 bit */
		i++;
	}
	AddNewDataToList(&datahead, arr, len); /*update data table*/
	free(arr);
	return FALSE;
}

/*instChecker: checks if ptr is a valid command and calls relevant functions to check operands
parameters: ptr - pointer to line tokens; label - indicates if label is defined or not and update symbol table
return value: 0 on success and -1 if error found*/
int instChecker(char **ptr, int label)
{
	int cmd = iscmd(*ptr);
	if (cmd > -1)
	{
		if (label) /*add label to symbol table*/
		{
			if (AddNewLabelToList(&symbolhead, ptr[-1], FALSE, FALSE))
				return -1; /* label already defined */
		}
		switch (cmd)
		{
		case mov:
		case cmp:
		case add:
		case sub:
		case lea: return twoOperandsInst(++ptr); /*instructions needs two perands*/
		case clr:
		case not:
		case inc:
		case dec:
		case jmp:
		case bne:
		case red:
		case prn:
		case jsr: return oneOperandInst(++ptr); /*instructions needs only one perand*/
		case rts:
		case stop: /*instructions without perands*/
			if (ptr[1])
			{
				fprintf(stderr, "%s: %d: error: invalid character(s)\n", fname, LC);
				return NEGATIVE; /*string found after inst name*/
			}
			AddNewWordToList(&insthead, 1); /*update inst table (only 1 word needed)*/
			return FALSE;
		}
	}
	fprintf(stderr, "%s: %d: error: undefined instruction\n", fname, LC);
	return NEGATIVE;
}

/*twoOperandsInst: checks if ptr contains two valid operands and right syntax and update instruction table
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 on success and -1 if error found*/
int twoOperandsInst(char **ptr)
{
	int i, op[2] = { 0 }; /*op[0] for register operand and op[1] for label and immediate operand*/
	for (i = 0; *ptr; i++, ptr++)
	{
		switch (i) /*statement pointer: [operand][comma][operand]*/
		{
		case 0:
		case 2:
			if (**ptr == '#') /*immediate number*/
			{
				if (isImmediate(*ptr) < 0)
					return NEGATIVE;
				op[1]++;
				continue;
			}
			if (**ptr == '*') /*indirect register*/
			{
				if (isIndirectReg(*ptr) < 0)
				{
					fprintf(stderr, "%s: %d: error: invalid operand (incomplete indirect *)\n", fname, LC);
					return NEGATIVE;
				}
				op[0]++;
				continue;
			}
			if (**ptr == 'r' && isReg(*ptr) > -1) /*direct register*/
			{
				op[0]++;
				continue;
			}
			if (isalpha(**ptr)) /* possibly label (2nd pass will deal with it) */
			{
				op[1]++;
				continue;
			}
			fprintf(stderr, "%s: %d: error: invalid operand\n", fname, LC);
			return NEGATIVE;
		case 1: /*comma expected*/
			if (**ptr == ',')
				continue;
			fprintf(stderr, "%s: %d: error: missing comma\n", fname, LC);
			return NEGATIVE;
		default: /*string found after target operand*/
			fprintf(stderr, "%s: %d: error: invalid character(s)\n", fname, LC);
			return NEGATIVE;
		}
	}
	if ((op[0] + op[1]) < 2)
	{
		fprintf(stderr, "%s: %d: error: missing operand(s)\n", fname, LC);
		return NEGATIVE;
	}
	if (op[1])
		AddNewWordToList(&insthead, MAX_WORDS_IN_INST); /* 3 words needed (inst + 2 operands) */
	else
		AddNewWordToList(&insthead, MAX_WORDS_IN_INST - 1); /* 2 words needed (inst + 2 reg's) */
	return FALSE;
}

/*oneOperandInst: checks if ptr contains a valid operand and right syntax and update instruction table
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 on success and -1 if error found*/
int oneOperandInst(char **ptr)
{
	if (!*ptr) /*string is null after inst name*/
	{
		fprintf(stderr, "%s: %d: error: missing operand\n", fname, LC);
		return NEGATIVE;
	}
	switch (**ptr)
	{
	case '#': /*immediate number*/
		if (isImmediate(*ptr) < 0)
			return NEGATIVE;
		break;
	case '*': /*indirect register*/
		if (isIndirectReg(*ptr) < 0)
		{
			fprintf(stderr, "%s: %d: error: invalid operand (incomplete indirect *)\n", fname, LC);
			return NEGATIVE;
		}
		break;
	default:
		if (isalpha(**ptr)) /*possibly direct register or label (2nd pass will deal with it)*/
			break;
		fprintf(stderr, "%s: %d: error: invalid operand\n", fname, LC);
		return NEGATIVE;
	}
	if (ptr[1]) /*string found after target operand*/
	{
		fprintf(stderr, "%s: %d: error: invalid character(s)\n", fname, LC);
		return NEGATIVE;
	}
	AddNewWordToList(&insthead, MAX_WORDS_IN_INST - 1); /* 2 words needed (inst + 1 operand) */
	return FALSE;
}