#include "alloc.h"


int checkInput(int argc, char** argv);
ILOC* loadFile(FILE* filePointer);
ILOC* getOp(char* currLine);

ILOC* createILOC(char* op, char* param1, char* param2, char* dest, int maxLife, ILOC* next);
ILOC* insertILOC(ILOC* head, ILOC* node);
void printILOCNode(ILOC* node);
void printAllILOC(ILOC* head, char f);
void freeILOC(ILOC* head);

char* getRegString(int reg);
char* toLower(char* string);
char* trimLeft(char* string);
int getReg(char* reg, int offset);
int allocReg(vREG* vRegs, pREG* pRegs, ILOC* pointer, int reg, int k);

ILOC* bottomUpB(ILOC* head, vREG* vREGs, int k, int v);

int checkBReg(pREG* pRegs, int reg, int* stackTop, int k);
int getBReg(vREG* vRegs, pREG* pRegs, ILOC* current, int reg, int k, int* param1Loaded, int* offset, int* previousReg);
int spillBReg(vREG* vRegs, pREG* pRegs, ILOC* current, int reg, int k, int* offset);
int getNextRegUsage(ILOC* current, int reg);
void checkRegs(vREG* vRegs, pREG* pRegs, ILOC* pointer, int param1, int param2, int dest);

int* makePriorityRegs(vREG* vRegs, int v, char flag);
int loadVRegs(ILOC* head, vREG* vRegs, char f);
int fetchReg(vREG* vRegs, pREG* pRegs, int* priorityRegs, int reg, int v, int k, int feasible);
ILOC* setLoadAI(vREG* vRegs, int param, int fReg, int offset);
ILOC* setStoreAI(vREG* vRegs, int dest, int fReg, int offset);

ILOC* topDownS(ILOC* head, vREG* vRegs, int k);

ILOC* topDownT(ILOC* head, vREG* vRegs, int k);

ILOC* topDownO(ILOC* head, vREG* vRegs, int k);

void SFReorder(int* input, vREG* vRegs, int lower, int upper);

int main(int argc, char** argv)
{
    //check for valid user input
    if(checkInput(argc, argv) == -1)
    {
        return 1;
    }

    int k = atoi(argv[1]);
    if(k < 2)
    {
        fprintf(stderr, "\n\033[1;31mError:\t\033[0mIt's impossible to perform register allocation with less than 2 physical registers\n\n");
        return 1;
    }
    char allocFlag = argv[2][0];
    char* inputFile = argv[3];

    FILE* filePointer = fopen(inputFile, "r");
    if(filePointer == NULL)
    {
        fprintf(stderr, "\n\033[1;31mError:\t\033[0mFailed to open the input file\n\n");
        return 1;
    }

    //create an ILOC linked list from the input file
    ILOC* head = loadFile(filePointer);
    vREG vRegs[256];                                //array to store all virtual reg info of source fill
    //head = loadVRegs(head, vRegs, allocFlag);       //store number of virtual regs
    //printf("total virtual regs: %d\n", v);
    
    ILOC* result = NULL;
    //decide which allocation to go with based on flag
    switch(allocFlag)
    {
        case 'b':
            result = bottomUpB(head, vRegs, 0, k);
            break;
        case 's':
            result = topDownS(head, vRegs, k);
            break;
        case 't':
            result = topDownT(head, vRegs, k);
            break;
        case 'o':
            result = topDownO(head, vRegs, k);
            break;
    }
    fclose(filePointer);

    //printAllILOC(head);
    
    //printf("\n\n");
    printAllILOC(result, -1);
    if(result != head)
    {
        freeILOC(head);
    } 
    freeILOC(result); 
    return 0;
}

int checkInput(int argc, char** argv)
{
    if(argc != 4)  
    {
        fprintf(stderr, "\n\033[1;31mError:\t\033[0mPlease enter valid arguments, see readMe.txt for instructions\n\n");
        return -1;
    } 
    
    for(int i = 0; i < strlen(argv[1]); i++)
    {
        if(!isdigit(argv[1][i]))
        {
            fprintf(stderr, "\n\033[1;31mError:\t\033[0m'k' argument must be a non-negative digit\n\n");
            return -1;
        }
    } 
    
    if(atoi(argv[1]) > 256)
    {
        fprintf(stderr, "\n\033[1;31mError:\t\033[0m'k' argument must be no more than 256\n\n");
        return -1;
    }
    if(!isalpha(argv[2][0])|| strlen(argv[2]) > 1)
    {
        fprintf(stderr, "\n\033[1;31mError:\t\033[0m'f' argument must be a single letter\n\n");
        return -1;
    }
    if(argv[2][0] != 'b' && argv[2][0] != 's' && argv[2][0] != 't' && argv[2][0] != 'o')
    {
        fprintf(stderr, "\n\033[1;31mError:\t\033[0m'f' argument must be a 'b, s, t, or o'\n\n");
        return -1;
    }

    return 1;
}

ILOC* loadFile(FILE* filePointer)
{
    ILOC* head = NULL;
    char currLine[256];
    
    while(!feof(filePointer))
    {
        //line by line gets loaded into buffer
        bzero(currLine, 256);
        fgets(currLine, 256, filePointer);
        
        //filter out comments in the input file
        char* comment = strstr(currLine, "//");
        char* comments = strstr(currLine, "/*");
        if(comment != NULL)
        {
            //printf("found a comment, ignoring\n");
        }
        else if(comments != NULL)
        {
            //printf("found multi-lined comments, ignoring\n");
            while(strstr("*/", currLine) == NULL)
            {
                bzero(currLine, 256);
                fgets(currLine, 256, filePointer);
            }
        }
        else if(strlen(currLine) > 1)
        {
            ILOC* instruc = getOp(currLine);
            head = insertILOC(head, instruc);
        }
    }
    return head;
}
ILOC* getOp(char* currLine)
{
    ILOC* instruc = malloc(sizeof(ILOC));
    strcpy(instruc->param1, "\0");
    strcpy(instruc->param2, "\0");
    strcpy(instruc->dest, "\0");
    instruc->maxLife = 0;
    instruc->next = NULL;

    char line[256];
    bzero(line, 256);
    strcpy(line, currLine);
    //printf("Tokenizing: %s", line);

    char leftSide[128];
    bzero(leftSide, 128);
    char* token = strtok(line, "=>");
    if(token != NULL)
    {
        strcpy(leftSide, token);
        //printf("%s\n", leftSide);
    }
    
    //handle case for outputAI instruction
    char* oHandler = toLower(line);
    if(strstr(oHandler, "outputai") != NULL)
    {
        token = strtok(leftSide, " \t");
        strcpy(instruc->op, token);
        token = strtok(NULL, "=>\n");
        /*
        char* ptr = strchr(token, '\n');
        *ptr = '\0';
        *(ptr + 1) = 0;
        */
        strcpy(instruc->param1, token);
        strcpy(instruc->param2, "\0");
        strcpy(instruc->dest, "\0");
        instruc->maxLife = 0;
        instruc->next = NULL;
        //printILOCNode(instruc);
        free(oHandler);
        return instruc;
    }
    free(oHandler);

    char rightSide[48];
    bzero(rightSide, 48);
    token = strtok(NULL, "\t=>\n");
    if(token != NULL)
    {
        strcpy(rightSide, trimLeft(token));
        strcat(rightSide, "\0");
        //printf("%s", rightSide);
    }
    
    //parse left side and fill intermediate array, load into ILOC node
    //printf("Breaking down left side...\n");
    char left[3][16] = {"\0", "\0", "\0"};
    int l = 0;
    token = strtok(leftSide, ", \t\n");
    while(token != NULL)
    {
        strcpy(left[l], token);
        //printf("%s\n", left[l]);
        l++;
        token = strtok(NULL, ", \t\n");
    }  

    /*
    printf("filling intermediate array...\n");
    for(int i = 0; i < 3; i++)
    {
        printf("%s ", left[i]);
    } 
    printf("\n");
    */
    
    //load into ILOC node
    strcpy(instruc->op, left[0]);
    strcpy(instruc->param1, left[1]);
    strcpy(instruc->param2, left[2]);
    strcpy(instruc->dest, rightSide);
    instruc->maxLife = 0;
    instruc->next = NULL;
    return instruc;
}

