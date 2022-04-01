/*Shadi Zidany
Julian Jubran*/
/*parser.c: this file includes multiple functions for syntax analysis for assembly language*/

#include "parser.h"

extern int LC; /*line counter*/
extern char *fname; /*filename*/

/*ignore: check if the given string is whitespaces, comment or longer than max allowed in assembly file
and adjust the file position to the next line if fgets() did not reads till end-of-line
parameters: line - pointer to string; file - file pointer where line reads from
printWarningMSG - 1: print warning message (in first pass) and 0: no warning message (second pass)
return value: 1 if string is empty, comment or longer than max allowed and 0 else*/
int ignore(char *line, FILE *file, int printWarningMSG)
{
	int len, i = MAX_LINE - 2; /*fgets read till MAX_LINE-1*/
	if (line[i]) /*char found*/
	{
		int chr = line[i];
		line[i] = '\0';
		while (chr != '\n' && (chr = fgetc(file)) != EOF); /*adjust file pointer to next line*/
	}
	for (i = 0; isspace(line[i]); i++);
	if (!line[i] || line[i] == ';')
		return TRUE; /*comment or empty line*/
	if (line[(len = strlen(line)) - 1] == '\n')
		len--;
	if (len > MAX_LINE_ALLOWED)
	{ /*line too long is ignored*/
		if (printWarningMSG) /*to prevent duplicate message in first and second pass*/
			fprintf(stderr, "%s: %d: warning: this line is ignored (too long)\n", fname, LC);
		return TRUE;
	}
	return FALSE;
}

/*trimStr: trim whitespaces on both ends of str
parameters: str - pointer to string
return value: string pointer to the first non-space character in str*/
char *trimStr(char *str)
{
	char *ptr = str + strlen(str) - 1;
	for (; isspace(*str); str++); /*trim left str*/
	for (; isspace(*ptr); ptr--); /*trim right str*/
	ptr[1] = '\0';
	return str;
}

/*linetok: splits a given line into separated tokens (sub-strings)
parameters: token - pointer to line tokens (sub-strings); buffer - pointer to where line will be copied and optimized
line - pointer to the original string reads from file
return value: pointer to the first token (sub-string) splitted in line*/
char **linetok(char **token, char *buffer, char *line)
{
	int i = 0;
	line = trimStr(line);
	if (*line == ',')
		buffer[i++] = *line++;
	while (*line)
	{ /*copies line to buffer*/
		switch (*line)
		{ /*optimize line for better strtok usage*/
		case ' ':
		case '\t':
			if (buffer[i - 1] != '\n')
				buffer[i++] = '\n';
			break;
		case ',':
			if (buffer[i - 1] == '\n')
			{
				buffer[i++] = ',';
				buffer[i++] = '\n';
			}
			else
			{
				buffer[i++] = '\n';
				buffer[i++] = ',';
				buffer[i++] = '\n';
			}
			break;
		case '"': /*copies the whole string as it is*/
			do
			{
				buffer[i++] = *line++;
			} while (*line && *line != '"');
			if (!*line)
				continue;
		default:
			buffer[i++] = *line;
		}
		line++;
	}
	*token = strtok(buffer, "\n"); /*splits line into tokens*/
	for (i = 0; token[i]; token[++i] = strtok(NULL, "\n")); /*every token[i] is pointer to a sub-string*/
	return token;
}

/*isLabelDef: check if token is a label definition
parameters: token - pointer to string to be checked
return value: 1 if token is a label definition; -1 on definition error and 0 if token is not a label definition*/
int isLabelDef(char *token)
{
	int len = strlen(token);
	if (len > 1 && token[len - 1] == ':') /*if label definition*/
	{
		if (--len < MAX_LABEL)
		{
			if (isalpha(*token)) /*if first char is alphabetic*/
			{
				token[len] = '\0';
				while (isalnum(*++token)); /*if token includes only alphanumeric chars*/
				if (!*token)
				{
					if (!isKeyword(token - len)) /*if not and assembley keyword (command, register, directive)*/
					{
						return TRUE;
					}
					fprintf(stderr, "%s: %d: error: invalid label (cannot be an assembly keyword)\n", fname, LC);
					return NEGATIVE;
				}
				fprintf(stderr, "%s: %d: error: invalid label (non-alphanumeric character)\n", fname, LC);
				return NEGATIVE;
			}
			fprintf(stderr, "%s: %d: error: invalid label (non-alphabetic first character)\n", fname, LC);
			return NEGATIVE;
		}
		fprintf(stderr, "%s: %d: error: invalid label (too long)\n", fname, LC);
		return NEGATIVE;
	}
	return FALSE;
}

