#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>


/**************************************************************************************************/
/********************************************Tracing Part******************************************/
// This is the linked list for stack holding function identifier
struct TRACE_NODE_STRUCT {  
    char* functionid;                      
    struct TRACE_NODE_STRUCT* next;
};
typedef struct TRACE_NODE_STRUCT TRACE_NODE;
static TRACE_NODE* TRACE_TOP = NULL; 


// Push p onto the stack
void PUSH_TRACE(char* p) {  
    TRACE_NODE* tnode;  
    static char glob[] = "global";  
    if (TRACE_TOP == NULL) {     
        TRACE_TOP = (TRACE_NODE*) malloc(sizeof(TRACE_NODE));    
        if (TRACE_TOP == NULL) {      
            printf("PUSH_TRACE: memory allocation error\n");      
            exit(1);    
        }    
        TRACE_TOP->functionid = glob;    
        TRACE_TOP->next = NULL; 
    }   
    tnode = (TRACE_NODE*) malloc(sizeof(TRACE_NODE));  
    if (tnode == NULL) {    
        printf("PUSH_TRACE: memory allocation error\n");    
        exit(1);  
    }  
    tnode->functionid = p;  
    tnode->next = TRACE_TOP;   
    TRACE_TOP = tnode;          
}

// Remove the top of the stack
void POP_TRACE() {  
    TRACE_NODE* tnode;  
    tnode = TRACE_TOP;  
    TRACE_TOP = tnode->next;  
    free(tnode);
}

// Print out function calls on the stack
char* PRINT_TRACE() {  
    int depth = 50; //A max of 50 levels in the stack will be combined in a string for printing out.  
    int i, length, j;  
    TRACE_NODE* tnode;  
    static char buf[100];  
    if (TRACE_TOP==NULL) { // stack not initialized yet, so we are    
        strcpy(buf,"global"); // still in the `global' area    
        return buf;  
    }  
    sprintf(buf,"%s",TRACE_TOP->functionid);  
    length = strlen(buf); // length of the string so far  
    for(i=1, tnode=TRACE_TOP->next; tnode!=NULL && i < depth; i++, tnode=tnode->next) {    
        j = strlen(tnode->functionid); // length of what we want to add    
        if (length+j+1 < 100) { // total length is ok      
            sprintf(buf+length,":%s",tnode->functionid);      
            length += j+1;    
        }else // it would be too long      
            break;  
    }  
    return buf;
}

// REALLOC calls realloc
void* REALLOC(void* p, int t, char* file, int line) {
    PUSH_TRACE("REALLOC");
    int outfile = open("memtrace.out", O_CREAT|O_APPEND|O_WRONLY);
    dup2(outfile, 1);
    p = realloc(p,t);
    printf("File %s, line %d, function %s reallocated the memory segment at address %p to a new size %d\n", file, line, TRACE_TOP->functionid, p, t);
    printf("--------------------------------stack information-------------------------------\n%s\n", PRINT_TRACE());
    close(outfile);
    POP_TRACE();
    return p;
}

// MALLOC calls malloc
void* MALLOC(int t, char* file, int line) {
    PUSH_TRACE("MALLOC");
    int outfile = open("memtrace.out", O_CREAT|O_APPEND|O_WRONLY);
    dup2(outfile, 1);
    void* p;
    p = malloc(t);
    printf("File %s, line %d, function %s allocated new memory segment at address %p to size %d\n", file, line, TRACE_TOP->functionid, p, t);
    printf("--------------------------------stack information-------------------------------\n%s\n", PRINT_TRACE());
    close(outfile);
    POP_TRACE();
    return p;
}

// FREE calls free
void FREE(void* p, char* file, int line) {
    PUSH_TRACE("FREE");
    int outfile = open("memtrace.out", O_CREAT|O_APPEND|O_WRONLY);
    dup2(outfile, 1);
    printf("File %s, line %d, function %s deallocated the memory segment at address %p\n", file, line, TRACE_TOP->functionid, p);
    close(outfile);
    printf("--------------------------------stack information-------------------------------\n%s\n", PRINT_TRACE());
    free(p);
    POP_TRACE();
}

