/*Shadi Zidany
Julian Jubran*/
/*second_pass.c: this file includes all relevant function used for 2nd pass on an assembly file
no need to check syntax cause first pass already did and if something went wrong assembler function would not
call second pass function.
in this file we done all instruction coding into words and all command operands type checked include labels
and update instruction table. directive thats not entry is ignored cause already first_pass updates the data*/

#include "parser.h"
#include "assembler.h"
#include "second_pass.h"

extern int IC, LC; /*instruction and line counters*/
extern char *fname; /*filename*/
extern FILE *file; /*file pointer*/
extern instruction *insthead; /*head of instruction linked list*/

/*secondPass: reads a line from assembly file and call other function to parse it till end-of-file
parameters: no arguments
return value: 0 if object file can be created else returns total errors found*/
int secondPass(void)
{
	char line[MAX_LINE];
	int err = 0;
	for (LC = 1; fgets(line, MAX_LINE, file); LC++) /*fgets returns a null pointer in end-of-file*/
	{
		if (ignore(line, file, FALSE))
			continue; /*line is whitespaces, comment or longer than max allowed*/
		else
		{
			char buffer[MAX_BUFFER] = { '\0' }; /*linetok func dest string*/
			char *token[MAX_LINE_ALLOWED + 1] = { NULL }; /*linetok func dest pointers*/
			err -= lineParser2(linetok(token, buffer, line));
		}
	}
	return err;
}

/*lineParser2: looks for entry directives or instructions and call relevant functions else line is ignored
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 on success and -1 if error found*/
int lineParser2(char **ptr)
{
	if (ptr[0][strlen(*ptr) - 1] == ':') /*label definition*/
		ptr++;
	if (**ptr == '.') /*directive*/
	{
		if (ptr[0][2] == 'n') /*.entry*/
		{
			int retval = CheckOnSymbolListAndFillVarList(ptr[1], TRUE);
			if (retval < MEMORY_ADDRESS) /*if label not found or not local*/
			{
				if (!retval) /*label address is 0 (external)*/
					fprintf(stderr, "%s: %d: error: label '%s' already designated as external\n", fname, LC, ptr[1]);
				else /*label not found*/
					fprintf(stderr, "%s: %d: error: undefined label -> '%s'\n", fname, LC, ptr[1]);
				return NEGATIVE;
			}
		}
		return FALSE;
	}
	return instCoding(iscmd(*ptr), ptr + 1);
}

/*instCoding: coded inst word and info word and updated instructions table
parameters: cmd - command number; ptr - pointer to line tokens (sub-strings)
return value: 0 on success and -1 if error found*/
int instCoding(unsigned cmd, char **ptr)
{
	static instruction *current; /*pointer to the node where to update instruction words*/
	unsigned srcOpAddMode, destOpAddMode, word[MAX_WORDS_IN_INST];
	if (IC == MEMORY_ADDRESS)
		current = insthead; /*reset the pointer*/
	word[0] = (cmd << opcode) | absolute; /*coding opcode and word1 always is absolute*/
	IC++;
	switch (cmd)
	{
	case mov:
	case cmp:
	case add:
	case sub:
	case lea:
		if (!(srcOpAddMode = isSrcOperand(cmd, word + 1, *ptr)))
			break; /*error on source operand*/
		IC++;
		if (!(destOpAddMode = isDestOperand(cmd, word + 2, ptr[2])))
			break; /*error on target operand*/
		word[0] |= (srcOpAddMode << src) | (destOpAddMode << dest); /*coding operands addressing mode*/
		if (ONE_WORD_REGISTERS) /*two registers operands share 1 word*/
		{
			word[1] |= word[2];
			current = UpdateAllWordsInList(current, word, MAX_WORDS_IN_INST - 1); /*update instruction table*/
		}
		else
		{
			IC++;
			current = UpdateAllWordsInList(current, word, MAX_WORDS_IN_INST); /*update instruction table*/
		}
		return FALSE;
	case clr:
	case not:
	case inc:
	case dec:
	case jmp:
	case bne:
	case red:
	case prn:
	case jsr:
		if (!(destOpAddMode = isDestOperand(cmd, word + 1, *ptr)))
			break; /*error on target operand*/
		IC++;
		word[0] |= destOpAddMode << dest; /*coding operand addressing mode*/
		current = UpdateAllWordsInList(current, word, MAX_WORDS_IN_INST - 1); /*update instruction table*/
		return FALSE;
	case rts:
	case stop:
		current = UpdateAllWordsInList(current, word, 1); /*update instruction table (word already coded)*/
		return FALSE;
	}
	return NEGATIVE;
}