/*iscmd: checks if token is an assembly command
parameters: token - pointer to string to be checked
return value: command decimal value on success and -1 if token is not a command*/
int iscmd(char *token)
{
	int i, len = strlen(token);
	static const char *cmd[] = { "mov", "cmp", "add", "sub", "lea",
		                         "clr", "not", "inc", "dec", "jmp",
								 "bne", "red", "prn", "jsr", "rts", NULL }; /*initialize commands array*/
	if (len == 3)
	{
		for (i = 0; cmd[i]; i++)
			if (!strcmp(token, cmd[i]))
				return i; /*return command opcode*/
	}
	else if (len == 4 && !strcmp(token, "stop"))
		return stop; /*return command opcode*/
	return NEGATIVE;
}

/*isReg: checks if token is an assembly register operand
parameters: token - pointer to string to be checked
return value: register number on success and -1 if token not a register*/
int isReg(char *token)
{
	if (strlen(token) == 2 && *token == 'r')
	{
		switch (*++token)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7': return *token - '0'; /*return register number*/
		}
	}
	return NEGATIVE;
}

/*isIndirectReg: checks if token is an assembly indirect register operand
parameters: token - pointer to string to be checked
return value: register number on success and -1 if token not an indirect register*/
int isIndirectReg(char *token)
{
	if (*token == '*')
		return isReg(++token); /*return register number or -1 on error*/
	return NEGATIVE;
}

/*isInt: checks if token is an integer
parameters: token - pointer to string to be checked
return value: 1 on success and 0 if token is not an integer*/
int isInt(char *token)
{
	if (*token == '-' || *token == '+')
	{
		if (strlen(token) > 1)
			while (isdigit(*++token));
	}
	else
		for (; isdigit(*token); token++);
	if (!*token) /*token is null: no illegal chars found*/
		return TRUE;
	return FALSE;
}

/*isData: check if ptr is data directive
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 if ptr is not data; -1 on data synatx error and total words needed for data coding on success*/
int isData(char **ptr)
{
	int cnt = 0;
	if (!strcmp(*ptr, ".data"))
	{
		int comma = 1; /* 1: IntegerExpected & 0: CommaExpected */
		while (*++ptr)
		{
			if (comma) /* IntegerExpected */
			{
				if (isInt(*ptr))
				{
					if (!dataSize(atoi(*ptr)))
						return NEGATIVE; /*data out of range*/
					cnt++;
					comma = 0; /*0: CommaExpected*/
					continue;
				}
				if (**ptr == ',')
				{
					if (!cnt) /* comma found at begin of data parameters */
						fprintf(stderr, "%s: %d: error: unexpected comma after '.data'\n", fname, LC);
					else
					{ /*at least two consecutive commas*/
						fprintf(stderr, "%s: %d: error: multiple consecutive commas", fname, LC);
						if (!ptr[1]) /*ptr+1 is null (end-of-statement)*/
							fprintf(stderr, " at end of statement");
						fputc('\n', stderr);
					}
				}
				else
					fprintf(stderr, "%s: %d: error: invalid parameter -> '%s'\n", fname, LC, *ptr);
				return NEGATIVE;
			}
			if (**ptr == ',') /* else CommaExpected */
			{
				comma = 1;
				continue;
			}
			fprintf(stderr, "%s: %d: error: missing comma\n", fname, LC);
			return NEGATIVE;
		}
		if (!cnt) /*no data found in directive*/
		{
			fprintf(stderr, "%s: %d: error: missing parameter(s)\n", fname, LC);
			return NEGATIVE;
		}
		if (comma) /* comma found at end of data statement */
		{
			fprintf(stderr, "%s: %d: error: illegal comma at end of statement\n", fname, LC);
			return NEGATIVE;
		}
	}
	return cnt;
}

/*isString: checks if ptr is string directive
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 if ptr is not string; -1 on string synatx error and total words needed for string coding on success*/
int isString(char **ptr)
{
	int len = 0;
	if (!strcmp(*ptr, ".string"))
	{
		char *str = *++ptr;
		if (!str)
		{
			fprintf(stderr, "%s: %d: error: missing string\n", fname, LC);
			return NEGATIVE; /*missing argument in directive*/
		}
		if ((len = strlen(str)) > 1 && str[0] == '"' && str[len - 1] == '"' && !*++ptr)
			return --len; /* if str = "" then sizeof(str) = 1 */
		fprintf(stderr, "%s: %d: error: invalid character(s)\n", fname, LC);
		return NEGATIVE;
	}
	return len;
}