ILOC* createILOC(char* op, char* param1, char* param2, char* dest, int maxLife, ILOC* next)
{
    ILOC* node = malloc(sizeof(ILOC));
    //load into ILOC node
    strcpy(node->op, op);
    strcpy(node->param1, param1);
    strcpy(node->param2, param2);
    strcpy(node->dest, dest);
    node->maxLife = maxLife;
    node->next = next;
    return node;
}

int loadVRegs(ILOC* head, vREG* vRegs, char f)
{
    for(int i = 0; i < 256; i++)
    {
        vRegs[i].reg = 0;
        vRegs[i].count = 0;
        vRegs[i].rangeStart = -1;
        vRegs[i].rangeEnd = -1;
        vRegs[i].length = 0;
        vRegs[i].spilled = 0;
        vRegs[i].spillIndex = 0;
        vRegs[i].bits = 0;
    }

    int count = 0;
    int offset = 0;
    ILOC* pointer = head;
    
    if(f == 's' || f == 't' || f == 'o')    //fill in frequency values for each virt reg
    {   
        for(int i = 1; i < 256; i++)
        {
            pointer = head;
            while(pointer != NULL)
            {
                int param1 = getReg(pointer->param1, offset);
                int param2 = getReg(pointer->param2, offset);
                int dest = getReg(pointer->dest, offset);
                if(param1 == i || param2 == i || dest == i)
                {
                    if(vRegs[i].count == 0)
                    {
                        count++;
                    }
                    vRegs[i].count++;
                }
                pointer = pointer->next;
            }
        }
    }
    
    if(f == 't')    
    {
        //fill in live range info
        for(int i = 1; i < 256; i++)
        {
            pointer = head;
            int currLine = 0;
            while(pointer != NULL)
            {
                int param1 = getReg(pointer->param1, offset);
                int param2 = getReg(pointer->param2, offset);
                int dest = getReg(pointer->dest, offset);

                if(param1 == i || param2 == i || dest == i)
                {
                    if(vRegs[i].rangeStart < 0)
                    {
                        vRegs[i].rangeStart = currLine++;
                    }
                    vRegs[i].rangeEnd = currLine - 1;
                }
                currLine++;
                pointer = pointer->next;
            }
            vRegs[i].length = vRegs[i].rangeEnd + 1 - vRegs[i].rangeStart;
            //printf("// length of %d is: %d\n", i, vRegs[i].length);
            //printf("// vr%d : [%d, %d]\n", i, vRegs[i].rangeStart, vRegs[i].rangeEnd);
        }

        //fill in max life info
        pointer = head;
        int currLine = 0;
        while(pointer != NULL)
        {
            //printILOCNode(pointer);
            
            int regAlive = 0;
            for(int i = 1; i < 256; i++)
            {
                if(vRegs[i].rangeStart <= currLine && currLine < vRegs[i].rangeEnd)
                {
                    regAlive++;   
                }
            }
            pointer->maxLife = regAlive;
            //printf("// maxlife for line: %d is %d\n", currLine, pointer->maxLife);
            currLine++;
            pointer = pointer->next;
        }
    }
    return count;
}

char* toLower(char* string) 
{
    char* result = (char*)malloc(256);
    strcpy(result, string);
    for(int i = 0; i < strlen(string); i++)
    {
        result[i] = tolower(result[i]);
    }
    return result;
}

char* trimLeft(char* string)
{
    while(isspace(*string))
    {
        string++;
    }
    return string;
}

int getReg(char* reg, int offset)
{
    //return 0 for constants, return register as an int if otherwise
    if(*reg == 'r')
    {
        //return index offset for spilled registers
        if(strchr(reg, ','))
        {
            return offset - 4;
        }
        return atoi(reg + 1);
    }
    return 0;
}

ILOC* insertILOC(ILOC* head, ILOC* node)
{
    if(head == NULL)
    {
        head = node;
        return head;
    }
    ILOC* pointer = head;
    while(pointer->next != NULL)
    {
        pointer = pointer->next;
    }
    pointer->next = node;
    return head;
}
void printILOCNode(ILOC* node)
{
    if(node == NULL)
    {
        return;
    }
    if(node->op[0] != '\0' && node->op[0] != '\n')
    {
        printf("%s ", node->op);
    }
    if(node->param1[0] != '\0')
    {
        if(node->param2[0] != '\0')
        {
            printf("%s, ", node->param1);
            printf("%s ", node->param2);
        }
        else
        {
            printf("%s ", node->param1);
        }
    }
    if(node->dest[0] != '\0')
    {
        printf("=> %s", node->dest);
    }
    //printf("\n//maxLife: %d", node->maxLife);
    printf("\n");
}

void printAllILOC(ILOC* head, char f)
{
    //printf("\n");
    ILOC* pointer = head;
    while(pointer != NULL)
    {
        printILOCNode(pointer);
        if(f == 'm')
        {
            printf("// maxlive: %d\n", pointer->maxLife);
        }
        pointer = pointer->next;
    }
}

void freeILOC(ILOC* head)
{
    ILOC* pointer = head;
    while(pointer != NULL)
    {
        ILOC* current = pointer->next;
        free(pointer);
        pointer = current;
    }
}

char* getRegString(int reg)
{
    int length = snprintf(NULL, 0, "%d", reg);
    char* regString = malloc(length + 2);
    strcpy(regString, "r");
    snprintf(regString + 1, length + 1, "%d", reg);
    //printf("outputing: %s\n", regString);
    return regString;
}

