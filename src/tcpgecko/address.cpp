#include <coreinit/memorymap.h>

int validateAddressRange(int starting_address, int ending_address) {
	return __OSValidateAddressSpaceRange(1, starting_address, ending_address - starting_address + 1);
}

/*bool isValidDataAddress(int address) {
	return OSIsAddressValid((const void *) address)
		   && address >= 0x10000000
		   && address < 0x50000000;
}*/

int roundUpToAligned(int number) {
	return (number + 3) & ~0x03;
}