#include <stdbool.h>
//#include "../dynamic_libs/os_functions.h"
#include <coreinit/context.h>

#include "../system/exception_handler.h"
//#include "../kernel/syscalls.h"
#include <kernel/kernel.h>

#include "../common/common.h"
//#include "../utils/logger.h"
#include <whb/log.h>
#include "software_breakpoints.h"

#ifndef TCPGECKO_HARDWARE_BREAKPOINTS_H
#define TCPGECKO_HARDWARE_BREAKPOINTS_H

#ifdef __cplusplus
extern "C" {
#endif

/*	Fix a gap in exception_handler.h
    Yes, this is the dsisr.
*/
#define dsisr exception_specific0
#define DSISR_DABR_MATCH 0x400000

extern /* kernelmode */ void KernelSetDABR(unsigned int dabr);

extern void SC0x2D_KernelSetDABR(unsigned int dabr);

static void RegisterDataBreakpointHandler(unsigned char (*breakpointHandler)(OSContext *ctx));

static void DataBreakpoints_Install();

static unsigned char DataBreakpoints_DSIHandler(void *ctx);

static unsigned char (*bHandler)(OSContext *ctx);

static void RegisterDataBreakpointHandler(unsigned char (*breakpointHandler)(OSContext *ctx)) {
	bHandler = breakpointHandler;
}

static inline int getDABRAddress(OSContext *interruptedContext) {
	OSContext *context = interruptedContext;
	return (int) context->srr0; // Offset 0xA4
}

unsigned char basicDABRBreakpointHandler(OSContext *context) {
	WHBLogPrintf("Getting DABR address\n");
	int address = getDABRAddress(context);
	WHBLogPrintf("Got DABR address: %08x\n", address);

	if (OSIsAddressValid(address)) {
		WHBLogPrintf("Data breakpoint address: %x08\n", address);
	} else {
		WHBLogPrintf("Data breakpoint invalid address: %x08\n", address);
	}

	return 0;
}

static void SetDataBreakpoint(unsigned int address, bool read, bool write) {
	WHBLogPrint("Before installing...\n");
	DataBreakpoints_Install();
	WHBLogPrint("After installing...\n");
	RegisterDataBreakpointHandler(basicDABRBreakpointHandler);
	WHBLogPrint("After registering...\n");

	unsigned int dabr = address & ~0b00000111; //GCC \o/
	WHBLogPrintf("DABR 1: %08x\n", dabr);
	dabr |= 0b00000100; //enable translation
	WHBLogPrintf("DABR 2: %08x\n", dabr);
	if (read) {
		dabr |= 0b00000001; //break on read
		WHBLogPrintf("DABR 3: %08x\n", dabr);
	}
	if (write) {
		dabr |= 0b00000010; //break on write
		WHBLogPrintf("DABR 4: %08x\n", dabr);
	}

	WHBLogPrint("Setting DABR...\n");
	SC0x2D_KernelSetDABR(dabr);
	WHBLogPrint("DABR set!\n");
}

static unsigned char DataBreakpoints_DSIHandler(OSContext *ctx) {
	WHBLogPrint("DSI handler\n");
	/*OSContext *context = (OSContext *) ctx;
	if (context->dsisr & DSISR_DABR_MATCH) {
		// WHBLogPrint("Running BP handler\n");
		if (bHandler) {
			return bHandler(context);
		}
	}*/

	WHBLogPrint("DSI exception\n");
	return dsi_exception_cb(ctx);
}

static void DataBreakpoints_Install() {
	kern_write((void *) (OS_SPECIFICS->addr_KernSyscallTbl1 + (0x2D * 4)), (unsigned int) &KernelSetDABR);
	kern_write((void *) (OS_SPECIFICS->addr_KernSyscallTbl2 + (0x2D * 4)), (unsigned int) &KernelSetDABR);
	kern_write((void *) (OS_SPECIFICS->addr_KernSyscallTbl3 + (0x2D * 4)), (unsigned int) &KernelSetDABR);
	kern_write((void *) (OS_SPECIFICS->addr_KernSyscallTbl4 + (0x2D * 4)), (unsigned int) &KernelSetDABR);
	kern_write((void *) (OS_SPECIFICS->addr_KernSyscallTbl5 + (0x2D * 4)), (unsigned int) &KernelSetDABR);

	OSSetExceptionCallback(OS_EXCEPTION_TYPE_DSI, &DataBreakpoints_DSIHandler);
}

// Special purpose registers
#define IABR 0x3F2
#define DABR 0x3F5

// http://www.ds.ewi.tudelft.nl/vakken/in1006/instruction-set/mtspr.html
#define mtspr(spr, value)                        \
    __asm__ __volatile__ ("mtspr %0, %1" : : "K" (spr), "r" (value))    \


// https://www.ibm.com/support/knowledgecenter/en/ssw_aix_71/com.ibm.aix.alangref/idalangref_isync_ics_instrs.htm
static inline void isync() {
	__asm__ __volatile__ ("isync" : : : "memory");
}

// https://www.ibm.com/support/knowledgecenter/en/ssw_aix_61/com.ibm.aix.alangref/idalangref_eieio_instrs.htm
static inline void eieio() {
	__asm__ __volatile__ ("eieio" : : : "memory");
}

// https://www.ibm.com/support/knowledgecenter/ssw_aix_71/com.ibm.aix.alangref/idalangref_rfi_retfinter_instrs.htm
static inline void rfi() {
	__asm__ __volatile__ ("rfi" : : : "memory");
}

// https://www.manualslib.com/manual/606065/Ibm-Powerpc-750gx.html?page=64
static inline void setIABR(unsigned int address) {
	mtspr(IABR, address);
	eieio();
	isync();
}

// http://elixir.free-electrons.com/linux/v2.6.24/source/include/asm-powerpc/reg.h#L713
#define mfspr(spr) \
({    register uint32_t _rval = 0; \
    asm volatile("mfspr %0," __stringify(spr) \
    : "=r" (_rval));\
    _rval; \
})

static inline int getIABRAddress() {
	return mfspr(IABR);
}

static inline int getIABRMatch(void *interruptedContext) {
	OSContext *context = (OSContext *) interruptedContext;
	//return (int) context->exception_specific1; // Offset 0x98
	return context->srr0; //Guess based on the above comment and the offset check in WUT
}

unsigned char breakPointHandler(void *interruptedContext);

void registerBreakPointHandler() {
	WHBLogPrint("Registering breakpoint handler...\n");
	// TODO Not working, never called?
	// OSSetExceptionCallback((u8) OS_EXCEPTION_DSI, &breakPointHandler);
	// OSSetExceptionCallback((u8) OS_EXCEPTION_ISI, &breakPointHandler);
	// OSSetExceptionCallback((u8) OS_EXCEPTION_PROGRAM, &breakPointHandler);
	OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_PROGRAM, &breakPointHandler);
	// __OSSetInterruptHandler((u8) OS_EXCEPTION_PROGRAM, &breakPointHandler);
	WHBLogPrint("Breakpoint handler(s) registered!\n");
}

