/*Shadi Zidany
Julian Jubran
assembler.c: this program is an assembler for assembly language
arguments: assembly source filename
return value: if no errors in source file the program will generate successfully 3 output file
1: object file for machine code
2: entry file if entry directives exist
3: external file if extern directives exist
if error found in source code the program will prints out all errors found and will not creat any files*/

#include "assembler.h"
#include "first_pass.h"
#include "second_pass.h"

int DC, IC, LC;/*DC: data counter, IC: instruction counter, LC: line counter*/
char *fname; /* File name (with full path) */
FILE *file;
/*head of all lists (symbol,data,instruction,variables) and start them with NULL*/
symbol *symbolhead;
data *datahead ;
instruction *insthead ;
variables *varhead;

/*main function that called assembler with path of files that pass on arguments*/
int main(int argc, char **argv)
{
	if (argc < 2) /* no arguments passed */
		fprintf(stderr, "missing command line arguments\n");
	else
	{
		while (*++argv) /*loop sent the path of file*/
		{
			if (strlen(*argv) > MAX_FILENAME)
			{
				fprintf(stderr, "%s%s: warning: this file is ignored (filename too long)\n", *argv, FILE_FORMAT);
				continue;
			}
			printf("-------------------------------------------------\n");
			printf("Compiling the file [%s] in process....\n", *argv);
			if (strstr(*argv, ".as"))
				assembler(*argv);
			else
			{
				char filename[MAX_FILENAME + MAX_FILE_FORMAT];
				assembler(strcat(strcpy(filename, *argv), ".as"));
			}
			printf("Compile the file [%s] has been finished\n",*argv);
			printf("-------------------------------------------------\n");

		}
	}
	return FALSE;
}

/*function that get open file that pass as argument and called 
  the functions first pass and second pass also called the fix functions that fixed the address in list 
  create and fill the output files
  arguments:string filename (path of file)
  returned value :no returned value */
void assembler(char *filename)
{
	if (!(file = fopen(filename, "r"))) /* opens a file for reading */
	{
		fprintf(stderr, "%s might not exist or permission error occurred while accessing the file\n", filename);
		return; /* the file cannot be opened, a null value is returned */
	}
	fname = filename;
	/*set NULL in all head list after free them*/
	symbolhead = NULL;
	datahead = NULL;
	insthead = NULL;
	varhead = NULL;
	DC = 0; IC = MEMORY_ADDRESS;
	if (!firstPass())/*if the file pass the (first pass)*/
	{
		if ((DC + IC) < MAX_MEMORY)
		{
			/*fix the addess value in symbol and data list*/
			FixDirectiveNodes(symbolhead);
			FixDecAddressNodes(datahead);
			IC = MEMORY_ADDRESS;
			rewind(file);
			if (!secondPass())/*if the file pass the (second pass)*/
			{
				/*create the output file (obj,ext,ent)*/
				printObjFile(filename);
				printExtAndEntFiles(filename);
			}
		}
		else
			fprintf(stderr, "%s: error: memory overflow (program too big to fit into %d words)\n", fname, MAX_MEMORY);
	}
	/*free lists and close file*/
	terminate(SUCCESS);
}

/*function that free all lists (symbol,data,instruction,variables) also set NULL on it 
in addition close the(input file) 
arguments:int failure .if it's equal to 1 that mean malloc failed if it's equal to 2 failed to create (output file )
returned value :no returned value */
void terminate(int failure)
{
	/*free all list */
	FreeLabelList(symbolhead);
	FreeDataList(datahead);
	FreeInstructionList(insthead);
	FreeVariableList(varhead);
	fclose(file);
	if (failure < 0)
	{
		if (failure == MALLOC_ERROR)
			fprintf(stderr, "memory allocation failed\n");
		exit(EXIT_SUCCESS); /*exit(0) indicates successful program termination*/
	}
}