/*
ILOC* bottomUpB(ILOC* head, vREG* vRegs, int v, int k)
{
    pREG* pRegs = malloc(sizeof(pREG) * k);   //array of k registers

    for(int i = 0; i < k; i++)
    {
        pRegs[i].reg = 0;
        pRegs[i].free = 1;
        pRegs[i].next = 0;
        pRegs[i].regStack = 0;
        pRegs[i].stackTop = 0;
    }

    //handle naive case where there is enough physical reg to store all virtual reg
    //in this case, no processing is needed to be done and the result is the same as input
    //however if there are storeAI and loadAI instructions, then they can possibly be 
    //eliminated by using the extra physical regesters instead of spilling

    int stackTop = 0;
    int offset = 0;
    ILOC* result = NULL;
    ILOC* pointer = head;
    while(pointer != NULL)
    {
        //check physical registers first

        //load the current op, check vreg if it is stored in a physical one
        int param1 = getReg(pointer->param1, offset);
        int param2 = getReg(pointer->param2, offset);
        int dest = getReg(pointer->dest, offset);

        ILOC* insert = malloc(sizeof(ILOC));
        ILOC* storeILOC = NULL;
        ILOC* loadILOC1 = NULL;
        ILOC* loadILOC2 = NULL;
        //printf("current op: %s\n", pointer->op);
        //printILOCNode(pointer);
        
        char* opString = pointer->op;
        char* regString1 = "\0";
        char* regString2 = "\0";
        char* destString = "\0";
        int param1Loaded = -1;
        int previousReg = -1;
        if(param1 > 0)  
        {
            int reg = checkBReg(pRegs, param1, &stackTop, k);
            if(reg > 0 && reg <= k)    //virtual reg can be assigned a physical one
            {
                regString1 = getRegString(reg);
                param1Loaded = reg;
                //printf("regString1 is: %s\n", regString1);
                strcpy(insert->param1, regString1);
                free(regString1);
            }
            else    //load a virtual register value from memory
            {
                //printf("need to get a register to load into\n");
               
                //printf("retreiving r%d at offset %d\n", reg, vRegs[reg].spillIndex);
            
                //now locate a register to spill into
                int regToGet = getBReg(vRegs, pRegs, pointer, -reg, k, &param1Loaded, &offset, &previousReg);
                param1Loaded = regToGet;
                destString = getRegString(regToGet);
                strcpy(insert->param1, destString);

                char spillSource[16];
                strcpy(spillSource, "r0, ");
                char* regOffset = getRegString(vRegs[-reg].spillIndex);
                strcat(spillSource, regOffset + 1);
                free(regOffset);

                //destString = getRegString(regToGet);
                if(regToGet < 0)    //handle case for creating a store operation before load
                {
                    free(destString);
                    destString = getRegString(-regToGet);
                    strcpy(insert->param1, destString);
                    char* regSource = getRegString(-regToGet);
                    storeILOC = createILOC("storeAI", regSource, "\0", spillSource, 0, NULL);
                    result = insertILOC(result, storeILOC);
                    param1Loaded = -param1Loaded;
                    free(regSource);
                    regToGet = -regToGet;
                    //free(destString);
                }
                
                //strcpy(insert->param1, destString);

                loadILOC1 = createILOC("loadAI", spillSource, "\0", destString, 0, NULL);
                result = insertILOC(result, loadILOC1);
                free(destString);
            }
            
        }
        else
        {
            strcpy(insert->param1, pointer->param1);
        }
        if(param2 > 0)
        {
            int reg = checkBReg(pRegs, param2, &stackTop, k);
            //printf("virt reg to process for param2 is %d\n", reg);
            if(reg > 0 && reg <= k)
            {
                regString2 = getRegString(reg);
                //printf("regString2 is: %s\n", regString2);
                strcpy(insert->param2, regString2);
                free(regString2);
            }
            else
            {
                //printf("need to get a register to load into\n");
                
                //printf("retreiving r%d at offset %d\n", reg, vRegs[reg].spillIndex);
            
                //now locate a register to spill into
                int regToGet = getBReg(vRegs, pRegs, pointer, -reg, k, &param1Loaded, &offset, &previousReg);

                char spillSource[16];
                strcpy(spillSource, "r0, ");
                char* regOffset = getRegString(vRegs[-reg].spillIndex);
                strcat(spillSource, regOffset + 1);
                free(regOffset);
                
                if(regToGet < 0)    //handle case for creating a store operation before load
                {
                    //free(destString);
                    //destString = getRegString(-regToGet);
                    strcpy(insert->param2, destString);
                    char* regSource = getRegString(-regToGet);
                    storeILOC = createILOC("storeAI", regSource, "\0", spillSource, 0, NULL);
                    result = insertILOC(result, storeILOC);
                    free(regSource);
                    regToGet = -regToGet;
                    //destString = pointer->param2;
                    //strcpy(insert->param2, pointer->param2);
                    //free(destString);
                    //destString = getRegString(param1Loaded);
                }
                destString = getRegString(regToGet);
                strcpy(insert->param2, destString);

                loadILOC2 = createILOC("loadAI", spillSource, "\0", destString, 0, NULL);
                result = insertILOC(result, loadILOC2);
                free(destString);
            }
        }
        else
        {
            strcpy(insert->param2, pointer->param2);
        }
        if(dest > 0)
        {
            int reg = checkBReg(pRegs, dest, &stackTop, k);
            if(reg > 0 && reg <= k) //virtual reg can be assigned a physical one
            {
                destString = getRegString(reg);
                //printf("dest is: %s\n", destString);
                strcpy(insert->dest, destString);
                free(destString);
            }
            else    //spill a register to make room
            {
                int regToSpill = spillBReg(vRegs, pRegs, pointer, -reg, k, &offset);
                char* spillReg;
                if(regToSpill < 0)
                {
                    spillReg = getRegString(-regToSpill);
                }
                else
                {
                    spillReg = getRegString(regToSpill);
                    destString = malloc(24);
                    strcpy(destString, "r0, ");
                    char* regOffset = getRegString(offset);
                    strcat(destString, regOffset + 1);
                    free(regOffset);

                    storeILOC = createILOC("storeAI", spillReg, "\0", destString, 0, NULL);
                    result = insertILOC(result, storeILOC);
                    free(destString);
                }
                
                strcpy(insert->dest, spillReg);
                free(spillReg);

                //create ILOC code for the storeAI spill of 
            
                //printILOCNode(spillILOC);
                //freeILOC(spillILOC);
            }
        }
        else
        {
            strcpy(insert->dest, pointer->dest);
        }
        strcpy(insert->op, opString);
        insert->next = NULL;
        if(storeILOC != NULL)
        {
            //result = insertILOC(result, storeILOC);
        }
        if(loadILOC1 != NULL)
        {
            //result = insertILOC(result, loadILOC1);
        }
        if(loadILOC2 != NULL)
        {
           //result = insertILOC(result, loadILOC2);
        }
        
        result = insertILOC(result, insert);
        
        
        //printILOCNode(result);
        //printAllILOC(result);
        
        for(int i = 0; i < k; i++)
        {
            //printf("pReg%d with r%d\n", i + 1, pRegs[i].reg);
        }
        for(int i = 0; i < 256; i++)
        {
            if(vRegs[i].spillIndex < 0)
            {
                //printf("vReg%d with r%d\n", i, vRegs[i].spillIndex);
            }
            
        }
        //printf("\n");
        
        pointer = pointer->next;
    }

    free(pRegs);
    return result;
}

*/

void checkRegs(vREG* vRegs, pREG* pRegs, ILOC* pointer, int param1, int param2, int dest)
{
    if(param1 > 0)
    {
        int reg = vRegs[param1].reg - 1;
        if(reg >= 0)
        {
            //reg--;
            pRegs[reg].next = getNextRegUsage(pointer, param1);
            if(pRegs[reg].next == 0)
            {
                pRegs[reg].free = 1;
                vRegs[param1].reg = 0;
            }
        }
    }
    if(param2 > 0)
    {
        int reg = vRegs[param2].reg - 1;
        if(reg >= 0)
        {
            //reg--;
            pRegs[reg].next = getNextRegUsage(pointer, param2);
            if(pRegs[reg].next == 0)
            {
                pRegs[reg].free = 1;
                vRegs[param2].reg = 0;
            }
        }
    }
    if(dest > 0)
    {
        int reg = vRegs[dest].reg - 1;
        if(reg >= 0)
        {
            //reg--;
            pRegs[reg].next = getNextRegUsage(pointer, dest);
            if(pRegs[reg].next == 0)
            {
                pRegs[reg].free = 1;
                vRegs[dest].reg = 0;
            }
            
        }
    }
}