/*isDestOperand: checks if target operand is legal to the given command and coded operand word
parameters: cmd - command number; word - pointer to operand info word; token - pointer to string (operand)
return value: operand type on success and 0 on error*/
int isDestOperand(unsigned cmd, unsigned *word, char *token)
{
	int labelAddress;
	if (*token == '#') /*immediate*/
	{
		if (cmdDestOpAllowed(cmd, immediate)) /*if target operand is valid for the command*/
		{
			int immed = atoi(++token);
			if (immed < 0)
				immed ^= -1 << sizeofImmediate; /*fit immediate in 12 bit*/
			*word = (immed << dest) | absolute; /*coding info word*/
			return immediate;
		}
		fprintf(stderr, "%s: %d: error: invalid destination operand (immediate)\n", fname, LC);
		return FALSE;
	}
	if (*token == '*') /*indirect register*/
	{
		if (cmdDestOpAllowed(cmd, indirectReg)) /*if target operand is valid for the command*/
		{
			*word = ((token[2] - '0') << dest) | absolute; /*coding info word*/
			return indirectReg;
		}
		fprintf(stderr, "%s: %d: error: invalid destination operand (indirect register)\n", fname, LC);
		return FALSE;
	}
	if (*token == 'r' && isReg(token) > -1) /*direct register*/
	{
		if (cmdDestOpAllowed(cmd, reg)) /*if target operand is valid for the command*/
		{
			*word = ((token[1] - '0') << dest) | absolute; /*coding info word*/
			return reg;
		}
		fprintf(stderr, "%s: %d: error: invalid destination operand (register)\n", fname, LC);
		return FALSE;
	}
	if ((labelAddress = CheckOnSymbolListAndFillVarList(token, 0)) > -1) /*label*/
	{
		if (cmdDestOpAllowed(cmd, label)) /*if target operand is valid for the command*/
		{
			if (!labelAddress) /*external operand*/
				*word = external; /*coding info word*/
			else
				*word = (labelAddress << dest) | relocatable; /*coding info word*/
			return label;
		}
		fprintf(stderr, "%s: %d: error: invalid destination operand (label)\n", fname, LC);
		return FALSE;
	}
	fprintf(stderr, "%s: %d: error: undefined label as destination operand -> '%s'\n", fname, LC, token);
	return FALSE; /*undefined label*/
}

/*isSrcOperand: checks if source operand is legal to the given command and coded operand word
parameters: cmd - command number; word - pointer to operand info word; token - pointer to string (operand)
return value: operand type on success and 0 on error*/
int isSrcOperand(unsigned cmd, unsigned *word, char *token)
{
	int labelAddress;
	if (*token == '#') /*immediate*/
	{
		if (cmdSrcOpAllowed(cmd, immediate)) /*if target source operand is valid for the command*/
		{
			int immed = atoi(++token);
			if (immed < 0)
				immed ^= -1 << sizeofImmediate; /*fit immediate in 12 bit*/
			*word = (immed << dest) | absolute; /*coding info word*/
			return immediate;
		}
		fprintf(stderr, "%s: %d: error: invalid source operand (immediate)\n", fname, LC);
		return FALSE;
	}
	if (*token == '*') /*indirect register*/
	{
		if (cmdSrcOpAllowed(cmd, indirectReg)) /*if target source operand is valid for the command*/
		{
			*word = ((token[2] - '0') << (src - 1)) | absolute; /*coding info word*/
			return indirectReg;
		}
		fprintf(stderr, "%s: %d: error: invalid source operand (indirect register)\n", fname, LC);
		return FALSE;
	}
	if (*token == 'r' && isReg(token) > -1) /*direct register*/
	{
		if (cmdSrcOpAllowed(cmd, reg)) /*if target source operand is valid for the command*/
		{
			*word = ((token[1] - '0') << (src - 1)) | absolute; /*coding info word*/
			return reg;
		}
		fprintf(stderr, "%s: %d: error: invalid source operand (register)\n", fname, LC);
		return FALSE;
	}
	if ((labelAddress = CheckOnSymbolListAndFillVarList(token, 0)) > -1) /*label*/
	{
		if (cmdSrcOpAllowed(cmd, label)) /*if target source operand is valid for the command*/
		{
			if (!labelAddress) /*external operand*/
				*word = external; /*coding info word*/
			else
				*word = (labelAddress << dest) | relocatable; /*coding info word*/
			return label;
		}
		fprintf(stderr, "%s: %d: error: invalid source operand (label)\n", fname, LC);
		return FALSE;
	}
	fprintf(stderr, "%s: %d: error: undefined label as source operand -> '%s'\n", fname, LC, token);
	return FALSE; /*undefined label*/
}

/*cmdSrcOpAllowed: checks if source operand type is valid in the given command
parameters: cmd - command number; srcOpAddMode - source operand type
return value: non-zero value if valid and 0 if invalid operand*/
int cmdSrcOpAllowed(unsigned cmd, unsigned srcOpAddMode)
{
	static unsigned srcOpAllowed[MAX_CMDS_HAVING_SRC_OP] =
	{ /*initializing source operand allowed table*/
		immediate | label | indirectReg | reg,
		immediate | label | indirectReg | reg,
		immediate | label | indirectReg | reg,
		immediate | label | indirectReg | reg,
		label
	}; /*& operator if 1 then valid operand. 0 invalid*/
	return srcOpAllowed[cmd] & srcOpAddMode;
}

/*cmdDestOpAllowed: checks if target operand type is valid in the given command
parameters: cmd - command number; destOpAddMode - target operand type
return value: non-zero value if valid and 0 if invalid operand*/
int cmdDestOpAllowed(unsigned cmd, unsigned destOpAddMode)
{
	static unsigned destOpAllowed[MAX_CMDS_HAVING_DEST_OP] =
	{ /*initializing target operand allowed table*/
		label | indirectReg | reg,
		immediate | label | indirectReg | reg,
		label | indirectReg | reg,
		label | indirectReg | reg,
		label | indirectReg | reg,
		label | indirectReg | reg,
		label | indirectReg | reg,
		label | indirectReg | reg,
		label | indirectReg | reg,
		label | indirectReg,
		label | indirectReg,
		label | indirectReg | reg,
		immediate | label | indirectReg | reg,
		label | indirectReg,
	}; /*& operator if 1 then valid operand. 0 invalid*/
	return destOpAllowed[cmd] & destOpAddMode;
}