/*isImmediate: checks if token is an immediate number
parameters: token - pointer to string to be checked
return value: 1 on success; 0 if token is not immediate and -1 if error found (value out of range or illegal char)*/
int isImmediate(char *token)
{
	if (*token == '#')
	{
		if (*++token)
		{
			if (isInt(token)) /*is integer*/
			{
				if (immedSize(atoi(token)))
					return TRUE;
				return NEGATIVE; /*immediate out of range*/
			}
			fprintf(stderr, "%s: %d: error: invalid character(s) -> '%s'\n", fname, LC, token);
			return NEGATIVE;
		}
		fprintf(stderr, "%s: %d: error: invalid operand (incomplete immediate #)\n", fname, LC);
		return NEGATIVE;
	}
	else if (isInt(token)) /*is integer without the char '#'*/
	{
		fprintf(stderr, "%s: %d: error: invalid operand (undefined addressing mode)\n", fname, LC);
		return NEGATIVE;
	}
	return FALSE;
}

/*isKeyword: checks if token is an assembly keyword
parameters: token - pointer to string to be checked
return value: 1 if token is an assembly keyword and 0 else*/
int isKeyword(char *token)
{
	int len = strlen(token);
	if (len <= MAX_KEYWORD)
	{
		switch (len)
		{
		case 2:
			if (isReg(token) > -1)
				return TRUE; /*token is register name*/
			break;
		case 3:
			if (iscmd(token) > -1)
				return TRUE; /*token is command name*/
			break;
		case 4:
			if (!strcmp(token, "data") || !strcmp(token, "stop"))
				return TRUE;
			break;
		case 5:
			if (!strcmp(token, "entry"))
				return TRUE;
			break;
		case 6:
			if (!strcmp(token, "extern") || !strcmp(token, "string"))
				return TRUE;
			break;
		}
	}
	return FALSE; /*not an assembly keyword*/
}

/*immedSize: checks if immediate is in range to fit in 12 bit
parameters: immediate - number to be checked
return value: 1 on success and 0 if immediate is out of range*/
int immedSize(int immediate)
{
	if (immediate > MAX_IMMEDIATE)
	{ /*immediate out of range*/
		fprintf(stderr, "%s: %d: error: invalid operand (positive immediate too high to fit in 12 bits)\n", fname, LC);
		return FALSE;
	}
	if (immediate < MIN_IMMEDIATE)
	{ /*immediate out of range*/
		fprintf(stderr, "%s: %d: error: invalid operand (negative immediate too low to fit in 12 bits)\n", fname, LC);
		return FALSE;
	}
	return TRUE; /*immediate is in range*/
}

/*dataSize: checks if data is in range to fit in 15 bit
parameters: data - number to be checked
return value: 1 on success and 0 if data is out of range*/
int dataSize(int data)
{
	if (data > MAX_DATA)
	{ /*data out of range*/
		fprintf(stderr, "%s: %d: error: data overflow (positive value too high to fit in 15 bits)\n", fname, LC);
		return FALSE;
	}
	if (data < MIN_DATA)
	{ /*data out of range*/
		fprintf(stderr, "%s: %d: error: data overflow (negative value too low to fit in 15 bits)\n", fname, LC);
		return FALSE;
	}
	return TRUE; /*data is in range*/
}

/*isExtern: checks if ptr is extern directive
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 if ptr is not extern; -1 on extern synatx error and 1 on success*/
int isExtern(char **ptr)
{
	if (!strcmp(".extern", *ptr))
	{
		if (*++ptr)
		{
			if (!ispunct(**ptr))
			{
				if (!*++ptr) /*if the directive argument is the last token in the statement*/
					return TRUE; /*external label is defined in other file. no need to check label name grammar here*/
			}
			fprintf(stderr, "%s: %d: error: invalid character(s) -> '%s'\n", fname, LC, *ptr);
			return NEGATIVE; /*non-alphanumeric char*/
		}
		fprintf(stderr, "%s: %d: error: missing argument in directive\n", fname, LC);
		return NEGATIVE; /*ptr is null after .extern*/
	}
	return FALSE;
}

/*isEntry: checks if ptr is entry directive
parameters: ptr - pointer to line tokens (sub-strings)
return value: 0 if ptr is not entry; -1 on entry synatx error and 1 on success*/
int isEntry(char **ptr)
{
	if (!strcmp(".entry", *ptr))
	{
		if (*++ptr)
		{
			if (!ispunct(**ptr))
			{
				if (!*++ptr) /*if the directive argument is the last token in the statement*/
					return TRUE; /*2nd pass will deal with label name and definition*/
			}
			fprintf(stderr, "%s: %d: error: invalid character(s) -> '%s'\n", fname, LC, *ptr);
			return NEGATIVE; /*non-alphanumeric char*/
		}
		fprintf(stderr, "%s: %d: error: missing argument in directive\n", fname, LC);
		return NEGATIVE; /*ptr is null after .entry*/
	}
	return FALSE;
}
