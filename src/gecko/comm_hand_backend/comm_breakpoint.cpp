#include "../command_handler.h"

#include <kernel/kernel.h>

#define GENERAL_BREAKPOINTS_COUNT 10
#define STEP_BREAKPOINTS_COUNT 2

#define INSTRUCTION_TRAP 0x7FE00008 // https://stackoverflow.com/a/10286705/3764804
#define INSTRUCTION_NOP 0x60000000

struct Breakpoint {
	uint32_t address;
	uint32_t instruction;
};

BreakPointC breakpoints[GENERAL_BREAKPOINTS_COUNT + STEP_BREAKPOINTS_COUNT];

static unsigned char (*bHandler)(OSContext *ctx);

extern /* kernelmode */ void KernelSetDABR(unsigned int dabr);

extern void SC0x2D_KernelSetDABR(unsigned int dabr);

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

class BreakPointC {
public:
	uint32_t address;
	uint32_t instruction;

	void remove(){
		writeCode(address, instruction);
		address = 0;
		instruction = 0;
	}
	void set(uint32_t new_address){
		address = new_address;
		instruction = *(uint32_t *) new_address;
		writeCode(address, (uint32_t) INSTRUCTION_TRAP);
	}
};

void writeCode(uint32_t address, uint32_t instruction) {
	uint32_t *pointer = (uint32_t *) (address + 0xA0000000);
	*pointer = instruction;
	DCFlushRange(pointer, 4);
	ICInvalidateRange(pointer, 4);
}

BreakPointC *getBreakpoint(uint32_t address, int size) {
	for (int index = 0; index < GENERAL_BREAKPOINTS_COUNT; index++) {
		if (breakpoints[index].address == address) {
			return &breakpoints[index];
		}
	}

	return NULL;
}

BreakPointC *allocateBreakpoint() {
	for (int breakpointsIndex = 0; breakpointsIndex < GENERAL_BREAKPOINTS_COUNT; breakpointsIndex++) {
		if (breakpoints[breakpointsIndex].address == 0) {
			return &breakpoints[breakpointsIndex];
		}
	}

	return NULL;
}

unsigned char basicDABRBreakpointHandler(OSContext *context) {
	// log_print("Getting DABR address\n");
	uint32_t address = context->srr0;
	// log_printf("Got DABR address: %08x\n", address);

	if (OSIsAddressValid(address)){
		// log_printf("Data breakpoint address: %x08\n", address);
	} else {
		// log_printf("Data breakpoint invalid address: %x08\n", address);
	}
	return 0;
}


static void RegisterDataBreakpointHandler(unsigned char (*breakpointHandler)(OSContext *ctx)) {
	bHandler = breakpointHandler;
}

static int DataBreakpoints_DSIHandler(OSContext *ctx) {
	// log_print("DSI handler\n");
	/*OSContext *context = (OSContext *) ctx;
	if (context->dsisr & DSISR_DABR_MATCH) {
		// log_print("Running BP handler\n");
		if (bHandler) {
			return bHandler(context);
		}
	}*/

	// log_print("DSI exception\n");
	//return dsi_exception_cb(ctx); //this function is... oh boy. I'm NOT dealing with that rn
	return 0;
}

static void DataBreakpoints_Install() {
	KernelPatchSyscall(0x2D, (uint32_t) &KernelSetDABR);
	OSSetExceptionCallback(OS_EXCEPTION_TYPE_DSI, &DataBreakpoints_DSIHandler);
}

static void SetDataBreakpoint(unsigned int address, bool read, bool write) {
	// log_print("Before installing...\n");
	DataBreakpoints_Install();
	// log_print("After installing...\n");
	RegisterDataBreakpointHandler(basicDABRBreakpointHandler);
	// log_print("After registering...\n");

	unsigned int dabr = address & ~0b00000111;
	// log_printf("DABR 1: %08x\n", dabr);
	dabr |= 0b00000100; //enable translation
	// log_printf("DABR 2: %08x\n", dabr);
	if (read) {
		dabr |= 0b00000001; //break on read
		// log_printf("DABR 3: %08x\n", dabr);
	}
	if (write) {
		dabr |= 0b00000010; //break on write
		// log_printf("DABR 4: %08x\n", dabr);
	}

	// log_print("Setting DABR...\n");
	SC0x2D_KernelSetDABR(dabr);
	// log_print("DABR set!\n");
}

