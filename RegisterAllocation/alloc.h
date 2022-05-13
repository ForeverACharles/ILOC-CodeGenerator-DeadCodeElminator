#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

typedef struct ILOC     //linked list node to store an ILOC instruction
{
    char op[16];
    char param1[16];
    char param2[16];
    char dest[48];
    int maxLife;
    struct ILOC* next;
} ILOC;

typedef struct vREG     //struct to represent virtual registers
{
    int reg;           //pReg that is mapped to it
    int count;         //frequency
    int rangeStart;    //live range info
    int rangeEnd;
    int length;
    int spilled;
    int spillIndex;    //spill offset relative to r0
    int bits;
} vREG;

typedef struct pREG
{
    int reg;       //contains which virtual reg is stored in physical ones
    int free;      //status of phsyical reg, 0 or 1
    int next;      //index of next use of a virtual register
    int regStack;  //
    int stackTop;   
} pREG;