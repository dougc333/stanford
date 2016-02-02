#ifndef REGISTERS_H
#define REGISTERS_H

// User program CPU state.  The full set of MIPS registers, plus a few
// more because we need to be able to start/stop a user program between
// any two instructions (thus we need to keep track of things like load
// delay slots, etc.)

#define StackReg	29	// User's stack pointer
#define RetAddrReg	31	// Holds return address for procedure calls
#define NumGPRegs	32	// 32 general purpose registers on MIPS
#define HiReg		32	// Double register to hold multiply result
#define LoReg		33
#define PCReg		34	// Current program counter
#define NextPCReg	35	// Next program counter (for branch delay) 
#define PrevPCReg	36	// Previous program counter (for debugging)
#define LoadReg		37	// The register target of a delayed load.
#define LoadValueReg 	38	// The value to be loaded by a delayed load.
#define BadVAddrReg	39	// The failing virtual address on an exception

#define NumTotalRegs 	40

#endif // REGISTERS_H