ILOC* bottomUpB(ILOC* head, vREG* vRegs, int x, int k)
{
    //make a pass over ILOC code and count the frequency for each virtual register
    int v = loadVRegs(head, vRegs, 'b');
    //printf("// %d total virt regs\n", v);

    pREG* pRegs = malloc(sizeof(pREG) * k);
   
    int offset = -4;
    for(int i = 0; i < k; i++)
    {
        pRegs[i].reg = 0;
        pRegs[i].free = 1;
        pRegs[i].next = -1;    
        pRegs[i].regStack = 0;  
        pRegs[i].stackTop = 0; 
    }

    //main loop, perform allocation
    offset = 0;
    ILOC* result = NULL;
    ILOC* pointer = head;
    while(pointer != NULL)
    {
        int param1 = getReg(pointer->param1, offset);
        int param2 = getReg(pointer->param2, offset);
        int dest = getReg(pointer->dest, offset);

        ILOC* insert = malloc(sizeof(ILOC));
        ILOC* storeILOC = NULL;
        ILOC* loadILOC1 = NULL;
        ILOC* loadILOC2 = NULL;
        ILOC* loadILOC3 = NULL;
        //printf("//    current op: %s\n", pointer->op);
        //printILOCNode(pointer);
        
        char* opString = pointer->op;
        char* regString1 = "\0";
        char* regString2 = "\0";
        char* destString = "\0";

        if(param1 > 0)
        {
            int reg = fetchReg(vRegs, pRegs, NULL, param1, v, k, 0);
            
            //printILOCNode(pointer);
            //printf("// param1 %d reg to consider\n", reg);
            if(reg > 0)
            {
                regString1 = getRegString(reg);
                strcpy(insert->param1, regString1);
                free(regString1);
            }
            else    //locate a physical reg for this virt reg
            {
                int pReg = allocReg(vRegs, pRegs, pointer, param1, k);
                if(pReg > 0)    //phsyical reg found
                {
                    regString1 = getRegString(pReg);
                }
                else    //need to spill the physical reg first
                {   
                    offset = offset - 4;
                    ILOC* regStore = setStoreAI(vRegs, -pReg, -pReg, offset);
                    result = insertILOC(result, regStore);
                    regString1 = getRegString(-pReg);
                    pReg = -pReg;
                    //pRegs[pReg - 1].reg = param1;
                    //vRegs[param1].reg = pReg;
                } 
                if(vRegs[param1].spilled == 1)
                {
                    int index = vRegs[param1].spillIndex;
                    loadILOC1 = setLoadAI(vRegs, param1, pReg, index);
                }
                strcpy(insert->param1, regString1);
                free(regString1);
            }
        }
        else
        {
            strcpy(insert->param1, pointer->param1);
        }
        if(param2 > 0)
        {
            int reg = fetchReg(vRegs, pRegs, NULL, param2, v, k, 0);
            //printf("// param2 %d reg to consider\n", reg);

            if(reg > 0)
            {
                regString2 = getRegString(reg);
                strcpy(insert->param2, regString2);
                free(regString2);
            }
            else    //locate a physical reg for this virt reg
            {
                int pReg = allocReg(vRegs, pRegs, pointer, param2, k);
                if(pReg > 0)    //phsyical reg found
                {
                    regString2 = getRegString(pReg);
                }
                else    //need to spill the physical reg first
                {   
                    offset = offset - 4;
                    ILOC* regStore = setStoreAI(vRegs, -pReg, -pReg, offset);
                    result = insertILOC(result, regStore);
                    regString2 = getRegString(-pReg);
                    pReg = -pReg;
                    //pRegs[pReg - 1].reg = param2;
                    //vRegs[param2].reg = pReg;
                } 
                if(vRegs[param2].spilled == 1)
                {
                    int index = vRegs[param2].spillIndex;
                    loadILOC2 = setLoadAI(vRegs, param2, pReg, index);
                }
                strcpy(insert->param2, regString2);
                free(regString2);
            }
        }
        else
        {
            strcpy(insert->param2, pointer->param2);
        }

        checkRegs(vRegs, pRegs, pointer, param1, param2, -1);

        if(dest > 0)
        {
            
            int reg = fetchReg(vRegs, pRegs, NULL, dest, v, k, 0);
            //printf("// dest %d reg to consider\n", reg);
            if(reg > 0)
            {
                destString = getRegString(reg);
                strcpy(insert->dest, destString);
                free(destString);
            }
            else
            {
                //handle unique case for store operation, where we must read from dest
                int pReg = allocReg(vRegs, pRegs, pointer, dest, k);
                //printf("// pReg for this dest is: %d\n", pReg);
                if(strstr(pointer->op, "store") != NULL)
                {
                    if(pReg > 0)    //phsyical reg found
                    {
                        destString = getRegString(pReg);
                    }
                    else if(vRegs)    //need to spill the physical reg first
                    {   
                        //int x = pRegs[pReg - 1].reg;
                        offset = offset - 4;
                        ILOC* regStore = setStoreAI(vRegs, -pReg, -pReg, offset);
                        result = insertILOC(result, regStore);
                        destString = getRegString(-pReg);
                        pReg = -pReg;
                    } 
                    if(vRegs[dest].spilled == 1)
                    {
                        int index = vRegs[dest].spillIndex;
                        loadILOC3 = setLoadAI(vRegs, dest, pReg, index);
                    }
                    strcpy(insert->dest, destString);
                    free(destString);
                }
                else
                {
                    if(pReg < 0)
                    {
                        pReg = -pReg;
                    }
                    int x = pRegs[pReg - 1].reg;
                    //printf("//virt reg was r%d\n", x);
                    if(vRegs[x].spilled == 1)
                    {
                        //printf("//..was spilled at %d\n", vRegs[x].spillIndex);
                        destString = getRegString(pReg);
                    }
                    else
                    {
                        offset = offset - 4;
                        ILOC* doStoreILOC = setStoreAI(vRegs, pReg, pReg, offset);
                        result = insertILOC(result, doStoreILOC);
                        destString = getRegString(pReg);
                    }
                    //dest must be a feasible register to spill into memory, spill f2 always
                   
                    strcpy(insert->dest, destString);
                    free(destString);
                }
                
            }
        }
        else
        {
            strcpy(insert->dest, pointer->dest);
        }

        checkRegs(vRegs, pRegs, pointer, -1, -1, dest);
        
        strcpy(insert->op, opString);
        insert->maxLife = 0;
        insert->next = NULL;

        if(loadILOC1 != NULL)
        {
            result = insertILOC(result, loadILOC1);
        }
        if(loadILOC2 != NULL)
        {
            result = insertILOC(result, loadILOC2);
        }
        if(loadILOC3 != NULL)
        {
            result = insertILOC(result, loadILOC3);
        }

        result = insertILOC(result, insert);

        if(storeILOC != NULL)
        {
            //printf("printing out store function\n");
            result = insertILOC(result, storeILOC);
        }

        pointer = pointer->next;
    }

    free(pRegs);
    return result;
}