#define realloc(a,b) REALLOC(a,b,__FILE__,__LINE__)
#define malloc(a) MALLOC(a,__FILE__,__LINE__)
#define free(a) FREE(a,__FILE__,__LINE__)

/**************************************************************************************************/
/*******************************Linked List for Storing********************************************/
// This is linked list to store commands
struct CommandNode {
    char* command;
    int index;
    struct CommandNode *next;
};

// Create a new node
void CreateCommandNode(struct CommandNode *thisNode, char cmd[25], int ind, struct CommandNode *nextCmd) {
    PUSH_TRACE("CreateCommandNode");
    thisNode->command = (char*)malloc(sizeof(char) * 25);
    strcpy(thisNode->command, cmd);     
    thisNode->index = ind;   
    thisNode->next = nextCmd;
    POP_TRACE();
    return;
}

// Print all commands recursively out to a file
void PrintNodes(struct CommandNode *start) {
    if(start == NULL) {
        return;
    }
    int outfile = open("memtrace.out", O_CREAT|O_APPEND|O_WRONLY);
    dup2(outfile, 1);
    printf("------------------------commands store in linked list-------------------------\n");
    printf("%d: %s\n", start->index, start->command);
    PrintNodes(start->next);
    close(outfile);
}

/**************************************************************************************************/
/**************************************************************************************************/
int main(int argc, char *argv[]) {
    PUSH_TRACE("main");
    
    if ( argc != 2 ){
        printf( "Input file error!\n");
    }
    
    FILE * fp;
    fp = fopen(argv[1], "r" );
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // Initialize a char** type to store commands
    int rows = 5; //lines count
    int columns = 25;
    char **commandChar;
    commandChar = (char**) malloc(sizeof(char*) * rows);
    for(int i = 0; i < rows; i++) {
        commandChar[i] = (char*) malloc(sizeof(char) * columns);
    }

    // Read and store lines of commands in char**
    int rowCount = 0; //lines count
    char command[20];
    while (fgets(command, sizeof(command), fp)) {
        if (rowCount >= rows) {
            commandChar = (char**) realloc(commandChar, sizeof(char*) * (rows + 1));
            commandChar[rows] = (char*) malloc(sizeof(char) * columns);
            rows += 1;
        }
        strcpy(commandChar[rowCount], command);
        rowCount++;
        memset(command, 0, 20);
    }
    fclose(fp);
    
    if(rowCount == 0) {
        printf("The file is empty!\n");
        exit(0);
    }

    // Store lines of commands in linked list
    struct CommandNode *headCommand = (struct CommandNode*)malloc(sizeof(struct CommandNode));
    CreateCommandNode(headCommand, commandChar[0], 1, NULL);
    struct CommandNode *curr = headCommand;
    for(int i = 1; i < rowCount; i++) {
        struct CommandNode *nextCommand = (struct CommandNode*)malloc(sizeof(struct CommandNode));
        CreateCommandNode(nextCommand, commandChar[i], i+1, NULL);
        curr->next = nextCommand;
        curr = nextCommand;
    }
    
    // Deallocate the char**
    for(int i = 0; i < rowCount; i++) {
        free((void*)commandChar[i]);
    }
    free((void*)commandChar);
    
    // Print the commands in the linked list
    PrintNodes(headCommand);

    // Deallocate the linked list
    struct CommandNode *ptr = headCommand;
    while(headCommand != NULL) {
        if(ptr==NULL) {                
            printf("List is Empty:n");
        }else {
            headCommand = headCommand->next ;
            free(ptr);
            ptr = headCommand;
        }
    }
    

    
    
    POP_TRACE();
    return(0);

}

