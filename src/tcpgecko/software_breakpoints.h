// https://github.com/Kinnay/DiiBugger/blob/master/wiiu_code/server.cpp

#ifndef TCPGECKO_BREAKPOINT_EXECUTE_H
#define TCPGECKO_BREAKPOINT_EXECUTE_H

#include "utilities.h"
#include "../system/exception_handler.h"

#include <coreinit/exception.h>
#include <coreinit/debug.h>
#include <coreinit/exception.h>

struct Breakpoint {
	uint32_t address;
	uint32_t instruction;
};

#define INSTRUCTION_TRAP 0x7FE00008 // https://stackoverflow.com/a/10286705/3764804
#define INSTRUCTION_NOP 0x60000000

#define STEP1 10
#define STEP2 11

#define GENERAL_BREAKPOINTS_COUNT 10
#define STEP_BREAKPOINTS_COUNT 2

unsigned char ProgramHandler_Debug(void *interruptedContext) {
	OSFatal("Hi");

	return 0;
}

//I don't think ProgramHandler_Debug will work here with WUT
void installBreakpointHandler() {
	OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_PROGRAM, ProgramHandler_Debug);
}

struct Breakpoint breakpoints[GENERAL_BREAKPOINTS_COUNT + STEP_BREAKPOINTS_COUNT];

struct Breakpoint *removeBreakpoint(struct Breakpoint *breakpoint) {
	writeCode(breakpoint->address, breakpoint->instruction);
	breakpoint->address = 0;
	breakpoint->instruction = 0;

	return breakpoint;
}

struct Breakpoint *setBreakpoint(struct Breakpoint *breakpoint, uint32_t address) {
	breakpoint->address = address;
	breakpoint->instruction = address;
	writeCode(address, (u32) INSTRUCTION_TRAP);

	return breakpoint;
}

struct Breakpoint *getBreakpoint(uint32_t address, int size) {
	for (int index = 0; index < GENERAL_BREAKPOINTS_COUNT; index++) {
		if (breakpoints[index].address == address) {
			return &breakpoints[index];
		}
	}

	return NULL;
}

void pokeRegisters(OSContext context, uint32_t gpr[32], double fpr[32]) {
	memcpy(&context.gpr, &gpr, 32 * sizeof(uint32_t));
	// memcpy(&context.fpr, &fpr, sizeof(fpr));
}

struct Breakpoint *allocateBreakpoint() {
	for (int breakpointsIndex = 0; breakpointsIndex < GENERAL_BREAKPOINTS_COUNT; breakpointsIndex++) {
		if (breakpoints[breakpointsIndex].address == 0) {
			return &breakpoints[breakpointsIndex];
		}
	}

	return NULL;
}

// TODO
uint32_t stepSource;

void RestoreStepInstructions() {
	writeCode(breakpoints[STEP1].address, breakpoints[STEP1].instruction);
	breakpoints[STEP1].address = 0;
	breakpoints[STEP1].instruction = 0;
	if (breakpoints[STEP2].address) {
		writeCode(breakpoints[STEP2].address, breakpoints[STEP2].instruction);
		breakpoints[STEP2].address = 0;
		breakpoints[STEP2].instruction = 0;
	}

	struct Breakpoint *breakpoint = getBreakpoint(stepSource, GENERAL_BREAKPOINTS_COUNT);
	if (breakpoint) {
		writeCode(breakpoint->address, (uint32_t) INSTRUCTION_TRAP);
	}
}

uint32_t getInstruction(uint32_t address) {
	struct Breakpoint *breakpoint = getBreakpoint(address, GENERAL_BREAKPOINTS_COUNT + STEP_BREAKPOINTS_COUNT);
	if (breakpoint != NULL) {
		return breakpoint->instruction;
	}

	return *(uint32_t *) address;
}

struct Breakpoint *getBreakpointRange(uint32_t address, uint32_t length, struct Breakpoint *previousBreakpoint) {
	unsigned long startingIndex = 0;
	if (previousBreakpoint) {
		startingIndex = (previousBreakpoint - breakpoints) + 1;
	}

	for (unsigned long index = startingIndex; index < GENERAL_BREAKPOINTS_COUNT + STEP_BREAKPOINTS_COUNT; index++) {
		struct Breakpoint breakpoint = breakpoints[index];

		if (breakpoint.address >= address && breakpoint.address < address + length) {
			return &breakpoints[index];
		}
	}

