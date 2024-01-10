#ifndef TCPGECKO_STACK_H
#define TCPGECKO_STACK_H

//I think this file is entirely redundant, it's just defines for 
//things in <coreinit/thread.h> and <coreinit/threadqueue.h>

/* typedef struct OSThreadQueue {
	OSThread *head;
	OSThread *tail;
	void *parentStruct;
	uint32_t reserved;
} OSThreadQueue; */

/* Notable difference: data0-3 instead of args[3]
typedef struct OSMessage {
	uint32_t message;
	uint32_t data0;
	uint32_t data1;
	uint32_t data2;
} OSMessage; */

/* typedef struct OSMessageQueue {
	uint32_t tag;
	char *name;
	uint32_t reserved;

	OSThreadQueue sendQueue;
	OSThreadQueue recvQueue;
	OSMessage *messages;
	int msgCount;
	int firstIndex;
	int usedCount;
} OSMessageQueue; */

/* typedef struct OSContext {
	char tag[8];

	uint32_t gpr[32];

	uint32_t cr;
	uint32_t lr;
	uint32_t ctr;
	uint32_t xer;

	uint32_t srr0;
	uint32_t srr1;

	uint32_t ex0;
	uint32_t ex1;

	uint32_t exception_type;
	uint32_t reserved;

	double fpscr;
	double fpr[32];

	uint16_t spinLockCount;
	uint16_t state;

	uint32_t gqr[8];
	uint32_t pir;
	double psf[32];

	u64 coretime[3];
	u64 starttime;

	uint32_t error;
	uint32_t attributes;

	uint32_t pmc1;
	uint32_t pmc2;
	uint32_t pmc3;
	uint32_t pmc4;
	uint32_t mmcr0;
	uint32_t mmcr1;
} OSContext;

typedef int (*ThreadFunc)(int argc, void *argv);

typedef struct OSThread {
	OSContext context;

	uint32_t txtTag; //uint32_t tag
	uint8_t state; //OSThreadState state
	uint8_t attr; //OSThreadAttributes attr

	short threadId; 
	int suspend; //
	int priority; //int32_t priority

	char _[0x394 - 0x330];

	void *stackBase;
	void *stackEnd;

	ThreadFunc entryPoint;

	char _3A0[0x6A0 - 0x3A0];
} OSThread; */

#endif