/*function that add new node  for symbol list and fill all fields the same argurments that pass
arguments: head - head of symbol list , label - string that save label (in assembler) ,directive if the line is directive or no (1 or 0) 
external - if label is external (equal to 1 that mean is external else it's not external )
returned value : -1 if it's failed , 0 if it's pass*/
int AddNewLabelToList(symbol **head, char * Label, unsigned directive, unsigned external){
	/*pointers of symbol struct*/
	symbol *ptr, *temp, *tail = NULL;
	ptr = malloc(sizeof(symbol));
	/*if malloc failed*/
	if (ptr == NULL){
		terminate(MALLOC_ERROR);
	}
	/*fill all fields */
	ptr->next = NULL;
	ptr->directive = directive;
	ptr->external = external;
	strcpy(ptr->label, Label);

	if (directive == 1)/*if it's directive use the DC variables*/
		ptr->dec_add = DC;

	if (directive == 0)/*if it's not directive use the IC variables*/
		ptr->dec_add = IC;

	if (external)/*if it's external put a 0*/
		ptr->dec_add = 0;

	if ((*head) == NULL){/*if list is empty*/

		(*head) = ptr;
		return FALSE;
	}
	temp = (*head);
	while (temp != NULL)/*go to last node in list*/
	{
		if (!strcmp(temp->label, Label)){
			free(ptr);
			if (!temp->external)
			{
				if (external)
					fprintf(stderr, "%s: %d: error: local label cannot be declared as external\n", fname, LC);
				else/*if label already declared */
					fprintf(stderr, "%s: %d: error: label was already declared earlier\n", fname, LC);
				return NEGATIVE;
			}
			return FALSE;
		}

		tail = temp;
		temp = temp->next;
	}
	tail->next = ptr;
	return FALSE;

}

/* Recursive function that free all list (symbol list)
arguments: head of list (symbol list)
returned value : no returned value*/
void FreeLabelList(symbol *head){
	if (head == NULL)
		return;
	FreeLabelList(head->next);/*call same function*/
	free(head);
}

/* function that fix all dec_address in (symbol list)
if the node is directive add the dec_address with IC 
arguments: head of list (symbol list)
returned value : no returned value*/
void FixDirectiveNodes(symbol *head)
{
	while (head)/*pass in all list*/
	{
		if (head->directive == 1)
			head->dec_add += IC;
		head = head->next;
	}
}

/*function that add new node  for data list and  (add all array values) all 
add new node it's prev of head  
arguments: head - head of data list , array and size of array 
returned value : 1 if pass else call terminate function*/
int AddNewDataToList(data **head, int *array, int size){
	/*pointers of data struct*/
	data *ptr, *temp;
	int i;/*used in for loop*/
	/*Loop pass on all elements on array and put it in the list*/
	for (i = 0; i < size; i++){
		ptr = malloc(sizeof(symbol));
		if (ptr == NULL){
			free(array);
			terminate(MALLOC_ERROR);
		}

		ptr->word = array[i];
		ptr->dec_add = DC++;
		if ((*head) == NULL){/*if list is empty*/
			ptr->next = ptr->prev = ptr;
			(*head) = ptr;
			continue;
		}
		if ((*head)->prev == (*head))/*just one node in list*/
		{
			(*head)->next = (*head)->prev = ptr;
			ptr->prev = ptr->next = (*head);
			continue;
		}
		/*if list more then one node added new node to head prev (before head)*/
		temp = (*head)->prev;
		(*head)->prev = ptr;
		temp->next = ptr;
		ptr->next = (*head);
		ptr->prev = temp;
	}
	return TRUE;
}

/* function that free all list (data list)
arguments: head of list (data list)
returned value : no returned value*/
void FreeDataList(data *head){
	/*pointer of data struct*/
	data *tail;
	if (head == NULL)/*if list already empty*/
		return;
	tail = head->prev;
	while (head != tail)
	{
		head = head->next;
		free(head->prev);
	}
	free(head);/*free last node*/
}