	return NULL;
}

// TODO
OSContext crashContext;

void predictStepAddresses(bool stepOver) {
	uint32_t currentAddress = crashContext.srr0;
	uint32_t instruction = getInstruction(currentAddress);

	struct Breakpoint *step1 = &breakpoints[STEP1];
	struct Breakpoint *step2 = &breakpoints[STEP2];
	step1->address = currentAddress + 4;
	step2->address = 0;

	uint8_t opcode = instruction >> 26;
	if (opcode == 19) {
		uint16_t XO = (instruction >> 1) & 0x3FF;
		bool LK = instruction & 1;
		if (!LK || !stepOver) {
			if (XO == 16) step2->address = crashContext.lr; // bclr
			if (XO == 528) step2->address = crashContext.ctr; // bcctr
		}
	} else if (opcode == 18) { //b
		bool AA = instruction & 2;
		bool LK = instruction & 1;
		uint32_t LI = instruction & 0x3FFFFFC;
		if (!LK || !stepOver) {
			if (AA) step1->address = LI;
			else {
				if (LI & 0x2000000) LI -= 0x4000000;
				step1->address = currentAddress + LI;
			}
		}
	} else if (opcode == 16) { //bc
		bool AA = instruction & 2;
		bool LK = instruction & 1;
		uint32_t BD = instruction & 0xFFFC;
		if (!LK || !stepOver) {
			if (AA) step2->address = BD;
			else {
				if (BD & 0x8000) BD -= 0x10000;
				step2->address = currentAddress + BD;
			}
		}
	}
}

void ReportCrash(uint32_t msg) {
	/*crashState = CRASH_STATE_UNRECOVERABLE;

	struct OSMessage messageStruct;
	messageStruct.message = msg;
	messageStruct.data0 = (u32) & crashContext;
	messageStruct.data1 = sizeof(crashContext);
	OSSendMessage(&serverQueue, &messageStruct, OS_MESSAGE_BLOCK);
	while (true) {
		OSSleepTicks((u64) 1000000);
	}*/
}

/*void HandleProgram(OSContext crashContext) {
	//Check if the exception was caused by a breakpoint
	if (!(crashContext.srr1 & 0x20000)) {
		ReportCrash(SERVER_MESSAGE_PROGRAM);
	}

	// Special case, the twu instruction at the start
	if (crashContext.srr0 == (u32) entryPoint + 0x48) {
		writeCode(crashContext.srr0, (u32) INSTRUCTION_NOP);
	}

	if (stepState == STEP_STATE_RUNNING || stepState == STEP_STATE_STEPPING) {
		crashState = CRASH_STATE_BREAKPOINT;

		OSMessage message;
		message.message = SERVER_MESSAGE_PROGRAM;
		message.data0 = (u32) & crashContext;
		message.data1 = sizeof(crashContext);
		OSSendMessage(&serverQueue, &message, OS_MESSAGE_BLOCK);
		OSReceiveMessage(&clientQueue, &message, OS_MESSAGE_BLOCK);

		if (stepState == STEP_STATE_STEPPING) {
			RestoreStepInstructions();
		}

		Breakpoint *breakpoint = getBreakpoint(crashContext.srr0);
		if (breakpoint != NULL) {
			writeCode(breakpoint->address, breakpoint->instruction);
		}

		PredictStepAddresses((u32) message.message == CLIENT_MESSAGE_STEP_OVER);
		breakpoints[STEP1].instruction = *(uint32_t * )(breakpoints[STEP1].address);
		writeCode(breakpoints[STEP1].address, TRAP);
		if (breakpoints[STEP2].address) {
			breakpoints[STEP2].instruction = *(uint32_t * )(breakpoints[STEP2].address);
			writeCode(breakpoints[STEP2].address, TRAP);
		}

		stepSource = crashContext.srr0;

		if ((u32) message.message == CLIENT_MESSAGE_CONTINUE) stepState = STEP_STATE_CONTINUE;
		else stepState = STEP_STATE_STEPPING;
	} else if (stepState == STEP_STATE_CONTINUE) {
		RestoreStepInstructions();
		stepState = STEP_STATE_RUNNING;
		crashState = CRASH_STATE_NONE;
	}

	// Resume execution
	OSLoadContext(&crashContext);
}*/

#endif