void setDataBreakpoint(int address, bool read, bool write) {
	OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_PROGRAM, &breakPointHandler);
	// log_print("Setting DABR...\n");
	OSSetDABR(1, address, read, write);
	// log_print("DABR set\n");
	// int enabled = OSIsInterruptEnabled();
	// log_printf("Interrupts enabled: %i\n", enabled);
}

int breakPointHandler(OSContext *interruptedContext) {
	// Check for data breakpoints
	uint32_t dataAddress = interruptedContext->srr0;
	if (OSIsAddressValid(dataAddress)) {
		// log_printf("Data breakpoint address: %x08\n", dataAddress);
	} else {
		// log_printf("Data breakpoint invalid address: %x08\n", dataAddress);

		// Check for instruction breakpoints
		uint32_t instructionAddress = interruptedContext->srr0;
		if (OSIsAddressValid(instructionAddress)) {
			// log_printf("Instruction breakpoint address: %x08\n", dataAddress);
		} else {
			// log_print("Instruction breakpoint failed!\n");
		}
	}

	setDataBreakpoint(0, false, false);
	setInstructionBreakpoint(0);

	rfi();

	return 0;
}

void setInstructionBreakpoint(unsigned int address) {
	OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_PROGRAM, &breakPointHandler);

	// int returnedAddress;

	// log_print("Setting IABR #1...\n");
	// OSSetIABR(1, address);
	setIABR(address);
	// log_print("IABR set #1...\n");
	/*
	// TODO Causes crash
	returnedAddress = getIABRAddress();
	// log_printf("IABR spr value: %08x\n", returnedAddress);

	// log_print("Setting IABR #2...\n");
	setIABR(address);
	// log_print("IABR set #2...\n");
	returnedAddress = mfspr(IABR);
	// log_printf("IABR spr value: %08x\n", returnedAddress);*/
}

void CommandHandler::command_set_data_breakpoint(){
	// Read the data from the client
	ret = recvwait(sizeof(int) + sizeof(bool) * 2);
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (data breakpoint)");

	// Parse the data and set the breakpoint
	int bufferIndex = 0;
	uint32_t address = ((uint32_t *)buffer)[bufferIndex];
	bufferIndex += sizeof(int);

	bool read = buffer[bufferIndex];
	bufferIndex += sizeof(bool);

	bool write = buffer[bufferIndex];
	bufferIndex += sizeof(bool);
	
	SetDataBreakpoint(address, read, write);
	// setDataBreakpoint(address, read, write);
};

void CommandHandler::command_set_instruction_breakpoint(){
	// Read the address
	ret = recvwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (instruction breakpoint)");

	// Parse the address and set the breakpoint
	uint32_t address = ((uint32_t *)buffer)[0];
	setInstructionBreakpoint(address);
};

void CommandHandler::command_toggle_breakpoint(){
	// Read the address
	ret = recvwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (toggle breakpoint)");
	uint32_t address = ((uint32_t *)buffer)[0];

	BreakPointC *breakpoint = getBreakpoint(address, GENERAL_BREAKPOINTS_COUNT);

	if (breakpoint != NULL)
	{
		breakpoint->remove();
	}
	else
	{
		breakpoint = allocateBreakpoint();

		if (breakpoint != NULL)
		{
			breakpoint->set(address);
		}
	}
};

void CommandHandler::command_remove_all_breakpoints(){
	//removeAllBreakpoints();
	for (int index = 0; index < GENERAL_BREAKPOINTS_COUNT; index++) {
		BreakPointC *breakpoint = &breakpoints[index];
		if (breakpoint->address != 0) {
			breakpoint->remove();
		}
	}
};