int checkBReg(pREG* pRegs, int reg, int* stackTop, int k)
{
    //printf("stackTop is: %d\n", *stackTop);
    //int result = 1;
    //check if virtual reg exists in any of the physical regs, if it does load it
    for(int i = 0; i < k; i++)
    {
        if(pRegs[i].reg == reg)
        {
            //load instruction and return from function
            //printf("virt reg found, returning reg: %d\n", i + 1);
            return i + 1;
            
        }
    }
    //check if the top of the stack is a free physical reg
    
    if(*stackTop < k && pRegs[*stackTop].free == 1)
    {
        //printf("r%d is not in physical regs \n", reg);
        pRegs[*stackTop].reg = reg;
        pRegs[*stackTop].free = 0;
        (*stackTop)++;
        //printf("stackTop is now: %d\n", *stackTop);
        return *stackTop;
    }
    //printf("returning reg: %d\n", reg);
    
    //return case for when no physical regs containing the virtual reg, or no physical reg is free
    return -reg;
}
int getBReg(vREG* vRegs, pREG* pRegs, ILOC* current, int reg, int k, int* param1Loaded, int* offset, int* previousReg)
{
    //printf("last used physical reg: %d\n", *param1Loaded);
    int regToGet = k - 1;
    int farthest = 0;
    for(int i = k - 1; i >= 0; i--)
    {
        pRegs[i].next = getNextRegUsage(current, pRegs[i].reg);
        //printf("phys r%d holds r%d with next of: %d\n", i + 1, pRegs[i].reg, pRegs[i].next);
        //obtain the reg that won't be used soon, -1 represents one that is never used again
        if(pRegs[i].next == 0 && i != *param1Loaded - 1)
        {
            //printf("wtf with %d != %d\n", i, *param1Loaded);
            farthest = -1;
            regToGet = i; 
        }
        else if(pRegs[i].next >= farthest && i != *param1Loaded - 1)
        {
            if(farthest != -1)
            {
                farthest = pRegs[i].next;
                regToGet = i; 
            }
        }
        //printf("%d\n", regToGet);
    }
    
    *previousReg = reg;
    if(vRegs[pRegs[regToGet].reg].spillIndex >= 0)
    {
        *offset = *offset - 4;
        vRegs[pRegs[regToGet].reg].spillIndex = *offset;

        pRegs[regToGet].reg = reg;
        pRegs[regToGet].next = 0;
        pRegs[regToGet].free = 0;
        return -(regToGet + 1);
    }

    pRegs[regToGet].reg = reg;
    pRegs[regToGet].next = 0;
    pRegs[regToGet].free = 0;

    //*offset = *offset - 4;
    //vRegs[reg].spillIndex = *offset;
    //printf("choosing %d to load into\n", regToGet + 1);
    return regToGet + 1;        //return physical register to spill
}

int spillBReg(vREG* vRegs, pREG* pRegs, ILOC* current, int reg, int k, int* offset)
{
    //look through all virtual registers that are stored in physical regs, 
    //check which one is going to be used furthest in the future
    //and spill that one
    int regToSpill = k - 1;
    int farthest = 0;
    for(int i = k - 1; i >= 0; i--)
    {
        pRegs[i].next = getNextRegUsage(current, pRegs[i].reg);
        //printf("r%d next is: %d lines down\n", i + 1, pRegs[i].next);
        //obtain the reg that won't be used soon, -1 represents one that is never used again
        if(pRegs[i].next == 0)
        {
            //pRegs[i].free = 1;
            farthest = -1;
            regToSpill = i; 
        }
        else if(pRegs[i].next > farthest)
        {
            if(farthest != -1)
            {
                farthest = pRegs[i].next;
                regToSpill = i; 
            }
        }
    }
    /*
    if(farthest == -1)
    {
        pRegs[regToSpill].reg = reg;
        pRegs[regToSpill].next = 0;
        return -(regToSpill + 1);
    }
    */
    
    /*
    if(vRegs[pRegs[regToSpill].reg].spillIndex < 0)
    {
        pRegs[regToSpill].reg = reg;
        pRegs[regToSpill].next = 0;
        return -(regToSpill + 1);
    }
    */

    //printf("\nfarthest is: r%d with %d away\n\n", regToSpill + 1, farthest);
    *offset = *offset - 4;
    vRegs[pRegs[regToSpill].reg].spillIndex = *offset;
    //printf("virtual r%d now at offset %d\n", pRegs[regToSpill].reg, *offset);
    pRegs[regToSpill].reg = reg;
    pRegs[regToSpill].next = 0;
    return regToSpill + 1;
}

int getNextRegUsage(ILOC* current, int reg)
{
    if(reg == 0)
    {
        return -1;
    }
    int i = 1;
    char* regString = getRegString(reg);
    //printf("%s\n", regString);
    ILOC* pointer = current->next;
    //printILOCNode(pointer);
    while(pointer != NULL)
    {
        if(strcmp(pointer->param1, regString) == 0 || strcmp(pointer->param2, regString) == 0)
        {
            free(regString);
            return i;
        }
        i++;
        pointer = pointer->next;
    }
    free(regString);
    return 0;
}

int* makePriorityRegs(vREG* vRegs, int v, char flag)
{
    //copy virt regs into a temp int array
    int* priorityRegs = malloc(sizeof(int) * v);
    int count = 0;
    for(int i = 1; i < 256; i++)
    {
        if(vRegs[i].count > 0 && count < v)
        {
            priorityRegs[count] = i;
            count++;
        }
    }

    //use selection sort to sort the temp array
    for(int i = 0; i < v; i++)
    { 
        int highest = i;
        for(int j = i + 1; j < v; j++)
        {
            if(vRegs[priorityRegs[j]].count > vRegs[priorityRegs[highest]].count)
            {
                highest = j;
            }
            else if(flag == 't' && vRegs[priorityRegs[j]].count == vRegs[priorityRegs[highest]].count)
            {
                if(vRegs[priorityRegs[j]].length > vRegs[priorityRegs[highest]].length)
                {
                    highest = j;
                }
            }
        }
        int temp = priorityRegs[i];
        priorityRegs[i] = priorityRegs[highest];
        priorityRegs[highest] = temp;
    }
    return priorityRegs;
}

int fetchReg(vREG* vRegs, pREG* pRegs, int* priorityRegs, int reg, int v, int k, int feasible)
{
    //this is for bottom-up
    if(priorityRegs == NULL)
    {   
        for(int i = 0; i < k; i++)
        {
            if(pRegs[i].reg == reg)
            {
                return i + 1;
            }
        }
        // let caller know that the reg needs to be loaded from memory
        // or that to replace the virt reg of a physical one
        return 0;
    }
    //this is for top-down
    for(int i = 0; i < v; i++)
    {
        //return the physical reg that holds this virt reg
        if(i < k - feasible && pRegs[i].reg == reg)
        {
            return i + 1;
        }
        else if(priorityRegs[i] == reg && vRegs[reg].spillIndex < 0) //get the offset that holds this virt reg
        {
            return vRegs[priorityRegs[i]].spillIndex;
        }
    }
    //printf("// failed to get phsyical nor an offset for: %d\n", reg);
    return vRegs[reg].reg;
}

int allocReg(vREG* vRegs, pREG* pRegs, ILOC* pointer, int reg, int k)
{
    //find an available physical reg to use
    int greatest = 0;
    for(int i = 0; i < k; i++)
    {
        if(pRegs[i].free == 1)
        {
            pRegs[i].free = 0;
            pRegs[i].reg = reg;
            vRegs[reg].reg = i + 1;
            return i + 1;
        }
        pRegs[i].next = getNextRegUsage(pointer, pRegs[i].reg);
        if(pRegs[i].next > pRegs[greatest].next)
        {
            greatest = i;
        }
    }   
     //check if that reg has a spilled value
    if(vRegs[pRegs[greatest].reg].spilled == 1)
    {
        vRegs[pRegs[greatest].reg].reg = 0;
        pRegs[greatest].reg = reg;
        vRegs[reg].reg = greatest + 1;
        return greatest + 1;
    }  
    /*
    vRegs[pRegs[greatest].reg].reg = 0;
    pRegs[greatest].reg = reg;
    vRegs[reg].reg = greatest + 1;
    */
    return -(greatest + 1);
}