/*function that add new node  for data list and  (add all string values using the strlen) 
all add new node it's prev of head
arguments: head - head of data list , string 
returned value : 1 if pass else call terminate function*/
int AddNewStringToList(data **head, char * string){
	/*pointers of data struct*/
	data *ptr, *temp;
	int i;/*used in for loop*/
	/*Loop pass on all elements on array and put it in the list*/
	for (i = 0; i < strlen(string) + 1; i++){
		ptr = malloc(sizeof(symbol));
		if (ptr == NULL){
			terminate(MALLOC_ERROR);
		}

		ptr->word = string[i];
		ptr->dec_add = DC++;
		if ((*head) == NULL){/*if list is empty*/
			ptr->next = ptr->prev = ptr;
			(*head) = ptr;
			continue;
		}
		if ((*head)->prev == (*head))/*just one node in list*/
		{
			(*head)->next = (*head)->prev = ptr;
			ptr->prev = ptr->next = (*head);
			continue;
		}
		/*if list more then one node added new node to head prev (before head)*/
		temp = (*head)->prev;
		(*head)->prev = ptr;
		temp->next = ptr;
		ptr->next = (*head);
		ptr->prev = temp;
	}

	return TRUE;
}

/* function that fix all dec_address in (data list)
add the dec_address with IC
arguments: head of list (data list)
returned value : no returned value*/
void FixDecAddressNodes(data *head)
{
	/*pointer of data struct*/
	data *tail;
	if (head == NULL)/*if there is no data with the first pass*/
		return;
	tail = head->prev;
	while (head != tail)/*all nodes*/
	{

		head->dec_add += IC;
		head = head->next;
	}
	/*last node*/
	head->dec_add += IC;
}

/* function that set 0 in all array elements
arguments: pointer for array and size of array
returned value : no returned value*/
void ResetArray(unsigned Array[], int size)
{
	int i;/*used in for loop*/
	for (i = 0; i < size; i++)
		Array[i] = 0;
}

/*function that add new node  for instruction list in word is empty that mean 0 
because this function called in the first pass 
in dec_address set IC  in addition increment IC
add new node it's prev of head
arguments: head - head of instruction list amount of word  for example 1 or 2 or 3 words
returned value : 1 if pass else call terminate function*/
int AddNewWordToList(instruction **head, int Amount)
{
	/*pointers of instruction struct*/
	instruction *ptr, *temp;
	int i;/*used in for loop*/
	ptr = malloc(sizeof(instruction));
	if (ptr == NULL){
		terminate(MALLOC_ERROR);
	}
	/*Reset all array (put 0 for all elements)*/
	ResetArray(ptr->dec_add, MAX_WORDS_IN_INST);
	ResetArray(ptr->word, MAX_WORDS_IN_INST);

	for (i = 0; i < Amount; i++)
	{
		ptr->word[i] = 0;
		ptr->dec_add[i] = IC++;
	}
	if ((*head) == NULL){/*if list is empty*/
		ptr->next = ptr->prev = ptr;
		(*head) = ptr;
		return TRUE;

	}
	if ((*head)->prev == (*head))/*just one node in list*/
	{
		(*head)->next = (*head)->prev = ptr;
		ptr->prev = ptr->next = (*head);
		return TRUE;
	}
	/*if list more then one node added new node to head prev (before head)*/
	temp = (*head)->prev;
	(*head)->prev = ptr;
	temp->next = ptr;
	ptr->next = (*head);
	ptr->prev = temp;

	return TRUE;
}

/* function that free all list (instruction list)
arguments: head of list (instruction list)
returned value : no returned value*/
void FreeInstructionList(instruction *head){
	instruction *tail;/*pointer of instruction struct*/
	if (head == NULL)/*if list is already empty*/
		return;
	tail = head->prev;
	while (head != tail) /* all nodes*/
	{
		head = head->next;
		free(head->prev);
	}
	free(head);/*last noce*/
}

/* function that update node that get from argument (array) it's update the word variable in node
arguments: node of list (instruction list) , array that include the value and the size of array
returned value : returned the next of the node(that got from argument)*/
instruction * UpdateAllWordsInList(instruction * node, unsigned Array[], int size)
{
	int i;/*used in for loop*/
	if (node == NULL)/*if list is empty */
		return NULL;
	for (i = 0; i<size; i++)/*update the (word array) with the pass argument (array)*/
	{
		node->word[i] = Array[i];
	}
	return node->next;
}