/*void forceDebuggerInitialized() {
	unsigned char patchBytes[] = {0x38, 0x60, 0x00, 0x01};
	patchFunction(OSIsDebuggerInitialized, (char *) patchBytes, sizeof(patchBytes), 0x1C);
}

void forceDebuggerPresent() {
	unsigned char patchBytes[] = {0x38, 0x60, 0x00, 0x01, 0x60, 0x00, 0x00, 0x00};
	patchFunction(OSIsDebuggerPresent, (char *) patchBytes, sizeof(patchBytes), 0x0);
}*/

static inline void setupBreakpointSupport() {
	/*WHBLogPrint("Clear and enable...\n");
	__OSClearAndEnableInterrupt();
	WHBLogPrint("Restore...\n");
	OSRestoreInterrupts();
	WHBLogPrint("Enable...\n");
	OSEnableInterrupts();
	forceDebuggerPresent();
	forceDebuggerInitialized();*/

	registerBreakPointHandler();
}

void setDataBreakpoint(int address, bool read, bool write) {
	setupBreakpointSupport();
	WHBLogPrint("Setting DABR...\n");
	OSSetDABR(1, address, read, write);
	WHBLogPrint("DABR set\n");
	// int enabled = OSIsInterruptEnabled();
	WHBLogPrintf("Interrupts enabled: %i\n", enabled);
}

void setInstructionBreakpoint(unsigned int address) {
	setupBreakpointSupport();

	// int returnedAddress;

	WHBLogPrint("Setting IABR #1...\n");
	// OSSetIABR(1, address);
	setIABR(address);
	WHBLogPrint("IABR set #1...\n");
	/*
	// TODO Causes crash
	returnedAddress = getIABRAddress();
	WHBLogPrintf("IABR spr value: %08x\n", returnedAddress);

	WHBLogPrint("Setting IABR #2...\n");
	setIABR(address);
	WHBLogPrint("IABR set #2...\n");
	returnedAddress = mfspr(IABR);
	WHBLogPrintf("IABR spr value: %08x\n", returnedAddress);*/
}

unsigned char breakPointHandler(OSContext *interruptedContext) {
	// Check for data breakpoints
	int dataAddress = getDABRAddress(interruptedContext);
	if (OSIsAddressValid(dataAddress)) {
		WHBLogPrintf("Data breakpoint address: %x08\n", dataAddress);
	} else {
		WHBLogPrintf("Data breakpoint invalid address: %x08\n", dataAddress);

		// Check for instruction breakpoints
		int instructionAddress = getIABRMatch(interruptedContext);
		if (OSIsAddressValid(instructionAddress)) {
			WHBLogPrintf("Instruction breakpoint address: %x08\n", dataAddress);
		} else {
			WHBLogPrint("Instruction breakpoint failed!\n");
		}
	}

	setDataBreakpoint(0, false, false);
	setInstructionBreakpoint(0);

	rfi();

	return 0;
}

#ifdef __cplusplus
} //extern "C"
#endif

#endif //TCPGECKO_HARDWARE_BREAKPOINTS_H