ILOC* setLoadAI(vREG* vRegs, int param, int fReg, int offset)
{
    ILOC* loadILOC = NULL;
    if(vRegs[param].spilled == 1)
    {
        char spillSource[16];
        strcpy(spillSource, "r0, ");
        char* regOffset = getRegString(offset);
        strcat(spillSource, regOffset + 1);

        char* spillReg = getRegString(fReg);

        loadILOC = createILOC("loadAI", spillSource, "\0", spillReg, 0, NULL);
        free(regOffset);
        free(spillReg);
    }
    else if(fReg < 0)
    {
         //printf("// failed to load virt reg %d b/c it is not in memory!\n", param);
         //if not located in memory, need to spill current physical reg
    }
    return loadILOC;
}

ILOC* setStoreAI(vREG* vRegs, int dest, int fReg, int offset)
{
    ILOC* storeILOC = NULL;
    if(vRegs[dest].spilled == 0)
    {
        char* spillReg = getRegString(fReg);
        char spillDest[16];
        strcpy(spillDest, "r0, ");
        char* regOffset = getRegString(offset);
        strcat(spillDest, regOffset + 1);

        vRegs[dest].spilled = 1;
        vRegs[dest].spillIndex = offset;
        vRegs[dest].reg = 0;

        storeILOC = createILOC("storeAI", spillReg, "\0", spillDest, 0, NULL);
        free(spillReg);
        free(regOffset);
    }
    return storeILOC;
}

ILOC* topDownS(ILOC* head, vREG* vRegs, int k)
{

    //make a pass over ILOC code and count the frequency for each virtual register
    int v = loadVRegs(head, vRegs, 's');
    //printf("// %d total virt regs\n", v);

    //create priority array based on least used virt registers
    
    int* priorityRegs = makePriorityRegs(vRegs, v, 's');
    
    //allocate the first k-f physical registers with the priority
    pREG* pRegs = malloc(sizeof(pREG) * k);
    int feasible = 2;   //k and k-1 are feasible regs
    int offset = -4;
    for(int i = 0; i < v; i++)
    {
        if(i < k - feasible)
        {
            pRegs[i].reg = priorityRegs[i];
            pRegs[i].free = 0;
            pRegs[i].next = 0;
            pRegs[i].regStack = 0;
            pRegs[i].stackTop = 0;
            //printf("// r%d gets virt r%d\n", i + 1, priorityRegs[i]);
        }
        else
        {
            if(i < k)
            {
                pRegs[i].reg = 0;
            }
            vRegs[priorityRegs[i]].spillIndex = offset;
            offset = offset - 4;
            //printf("// virt r%d spills to %d\n", priorityRegs[i], vRegs[priorityRegs[i]].spillIndex);
        }
    }

    //main loop, perform allocation
    offset = 0;
    ILOC* result = NULL;
    ILOC* pointer = head;
    while(pointer != NULL)
    {
        int param1 = getReg(pointer->param1, offset);
        int param2 = getReg(pointer->param2, offset);
        int dest = getReg(pointer->dest, offset);

        ILOC* insert = malloc(sizeof(ILOC));
        ILOC* storeILOC = NULL;
        ILOC* loadILOC1 = NULL;
        ILOC* loadILOC2 = NULL;
        ILOC* loadILOC3 = NULL;
        //printf("current op: %s\n", pointer->op);
        //printILOCNode(pointer);
        
        char* opString = pointer->op;
        char* regString1 = "\0";
        char* regString2 = "\0";
        char* destString = "\0";

        if(param1 > 0)
        {
            int reg = fetchReg(vRegs, pRegs, priorityRegs, param1, v, k, feasible);
            //printILOCNode(pointer);
            //printf("// %d reg to consider\n", reg);
            if(reg > 0)
            {
                regString1 = getRegString(reg);
                strcpy(insert->param1, regString1);
                free(regString1);
            }
            else
            {
                loadILOC1 = setLoadAI(vRegs, param1, k - 1, reg);
                pRegs[k - 2].reg = param1;

                regString1 = getRegString(k - 1);
                strcpy(insert->param1, regString1);
                free(regString1);
            }
        }
        else
        {
            strcpy(insert->param1, pointer->param1);
        }
        if(param2 > 0)
        {
            int reg = fetchReg(vRegs, pRegs, priorityRegs, param2, v, k, feasible);
            if(reg > 0)
            {
                regString2 = getRegString(reg);
                strcpy(insert->param2, regString2);
                free(regString2);
            }
            else
            {
                //call loadAI
                loadILOC2 = setLoadAI(vRegs, param2, k, reg);
                pRegs[k - 1].reg = param2;
                //fill in param2 with feasible reg
                regString2 = getRegString(k);
                strcpy(insert->param2, regString2);
                free(regString2);
            }
        }
        else
        {
            strcpy(insert->param2, pointer->param2);
        }
        if(dest > 0)
        {
            int reg = fetchReg(vRegs, pRegs, priorityRegs, dest, v, k, feasible);
            if(reg > 0)
            {
                destString = getRegString(reg);
                strcpy(insert->dest, destString);
                free(destString);
            }
            else
            {
                //handle unique case for store operation, where we must read from dest
                if(strstr(pointer->op, "store") != NULL)
                {
                    loadILOC3 = setLoadAI(vRegs, dest, k, reg);
                    pRegs[k - 1].reg = dest;

                    destString = getRegString(k);
                    strcpy(insert->dest, destString);
                    free(destString);
                }
                else
                {
                    //dest must be a feasible register to spill into memory, spill f2 always
                    destString = getRegString(k);
                    strcpy(insert->dest, destString);
                    free(destString);

                    //peform the spill, where reg = offset
                    storeILOC = setStoreAI(vRegs, dest, k, reg);
                    pRegs[k - 1].reg = dest;
                }
            }
        }
        else
        {
            strcpy(insert->dest, pointer->dest);
        }

        strcpy(insert->op, opString);
        insert->maxLife = 0;
        insert->next = NULL;

        if(loadILOC1 != NULL)
        {
            result = insertILOC(result, loadILOC1);
        }
        if(loadILOC2 != NULL)
        {
            result = insertILOC(result, loadILOC2);
        }
        if(loadILOC3 != NULL)
        {
            result = insertILOC(result, loadILOC3);
        }

        result = insertILOC(result, insert);

        if(storeILOC != NULL)
        {
            result = insertILOC(result, storeILOC);
        }

        pointer = pointer->next;
    }

    free(priorityRegs);
    free(pRegs);
    return result;
}