/*function that add new node  for variables list with the pass arguments
used static that save the tail of the list to add all times after this node
arguments: isextrn - if extrnal  equal to 1 else 0 , address and string that is the name of the variable
returned value : 1 if pass else call terminate function*/
int AddNewVariableToList(int isextrn,int address,char * string){
	static variables *lastNode = NULL; /*static variable pointer no save last node*/
	variables *temp;/*variable pointer of struct*/
	temp = malloc(sizeof(variables));

	if (temp == NULL)
	{
		terminate(MALLOC_ERROR);
	}
	/*fill all fields*/
	temp->next = NULL;
	temp->variableName = string;
	temp->dec_add = address;
	temp->external = isextrn;

	if (varhead == NULL){
		/*if list is empty*/
		varhead = temp;
		lastNode = temp;
		return TRUE;
	}
	if (varhead->next == NULL){
		/*there is one node*/
		varhead->next = temp;
		lastNode = temp;
		return TRUE;
	}
	/*adding for the last node*/
	lastNode->next = temp;
	lastNode = lastNode->next;
	return TRUE;


}

/* Recursive function that free all list (variables list)
arguments: head of list (variables list)
returned value : no returned value*/
void FreeVariableList(variables *head)
{
	if (head == NULL)
		return;
	FreeVariableList(head->next);
	free(head);
}

/* function that go over on symbol list and check if label (pass in argument) and check about 
isentry  if it is entrnal  called the AddNewVariableToList function  and added it to variable list
with label and the dec_address that was in symbol list
if is external called the AddNewVariableToList function and added it to variable list with label
and with the currecnt IC becuase it's called in second pass
arguments: label-string that need to searvh , isentry- if equal to 1 that mean it's entry variable 
else it's an external variable
returned value : returned the dec_address if the label else -1 if there any problem */
int CheckOnSymbolListAndFillVarList(char Label[], int isentry){
	int flag = 0;/*flag to know if found the label in list*/
	symbol *ptr;/*pointer of symbol struct*/
	if (symbolhead == NULL)/*if list is empty*/
		return NEGATIVE;
	ptr = symbolhead;
	while (ptr->next != NULL)
	{/*pass and check the label on list to last node*/
		if (!strcmp(ptr->label, Label)){
			flag = 1;
			break;
		}
			
		ptr = ptr->next;
	}
	if (!flag&&strcmp(ptr->label, Label))/*if label of last node is not equal to label*/
		return NEGATIVE;
	if (isentry)/*label after .enrty*/
	{
		if (ptr->external == 0)
			AddNewVariableToList(0, ptr->dec_add, ptr->label);
		return ptr->dec_add;
	}
	if (ptr->external)
		/*if variable is external got the current IC*/
		AddNewVariableToList(1, IC, ptr->label);
	/*returned dec_address of label*/
	return ptr->dec_add;
}

/* function that change the end of path file it's get as argument the path as string
and also the new end that need to concatenate (end of file like .obj/.ext/.ent),
it's use also malloc and the free function use on function that call it , (printExtAndEntFiles,printObjFile)
arguments: oldStr- string is path of file  newEnd-string is new end need to concatenate (end of file like .obj/.ext/.ent)
returned value : pointer of new string after all changed */
char * ChangeEndOfFile(char *oldStr,char * newEnd){
	char *ptr;/*pointer of char*/
	ptr = malloc((strlen(oldStr) + 1)*sizeof(char *));/*malloc to same size of path*/
	if (ptr == NULL)
	{
		terminate(MALLOC_ERROR);
	}
	strcpy(ptr, oldStr);
	*strrchr(ptr, '.') = '\0'; /*search on dot and cut it*/
	strcat(ptr, newEnd);/*concatenate (end of file like .obj/.ext/.ent)*/
	return ptr;
}

/*function that get a pointer of FILE and path then it's create a file with W mode (write mode)
if it's failed call terminate function that close the program
arguments: file-pointer of pointer (FILE) and path-string path of new file that need to create
returned value : no returned value*/
void CreateFile(FILE **file,char *path){
	(*file) = fopen(path, "w");
	if ((*file) == NULL){
		fprintf(stderr, "file '%s' creation failed",path);
		terminate(FILE_CREATION_ERROR);
	}

}


