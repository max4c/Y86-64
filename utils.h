#ifndef UTILS
#define UTILS

/** BEGIN Exit codes **/
#define CALLOC_ERROR 2
#define BAD_ADDRESS 3
#define BAD_REGISTER 4
#define BAD_FILENAME 5
#define BAD_IFUN 6
/** END Exit codes **/

FILE *parseCommandLine(int argc, char **argv, int *stepMode);

/** BEGIN Machine State Definition **/
typedef unsigned char byteType;
typedef long int wordType;

extern byteType *memory;
extern int memorySizeInBytes;

extern const int REGISTER_COUNT;
extern wordType *registers;

typedef enum {FALSE = 0,TRUE} bool;
extern bool signFlag;
extern bool zeroFlag ;
extern bool overflowFlag;

typedef enum {STAT_AOK, STAT_HLT} status;
extern status stat;
extern wordType pc;

extern int cycleCounter;
/** END Machine State Definition **/

/** BEGIN Machine State Interface Definitions **/ //USE ALL OF THESE FUNCIONS
void initializeMemory(wordType memorySize); 
void initializeRegisters();
void loadMemory(FILE *input);
byteType getByteFromMemory(wordType address); //memory
void setByteInMemory(wordType address, byteType value); //memory
wordType getWordFromMemory(wordType address); //memory
void setWordInMemory(wordType address, wordType value); //memory

wordType getRegister(int registerID); //register 
void setRegister(int registerID, wordType value); //register 

bool Cond(int ifun); //condition code 
void setFlags(bool sf, bool zf, bool of); // condition code

status getStatus(); //machine status 
void setStatus(status newStatus); //machine status

wordType getPC(); //PC
void setPC(wordType value); //PC

void incrementCycleCounter();
int getCycleCount();

void printMachineState();
/** END Machine State Interface Definitions **/

/** BEGIN Instruction IDs **/
extern const int HALT;
extern const int NOP;
extern const int RRMOVQ;
extern const int CMOVXX;
extern const int IRMOVQ;
extern const int RMMOVQ;         // THESE ARE ASSOCIATED WITH ICODE 
extern const int MRMOVQ;         
extern const int OPQ;
extern const int JXX;
extern const int CALL;
extern const int RET;
extern const int PUSHQ;
extern const int POPQ;
/** END Instruction IDs **/

/** BEGIN Function IDs **/
extern const int ADD;
extern const int SUB;               // THESE ARE ASSOCIATED WITH IFUN
extern const int AND;               // JXX uses IFUN as well
extern const int XOR;
/** END Function IDs **/

/** BEGIN Register IDs **/
extern const int RSP;               // THIS FOR %RSP
/** END Register IDs **/

void applyStageStepMode(int stepMode, char *stageID, int icode, int ifun, int rA, int rB, wordType valC, wordType valP, 
          wordType valA, wordType valB, wordType valE, bool Cnd, wordType valM);
void applyStepMode(int stepMode);
#endif