ILOC* topDownT(ILOC* head, vREG* vRegs, int k)
{

    //make a pass over ILOC code and count the frequency for each virtual register
    int v = loadVRegs(head, vRegs, 't');
    //printf("// %d total virt regs\n", v);
    //printAllILOC(head, 'm');
    /*
    if(v <= k)
    {
        //there are enough physical registers for each virtual registers
        //no special allocation needed to be done, return the input ILOC code
        return head;
    }
    */

    //create priority array based on least used virt registers
    
    int* priorityRegs = makePriorityRegs(vRegs, v, 't');

    //allocate the first k-f physical registers with the priority
    pREG* pRegs = malloc(sizeof(pREG) * k);
    int feasible = 2;   //k and k-1 are feasible regs
    int offset = -4;
    

    for(int i = 0; i < k; i++)
    {
        if(i < k - 2 && i < v)
        {
            pRegs[i].reg = priorityRegs[i];
            vRegs[priorityRegs[i]].reg = i + 1;
            //save the mapping of virt reg to physical one
        }
        else
        {
            pRegs[i].reg = 0;
        }
    }

    int thisLine = 1;
    //offset = -4;
    ILOC* pointer = head;
    while(pointer != NULL)
    {
        int dest = getReg(pointer->dest, 0);
        if(pointer->maxLife > k - 2)
        {                
            if(dest > 0 && vRegs[dest].reg == 0 && vRegs[dest].spillIndex == 0)
            {
                vRegs[dest].reg = -1;
                vRegs[dest].spillIndex = offset;
                offset = offset - 4;

                int j = thisLine;
                ILOC* pointer2 = pointer;
                while(pointer2 != NULL)
                {
                    if(j < vRegs[dest].rangeEnd)
                    {
                        pointer2->maxLife--;
                    }
                    j++;
                    pointer2 = pointer2->next;
                }
            }
        }
        thisLine++;
        pointer = pointer->next;
    }

    //fill in on all virt regs with no assignment and see if we get it a physical one

    for(int i = 0; i < v; i++)
    {
        if(vRegs[priorityRegs[i]].reg == 0)
        {
            int replace = -1;
            for(int j = 0; j < v; j++)
            {
                if(vRegs[priorityRegs[i]].rangeStart > vRegs[priorityRegs[j]].rangeEnd)
                {
                    //get latest reg that is done with its live range
                    replace = vRegs[priorityRegs[j]].reg;

                    for(int l = k - 2; l < v; l++)
                    {
                        if(vRegs[priorityRegs[l]].reg == replace)
                        {
                            if(vRegs[priorityRegs[i]].rangeStart <= vRegs[priorityRegs[l]].rangeEnd )
                            {
                                replace = -1;
                            }
                        }
                    }
                }
            }
            if(replace != -1)
            {
                vRegs[priorityRegs[i]].reg = replace;
            }
            //assign to spill code if it cannot get a physical reg
            else if(vRegs[priorityRegs[i]].reg == 0)
            {
                vRegs[priorityRegs[i]].reg = -1;
                vRegs[priorityRegs[i]].spillIndex = offset;
                offset = offset - 4;
            }
        }
    }
    
    for(int i = 0; i < v; i++)
    {
        //printf("// virt r%d spill is: %d, with pReg%d, [%d|%d]\n", priorityRegs[i], vRegs[priorityRegs[i]].spillIndex, vRegs[priorityRegs[i]].reg, vRegs[priorityRegs[i]].rangeStart, vRegs[priorityRegs[i]].rangeEnd);
    }
    /*
    int line = 0;
    pointer = head;
    while(pointer != NULL)
    {
        printILOCNode(pointer);
        printf("// \tmax life is now: %d - ", pointer->maxLife);
        for(int i = 1; i < 256; i++)
        {
            if(vRegs[i].rangeStart <= line && line < vRegs[i].rangeEnd && vRegs[i].reg > 0)
            {
                printf("r%d ", i);
            }
        }
        printf("\n");
        pointer = pointer->next;
        line++;
    }
    */

    //main loop, perform allocation
    int currLine = 1;
    offset = 0;
    ILOC* result = NULL;
    pointer = head;
    while(pointer != NULL)
    {
        int param1 = getReg(pointer->param1, offset);
        int param2 = getReg(pointer->param2, offset);
        int dest = getReg(pointer->dest, offset);

        ILOC* insert = malloc(sizeof(ILOC));
        ILOC* storeILOC = NULL;
        ILOC* loadILOC1 = NULL;
        ILOC* loadILOC2 = NULL;
        ILOC* loadILOC3 = NULL;
        //printf("current op: %s\n", pointer->op);
        //printILOCNode(pointer);
        
        char* opString = pointer->op;
        char* regString1 = "\0";
        char* regString2 = "\0";
        char* destString = "\0";

        if(param1 > 0)
        {
            int reg = fetchReg(vRegs, pRegs, priorityRegs, param1, v, k, feasible);
            //printILOCNode(pointer);
            //printf("// %d reg to consider\n", reg);
            if(reg > 0)
            {
                regString1 = getRegString(reg);
                strcpy(insert->param1, regString1);
                free(regString1);
            }
            else
            {
                loadILOC1 = setLoadAI(vRegs, param1, k - 1, reg);
                pRegs[k - 2].reg = param1;

                regString1 = getRegString(k - 1);
                strcpy(insert->param1, regString1);
                free(regString1);
            }
        }
        else
        {
            strcpy(insert->param1, pointer->param1);
        }
        if(param2 > 0)
        {
            int reg = fetchReg(vRegs, pRegs, priorityRegs, param2, v, k, feasible);
            if(reg > 0)
            {
                regString2 = getRegString(reg);
                strcpy(insert->param2, regString2);
                free(regString2);
            }
            else
            {
                //call loadAI
                loadILOC2 = setLoadAI(vRegs, param2, k, reg);
                pRegs[k - 1].reg = param2;
                //fill in param2 with feasible reg
                regString2 = getRegString(k);
                strcpy(insert->param2, regString2);
                free(regString2);
            }
        }
        else
        {
            strcpy(insert->param2, pointer->param2);
        }
        if(dest > 0)
        {
            int reg = fetchReg(vRegs, pRegs, priorityRegs, dest, v, k, feasible);
            if(reg > 0)
            {
                destString = getRegString(reg);
                strcpy(insert->dest, destString);
                free(destString);
            }
            else if(reg == 0)
            {
                int replace = 0;
                //handle case for when this is a reg we can do simple replacing for b/c maxlife <= k - 2
                for(int i = 0; i < k - 2; i++)
                {
                    //printf("// vReg r%d has range end of %d, vs currLine%d\n", pRegs[i].reg, vRegs[pRegs[i].reg].rangeEnd, currLine);
                    if(currLine < vRegs[pRegs[i].reg].rangeStart || currLine > vRegs[pRegs[i].reg].rangeEnd)
                    {
                        replace = i;
                        //j = pRegs[i];
                    }
                }
                //if(replace)
                //printf("// replacing for r%d: r%d with r%d\n", replace + 1, pRegs[replace].reg, dest);

                destString = getRegString(replace + 1);
                strcpy(insert->dest, destString);
                free(destString);
                
                vRegs[pRegs[replace].reg].reg = replace;
                pRegs[replace].reg = dest;
            }
            else
            {
                //handle unique case for store operation, where we must read from dest
                if(strstr(pointer->op, "store") != NULL)
                {
                    loadILOC3 = setLoadAI(vRegs, dest, k, reg);
                    pRegs[k - 1].reg = dest;

                    destString = getRegString(k);
                    strcpy(insert->dest, destString);
                    free(destString);
                }
                else
                {
                    //dest must be a feasible register to spill into memory, spill f2 always
                    destString = getRegString(k);
                    strcpy(insert->dest, destString);
                    free(destString);

                    //peform the spill, where reg = offset
                    storeILOC = setStoreAI(vRegs, dest, k, reg);
                    pRegs[k - 1].reg = dest;
                }
            }
        }
        else
        {
            strcpy(insert->dest, pointer->dest);
        }

        strcpy(insert->op, opString);
        insert->maxLife = 0;
        insert->next = NULL;

        if(loadILOC1 != NULL)
        {
            result = insertILOC(result, loadILOC1);
        }
        if(loadILOC2 != NULL)
        {
            result = insertILOC(result, loadILOC2);
        }
        if(loadILOC3 != NULL)
        {
            result = insertILOC(result, loadILOC3);
        }

        result = insertILOC(result, insert);

        if(storeILOC != NULL)
        {
            result = insertILOC(result, storeILOC);
        }
        currLine++;
        pointer = pointer->next;
    }

    free(priorityRegs);
    free(pRegs);
    return result;
}

