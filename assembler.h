/*Shadi Zidany
Julian Jubran*/
#ifndef ASSEMBLER_H /*returns true if this macro is not defined*/
#define ASSEMBLER_H /*substitutes a preprocessor macro*/

#include "utilities.h"

/*function that get open file that pass as argument and called
the functions first pass and second pass also called the fix functions that fixed the address in list
create and fill the output files
arguments:string filename (path of file)
returned value :no returned value */
void assembler(char *filename);

/*function that free all lists (symbol,data,instruction,variables) also set NULL on it
in addition close the(input file)
arguments:int failure .if it's equal to 1 that mean malloc failed if it's equal to 2 failed to create (output file )
returned value :no returned value */
void terminate(int failure);

/*function that add new node  for symbol list and fill all fields the same argurments that pass
arguments: head - head of symbol list , label - string that save label (in assembler) ,directive if the line is directive or no (1 or 0)
external - if label is external (equal to 1 that mean is external else it's not external )
returned value : -1 if it's failed , 0 if it's pass*/
int AddNewLabelToList(symbol **head, char * Label, unsigned directive, unsigned external);

/* Recursive function that free all list (symbol list)
arguments: head of list (symbol list)
returned value : no returned value*/
void FreeLabelList(symbol *head);

/* function that fix all dec_address in (symbol list)
if the node is directive add the dec_address with IC
arguments: head of list (symbol list)
returned value : no returned value*/
void FixDirectiveNodes(symbol *head);

/*function that add new node  for data list and  (add all array values) all
add new node it's prev of head
arguments: head - head of data list , array and size of array
returned value : 1 if pass else call terminate function*/
int AddNewDataToList(data **head, int *array, int size);

/* function that free all list (data list)
arguments: head of list (data list)
returned value : no returned value*/
void FreeDataList(data *head);

/*function that add new node  for data list and  (add all string values using the strlen)
all add new node it's prev of head
arguments: head - head of data list , string
returned value : 1 if pass else call terminate function*/
int AddNewStringToList(data **head, char * string);

/* function that fix all dec_address in (data list)
add the dec_address with IC
arguments: head of list (data list)
returned value : no returned value*/
void FixDecAddressNodes(data *head);

/* function that set 0 in all array elements
arguments: pointer for array and size of array
returned value : no returned value*/
void ResetArray(unsigned Array[], int size);

/*function that add new node  for instruction list in word is empty in dec_address there
IC  in addition increment IC
add new node it's prev of head
arguments: head - head of instruction list amount of word  for example 1 or 2 or 3 words
returned value : 1 if pass else call terminate function*/
int AddNewWordToList(instruction **head, int Amount);

/* function that free all list (instruction list)
arguments: head of list (instruction list)
returned value : no returned value*/
void FreeInstructionList(instruction *head);

/* function that update node that get from argument (array) it's update the word variable in node
arguments: node of list (instruction list) , array that include the value and the size of array
returned value : returned the next of the node(that got from argument)*/
instruction * UpdateAllWordsInList(instruction * node, unsigned Array[], int size);

/*function that add new node  for variables list with the pass arguments
used static that save the tail of the list to add all times after this node
arguments: isextrn - if extrnal  equal to 1 else 0 , address and string that is the name of the variable
returned value : 1 if pass else call terminate function*/
int AddNewVariableToList(int isextrn, int address, char * string);

/* Recursive function that free all list (variables list)
arguments: head of list (variables list)
returned value : no returned value*/
void FreeVariableList(variables *head);

/* function that go over on symbol list and check if label (pass in argument) and check about
isentry  if it is entrnal  called the AddNewVariableToList function  and added it to variable list
with label and the dec_address that was in symbol list
if is external called the AddNewVariableToList function and added it to variable list with label
and with the currecnt IC becuase it's called in second pass
arguments: label-string that need to searvh , isentry- if equal to 1 that mean it's entry variable
else it's an external variable
returned value : returned the dec_address if the label else -1 if there any problem */
int CheckOnSymbolListAndFillVarList(char Label[], int isentry);

/* function that change the end of path file it's get as argument the path as string
and also the new end that need to concatenate (end of file like .obj/.ext/.ent),
it's use also malloc and the free function use on function that call it , (printExtAndEntFiles,printObjFile)
arguments: oldStr- string is path of file  newEnd-string is new end need to concatenate (end of file like .obj/.ext/.ent)
returned value : pointer of new string after all changed */
char * ChangeEndOfFile(char *oldStr, char * newEnd);

/*function that get a pointer of FILE and path then it's create a file with W mode (write mode)
if it's failed call terminate function that close the program
arguments: file-pointer of pointer (FILE) and path-string path of new file that need to create
returned value : no returned value*/
void CreateFile(FILE **file, char *path);

/*function that go over variable list (this list have all information about variables entrnal,external)
if met first variable of (entrnal,external) it's changed the path then create the file same type of variable
then it's print all information on the file that created
it's created two files  (.ent and .ext) and fill all information
arguments:string filename (path of file)
returned value :no returned value */
void printExtAndEntFiles(char *pathfile);

/*function that go over instruction list and data list and create a (.ob)
file  then print the IC and DC values in the first line of output file
first print word and dec_address from all instruction list
then print  word and dec_address from all data list
arguments:string filename (path of file)
returned value :no returned value */
void printObjFile(char *pathfile);

#endif /*ends preprocessor conditional*/