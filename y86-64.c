#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

const int MAX_MEM_SIZE  = (1 << 13);

void fetchStage(int *icode, int *ifun, int *rA, int *rB, wordType *valC, wordType *valP) {

	// get icode:ifun
	int byte = getByteFromMemory(getPC());
	*icode = byte >> 4;
	*ifun = byte & 0xf;

	// get rA:rB
	if(*icode == OPQ ||*icode == RRMOVQ ||*icode == IRMOVQ ||*icode == PUSHQ ||*icode == POPQ ||*icode == RMMOVQ ||*icode == MRMOVQ || *icode == CMOVXX){
		int byte2 = getByteFromMemory(getPC()+1);
		*rA = byte2 >> 4;
		*rB = byte2 & 0xf;
	}

	// set valC
	if(*icode == IRMOVQ || *icode == MRMOVQ || *icode == RMMOVQ){
		*valC = getWordFromMemory(getPC()+2);
	}
	else if(*icode == JXX || *icode == CALL){
		*valC = getWordFromMemory(getPC()+1);
	}


	// set valP
	if(*icode == HALT ||*icode == NOP ||*icode == RET) {
	*valP = getPC() + 1;
	}
	else if(*icode == IRMOVQ || *icode == RMMOVQ || *icode == MRMOVQ){
		*valP = getPC() + 10;
	}
	else if(*icode == OPQ || *icode == RRMOVQ || *icode == PUSHQ || *icode == POPQ || *icode == CMOVXX){
		*valP = getPC() + 2;
	}
	else if(*icode == JXX || *icode == CALL){
		*valP = getPC() + 9;
	}

}

void decodeStage(int icode, int rA, int rB, wordType *valA, wordType *valB) {
 	// set valA
	if(icode == RRMOVQ || icode == RMMOVQ || icode == OPQ || icode == PUSHQ || icode == CMOVXX){
		*valA =getRegister(rA);}
	else if(icode == RET || icode == POPQ){
		*valA = getRegister(RSP);	
		}

	// set valB
	if(icode == RMMOVQ || icode == MRMOVQ || icode == OPQ){
		*valB = getRegister(rB);
	}
	else if(icode == CALL || icode == RET || icode == PUSHQ || icode == POPQ){
		*valB = getRegister(RSP);
	}
	
}

void executeStage(int icode, int ifun, wordType valA, wordType valB, wordType valC, wordType *valE, bool *Cnd) {

	// set valE
	if(icode == IRMOVQ || icode == RMMOVQ || icode == MRMOVQ){
		*valE = valB + valC;
	}
	else if(icode == RRMOVQ || icode == CMOVXX){
		*valE = valB + valA;
	}
	else if(icode == CALL || icode == PUSHQ){
		*valE = valB + (-8);
	}
	else if(icode == RET || icode == POPQ){
		*valE = valB + 8;
	}
	else if(icode == OPQ){
		if(ifun == ADD){
			*valE = valB + valA;
		}
		else if(ifun == SUB){
			valA = valA * -1;
			*valE = valB + valA;
		}
		else if(ifun == AND){
			*valE = valB & valA;
		}
		else if(ifun == XOR){
			*valE = valB ^ valA;
		}
	}

	// set and check CC
	if(icode == OPQ){
		if(*valE == 0){
			zeroFlag = TRUE;
		}
		else{
			zeroFlag = FALSE;
		}
		if(*valE < 0){
			signFlag = TRUE;
		}
		else{
			signFlag = FALSE;
		}
		if(ifun == SUB || ifun == ADD){
			if(((valB < 0) == (valA < 0)) && ((*valE < 0) != (valB < 0))){
				overflowFlag = TRUE;
			}
			else{
				overflowFlag = FALSE;
			}
		}
		setFlags(signFlag, zeroFlag, overflowFlag);
	}

	if(icode == JXX){
		*Cnd = Cond(ifun);
	}
	
  
}

void memoryStage(int icode, wordType valA, wordType valP, wordType valE, wordType *valM) {

 //put register val in memory
	if(icode == RMMOVQ || icode == PUSHQ){
		setWordInMemory(valE,valA);
	}
	else if(icode == CALL){
		setWordInMemory(valE,valP);
	}
	else if(icode == MRMOVQ){
		*valM = getWordFromMemory(valE);
	}
	else if(icode == RET || icode == POPQ){
		*valM = getWordFromMemory(valA);
	}


}

void writebackStage(int icode, int rA, int rB, wordType valE, wordType valM) {
		
	if(icode == OPQ ||icode == RRMOVQ || icode == IRMOVQ || icode == OPQ || icode == CMOVXX){
 		setRegister(rB,valE);}
	else if(icode == MRMOVQ){
		setRegister(rA,valM);
	}
	else if(icode == CALL || icode == RET || icode == POPQ || icode == PUSHQ){
		setRegister(RSP,valE);
	}

	if(icode == POPQ){
		setRegister(rA,valM);
	}

}

void pcUpdateStage(int icode, wordType valC, wordType valP, bool Cnd, wordType valM) {

	if(icode == HALT){
		setPC(valP);
		setStatus(STAT_HLT);
	}
	else if (icode == RET){
   setPC(valM);
	}
	else if(icode == CALL){
		setPC(valC);
	}
	else if(icode == JXX){
		if(Cnd == TRUE){
			setPC(valC);
		}
		else{
			setPC(valP);
		}
	}
	else{
		setPC(valP);
	}
	
}

void stepMachine(int stepMode) {
  /* FETCH STAGE */
  int icode = 0, ifun = 0;
  int rA = 0, rB = 0;
  wordType valC = 0;
  wordType valP = 0;
 
  /* DECODE STAGE */
  wordType valA = 0;
  wordType valB = 0;

  /* EXECUTE STAGE */
  wordType valE = 0;
  bool Cnd = 0;

  /* MEMORY STAGE */
  wordType valM = 0;

  fetchStage(&icode, &ifun, &rA, &rB, &valC, &valP);
  applyStageStepMode(stepMode, "Fetch", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);

  decodeStage(icode, rA, rB, &valA, &valB);
  applyStageStepMode(stepMode, "Decode", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  executeStage(icode, ifun, valA, valB, valC, &valE, &Cnd);
  applyStageStepMode(stepMode, "Execute", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  memoryStage(icode, valA, valP, valE, &valM);
  applyStageStepMode(stepMode, "Memory", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  writebackStage(icode, rA, rB, valE, valM);
  applyStageStepMode(stepMode, "Writeback", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  pcUpdateStage(icode, valC, valP, Cnd, valM);
  applyStageStepMode(stepMode, "PC update", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);

  incrementCycleCounter();
}

/** 
 * main
 * */
int main(int argc, char **argv) {
  int stepMode = 0;
  FILE *input = parseCommandLine(argc, argv, &stepMode);

  initializeMemory(MAX_MEM_SIZE);
  initializeRegisters();
  loadMemory(input);

  applyStepMode(stepMode);
  while (getStatus() != STAT_HLT) {
    stepMachine(stepMode);
    applyStepMode(stepMode);
#ifdef DEBUG
    printMachineState();
    printf("\n");
#endif
  }
  printMachineState();
  return 0;
}