ILOC* topDownO(ILOC* head, vREG* vRegs, int k)
{

    //make a pass over ILOC code and count the frequency for each virtual register
    int v = loadVRegs(head, vRegs, 'o');
    //printf("// %d total virt regs\n", v);

    //create priority array based on least used virt registers
    int* priorityRegs = makePriorityRegs(vRegs, v, 'o');
    for(int i = 0; i < v; i++)
    {
        //printf("// r%d wtih count: %d\n", priorityRegs[i], vRegs[priorityRegs[i]].count);
    }

    //do shannon-fano encoding to sort priority order for the virt regs
    SFReorder(priorityRegs, vRegs, 0, v);

    for(int i = 0; i < v; i++)
    { 
        int lowest = i;
        for(int j = i + 1; j < v; j++)
        {
            if(vRegs[priorityRegs[j]].bits < vRegs[priorityRegs[lowest]].bits)
            {
                lowest = j;
            }
        }
        int temp = priorityRegs[i];
        priorityRegs[i] = priorityRegs[lowest];
        priorityRegs[lowest] = temp;
    }

    for(int i = 0; i < v; i++)
    {
        //printf("// r%d wtih count: %d, bits of %d\n", priorityRegs[i], vRegs[priorityRegs[i]].count, vRegs[priorityRegs[i]].bits);
    }

    
    //allocate the first k-f physical registers with the priority
    pREG* pRegs = malloc(sizeof(pREG) * k);
    int feasible = 2;   //k and k-1 are feasible regs
    int offset = -4;
    for(int i = 0; i < v; i++)
    {
        if(i < k - feasible)
        {
            pRegs[i].reg = priorityRegs[i];
            pRegs[i].free = 0;
            pRegs[i].next = 0;
            pRegs[i].regStack = 0;
            pRegs[i].stackTop = 0;
            //printf("// r%d gets virt r%d\n", i + 1, priorityRegs[i]);
        }
        else
        {
            if(i < k)
            {
                pRegs[i].reg = 0;
            }
            vRegs[priorityRegs[i]].spillIndex = offset;
            offset = offset - 4;
            //printf("// virt r%d spills to %d\n", priorityRegs[i], vRegs[priorityRegs[i]].spillIndex);
        }
    }

    //main loop, perform allocation
    offset = 0;
    ILOC* result = NULL;
    ILOC* pointer = head;
    while(pointer != NULL)
    {
        int param1 = getReg(pointer->param1, offset);
        int param2 = getReg(pointer->param2, offset);
        int dest = getReg(pointer->dest, offset);

        ILOC* insert = malloc(sizeof(ILOC));
        ILOC* storeILOC = NULL;
        ILOC* loadILOC1 = NULL;
        ILOC* loadILOC2 = NULL;
        ILOC* loadILOC3 = NULL;

        char* opString = pointer->op;
        char* regString1 = "\0";
        char* regString2 = "\0";
        char* destString = "\0";

        if(param1 > 0)
        {
            int reg = fetchReg(vRegs, pRegs, priorityRegs, param1, v, k, feasible);
            //printILOCNode(pointer);
            //printf("// %d reg to consider\n", reg);
            if(reg > 0)
            {
                regString1 = getRegString(reg);
                strcpy(insert->param1, regString1);
                free(regString1);
            }
            else
            {
                loadILOC1 = setLoadAI(vRegs, param1, k - 1, reg);
                pRegs[k - 2].reg = param1;

                regString1 = getRegString(k - 1);
                strcpy(insert->param1, regString1);
                free(regString1);
            }
        }
        else
        {
            strcpy(insert->param1, pointer->param1);
        }
        if(param2 > 0)
        {
            int reg = fetchReg(vRegs, pRegs, priorityRegs, param2, v, k, feasible);
            if(reg > 0)
            {
                regString2 = getRegString(reg);
                strcpy(insert->param2, regString2);
                free(regString2);
            }
            else
            {
                //call loadAI
                loadILOC2 = setLoadAI(vRegs, param2, k, reg);
                pRegs[k - 1].reg = param2;
                //fill in param2 with feasible reg
                regString2 = getRegString(k);
                strcpy(insert->param2, regString2);
                free(regString2);
            }
        }
        else
        {
            strcpy(insert->param2, pointer->param2);
        }
        if(dest > 0)
        {
            int reg = fetchReg(vRegs, pRegs, priorityRegs, dest, v, k, feasible);
            if(reg > 0)
            {
                destString = getRegString(reg);
                strcpy(insert->dest, destString);
                free(destString);
            }
            else
            {
                //handle unique case for store operation, where we must read from dest
                if(strstr(pointer->op, "store") != NULL)
                {
                    loadILOC3 = setLoadAI(vRegs, dest, k, reg);
                    pRegs[k - 1].reg = dest;

                    destString = getRegString(k);
                    strcpy(insert->dest, destString);
                    free(destString);
                }
                else
                {
                    //dest must be a feasible register to spill into memory, spill f2 always
                    destString = getRegString(k);
                    strcpy(insert->dest, destString);
                    free(destString);

                    //peform the spill, where reg = offset
                    storeILOC = setStoreAI(vRegs, dest, k, reg);
                    pRegs[k - 1].reg = dest;
                }
            }
        }
        else
        {
            strcpy(insert->dest, pointer->dest);
        }

        strcpy(insert->op, opString);
        insert->maxLife = 0;
        insert->next = NULL;

        if(loadILOC1 != NULL)
        {
            result = insertILOC(result, loadILOC1);
        }
        if(loadILOC2 != NULL)
        {
            result = insertILOC(result, loadILOC2);
        }
        if(loadILOC3 != NULL)
        {
            result = insertILOC(result, loadILOC3);
        }

        result = insertILOC(result, insert);

        if(storeILOC != NULL)
        {
            result = insertILOC(result, storeILOC);
        }

        pointer = pointer->next;
    }

    free(priorityRegs);
    free(pRegs);
    return result;
}

void SFReorder(int* input, vREG* vRegs, int lower, int upper)
{
    int size = upper - lower;
    if(size <= 1)
    {
        return;
    }
 
    double mid = (size) / 2;
    int split = (int)ceil(mid) + lower;
    for(int i = lower; i < split; i++)
    {            
        vRegs[input[i]].bits = (vRegs[input[i]].bits) << 1;
    }
        //printf("// split is now %d, [%d %d]\n", split, lower, upper); 
    for(int i = split; i < upper; i++)
    {
        vRegs[input[i]].bits = (vRegs[input[i]].bits << 1) | 1;
    }
    //call on left and right halves
    SFReorder(input, vRegs, lower, split);
    SFReorder(input, vRegs, split, upper);
    
    return;
}