/*function that go over variable list (this list have all information about variables entrnal,external)
if met first variable of (entrnal,external) it's changed the path then create the file same type of variable 
then it's print all information on the file that created
it's created two files  (.ent and .ext) and fill all information
arguments:string filename (path of file)
returned value :no returned value */
void printExtAndEntFiles(char *pathfile){
	FILE * entryFile, *extrnalFile;/*pointers if FILE*/
	int entryFlag, extrnalFlag;/*flag to know the first met of variable to create*/
	variables *ptr;/*pointer of variable struct*/
	char *newPath = NULL;/*pointer of char*/
	entryFile = extrnalFile = NULL;
	entryFlag = extrnalFlag = 0;
	if (varhead == NULL)/*if list is empty*/
		return;
	ptr = varhead;
	while (ptr)/*loop go over variable list and print on files*/
	{
		if (ptr->external)/*if variable is external*/
		{
			if (!extrnalFlag){/*if it's the first time need to create file with the end .ext */
				extrnalFlag = 1;
				newPath = ChangeEndOfFile(pathfile, ".ext");
				CreateFile(&extrnalFile, newPath);
				fprintf(extrnalFile, "%-33s%04d\n", ptr->variableName, ptr->dec_add);
			}
			else/*else print to the opened file that was created before*/
				fprintf(extrnalFile, "%-33s%04d\n", ptr->variableName, ptr->dec_add);
		}
		else
		{/*if variable is entrnal*/
			if (!entryFlag){/*if it's the first time need to create file with the end .ent */
				entryFlag = 1;
				newPath = ChangeEndOfFile(pathfile, ".ent");
				CreateFile(&entryFile, newPath);
				fprintf(entryFile, "%-33s%04d\n", ptr->variableName, ptr->dec_add);
			}
			else/*else print to the opened file that was created before*/
				fprintf(entryFile, "%-33s%04d\n", ptr->variableName, ptr->dec_add);
		}
		ptr = ptr->next;
	}
	/*free the variable that go from ChangeEndOfFile function */
	free(newPath);
	
	if (entryFlag)/*if create enrty file need to close it*/
		fclose(entryFile);
	if (extrnalFile)/*if create extrnal file need to close it*/
		fclose(extrnalFile);

}

/*function that go over instruction list and data list and create a (.ob) 
file  then print the IC and DC values in the first line of output file
first print word and dec_address from all instruction list 
then print  word and dec_address from all data list
arguments:string filename (path of file)
returned value :no returned value */
void printObjFile(char *pathfile){
	FILE *objFile; /*pointer of FILE*/
	instruction *Instptr, *InstTail; /*pointers of instruction*/
	data * Dataptr, *DataTail;/*pointers of data*/
	char *newPath = NULL;
	int i;
	if (insthead == NULL)/*if list is empty*/
		return;

	Instptr = insthead;
	InstTail = insthead->prev;
	/*change end of file and then create new file*/
	newPath = ChangeEndOfFile(pathfile, ".ob");
	CreateFile(&objFile, newPath);
	fprintf(objFile, "   %d %d\n", IC - MEMORY_ADDRESS, DC);/*print the IC and DC */
	/*print all instruction list*/
	/*print all nodes*/
	while (Instptr != InstTail)
	{
		for (i = 0; i < MAX_WORDS_IN_INST && (Instptr->dec_add[i]) != 0; i++){
			fprintf(objFile, "%04d  %05o\n", Instptr->dec_add[i], Instptr->word[i]);
		}
		Instptr = Instptr->next;
	}
	/*print the last node in list*/
	for (i = 0; i < MAX_WORDS_IN_INST && (InstTail->dec_add[i]) != 0; i++){
		fprintf(objFile, "%04d  %05o\n", InstTail->dec_add[i], InstTail->word[i]);

	}

	Dataptr = datahead;
	DataTail = datahead->prev;
	/*print all data list*/
	/*print all nodes*/
	while (Dataptr!=DataTail)
	{
		fprintf(objFile, "%04d  %05o\n", Dataptr->dec_add, Dataptr->word);
		Dataptr = Dataptr->next;
	}
	/*print the last node in list*/
	fprintf(objFile, "%04d  %05o\n", DataTail->dec_add, DataTail->word);
	free(newPath);
	fclose(objFile);
}