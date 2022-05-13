/*
 *********************************************
 *  415 Compilers                            *
 *  Spring 2021                              *
 *  Students                                 *
 *********************************************
 */


#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "Instr.h"
#include "InstrUtils.h"


int main(int argc, char *argv[])
{
        Instruction *InstrList = NULL;
	
	if (argc != 1) {
  	    fprintf(stderr, "Use of command:\n  deadcode  < ILOC file\n");
		exit(-1);
	}

	fprintf(stderr,"------------------------------------------------\n");
	fprintf(stderr,"        Local Deadcode Elimination\n               415 Compilers\n                Spring 2021\n");
	fprintf(stderr,"------------------------------------------------\n");

        InstrList = ReadInstructionList(stdin);
 
        /* HERE IS WHERE YOUR CODE GOES */

        PrintInstructionList(stdout, InstrList);

	fprintf(stderr,"\n-----------------DONE---------------------------\n");
	
	return 0;
}
