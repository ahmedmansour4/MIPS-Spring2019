#include "spimcore.h"
// Created by Ahmed Mansour, Mark, and Will

// Chooses operation based on ALUControl values represented in binary
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
	// Different options for all ALUControl codes
	// Z -> ALUResult
	switch(ALUControl) {
	
		// Z = A + B
		case 0b000:
			*ALUresult = (A + B);
			break;
			
		// Z = A - B
		case 0b001:
			*ALUresult = A - B;
			break;
			
		// A < B ? Z = 1 : Z = 0  !{SIGNED}!
		case 0b010:
			if ((int)A < (int)B)
				*ALUresult = 1;
			else
				*ALUresult = 0;
			break;

		// A < B ? Z = 1 : Z = 0  !{UNSIGNED}!  
		case 0b011:
			if (A < B)
				*ALUresult = 1;
			else
				*ALUresult = 0;
			break;

		// Z = A & B
		case 0b100:
			*ALUresult = A & B;
			break;
			
		// Z = A | B
		case 0b101:
			*ALUresult = A | B;
			break;

		// Z = A << B
		case 0b110:
			*ALUresult = (B << 16);
			break;
			
		// Z = ~A
		case 0b111:
			*ALUresult = ~A;
			break;
	}

	if (*ALUresult == 0)
		*Zero = 1;
	else
		*Zero = 0;
	
	return;
}

// Sets instruction from Mem array indexed by PC
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
	// If the index is not word aligned, halt
	if (PC % 4 != 0)
		return 1;
 
	// If out of bounds in memory [4096 - 16384], halt
	if (PC < 0x0000 || PC > 0x4096 )
		return 1;
 
	// Otherwise we set the instruction value to MEM[PC offset 2]
	*instruction = Mem[(PC >> 2)];
	
	return 0;
}

// Partitions 32 bit instruction code for each type (R, I, J)
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
	// Bits 31 - 26 (R/I/J) 11111100000000000000000000000000(binary) -> 0xFC000000(hex)
	*op = (instruction & 0xFC000000) >> 26;

	// Bits 25 - 21 (R/I) 00000011111000000000000000000000(binary) -> 0x03e00000(hex)
	*r1 = (instruction & 0x03E00000) >> 21;

	// Bits 20 - 16 (R/I) 00000000000111110000000000000000(binary) - > 0x001F0000(hex)
	*r2 = (instruction & 0x001F0000) >> 16;

	// Bits 15 - 11 (R) 00000000000000001111100000000000(binary) -> 0x0000F800)(hex)
	*r3 = (instruction & 0x0000F800) >> 11;

	/* Shamt 10 - 6 not included. Only used for shift instructions */

	// Bits 5 - 0 (R) 00000000000000000000000000111111(binary) -> 0x0000003F(hex)
	*funct = instruction & 0x0000003F;

	// Bits 15 - 0 (I) 00000000000000001111111111111111(binary) -> 0x0000FFFF(hex)
	*offset = instruction & 0x0000FFFF;

	// Bits 25 - 0 (J) 00000011111111111111111111111111(binary) -> 0x03FFFFFF(hex)
	*jsec = instruction & 0x03FFFFFF;

  return;
}

int instruction_decode(unsigned op,struct_controls *controls)
{
	// Sets the control unit values depending on the current op code
	
	switch (op) { 
		// R-type instruction, all op codes are the same
		case 0:
			controls->RegDst = 1;
			controls->Jump = 0;
			controls->ALUSrc = 0;
			controls->MemtoReg = 0;
			controls->RegWrite = 1;
			controls->MemRead = 0;
			controls->MemWrite = 0;
			controls->Branch = 0;
			controls->ALUOp = 7;
			break;
		// J-type instruction, only includes jump
		case 2:
			controls->RegDst = 2;
			controls->Jump = 1;
			controls->ALUSrc = 2;
			controls->MemtoReg = 2;
			controls->RegWrite = 0;
			controls->MemRead = 0;
			controls->MemWrite = 0;
			controls->Branch = 2;
			controls->ALUOp = 0;
			break;
		// Branch on equal instruction
		case 4:
			controls->RegDst = 2;
			controls->Jump = 0;
			controls->ALUSrc = 0;
			controls->MemtoReg = 2;
			controls->RegWrite = 0;
			controls->MemRead = 0;
			controls->MemWrite = 0;
			controls->Branch = 1;
			controls->ALUOp = 1;
			break;
		// Branch on not equal instruction
		case 5:
			controls->RegDst = 2;
			controls->Jump = 1;
			controls->ALUSrc = 0;
			controls->MemtoReg = 2;
			controls->RegWrite = 0;
			controls->MemRead = 0;
			controls->MemWrite = 0;
			controls->Branch = 1;
			controls->ALUOp = 7;
			break;
		// Add immediate instruction
		case 8:
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->ALUSrc = 1;
			controls->MemtoReg = 0;
			controls->RegWrite = 1;
			controls->MemRead = 0;
			controls->MemWrite = 0;
			controls->Branch = 0;
			controls->ALUOp = 0;
			break;
		// Set less than immediate instruction
		case 10:
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->ALUSrc = 1;
			controls->MemtoReg = 2;
			controls->RegWrite = 1;
			controls->MemRead = 0;
			controls->MemWrite = 0;
			controls->Branch = 0;
			controls->ALUOp = 2;
			break;
		// Set less than immediate unsigned instruction
		case 11:
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->ALUSrc = 1;
			controls->MemtoReg = 0;
			controls->RegWrite = 1;
			controls->MemRead = 0;
			controls->MemWrite = 0;
			controls->Branch = 0;
			controls->ALUOp = 3;
			break;
		// And immediate instruction
		case 12:
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->ALUSrc = 1;
			controls->MemtoReg = 0;
			controls->RegWrite = 1;
			controls->MemRead = 0;
			controls->MemWrite = 0;
			controls->Branch = 0;
			controls->ALUOp = 4;
			break;
		// Or immediate instruction
		case 13:
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->ALUSrc = 1;
			controls->MemtoReg = 0;
			controls->RegWrite = 1;
			controls->MemRead = 0;
			controls->MemWrite = 0;
			controls->Branch = 0;
			controls->ALUOp = 5;
			break;
		// Load upper immediate instruction
		case 15:
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->ALUSrc = 1;
			controls->MemtoReg = 0;
			controls->RegWrite = 1;
			controls->MemRead = 0;
			controls->MemWrite = 0;
			controls->Branch = 0;
			controls->ALUOp = 6;
			break;
		// Load word instruction
		case 35:
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->ALUSrc = 1;
			controls->MemtoReg = 1;
			controls->RegWrite = 1;
			controls->MemRead = 1;
			controls->MemWrite = 0;
			controls->Branch = 0;
			controls->ALUOp = 0;
			break;
		// Store word instruction
		case 43: 
			controls->RegDst = 2;
			controls->Jump = 0;
			controls->ALUSrc = 1;
			controls->MemtoReg = 2;
			controls->RegWrite = 0;
			controls->MemRead = 0;
			controls->MemWrite = 1;
			controls->Branch = 0;
			controls->ALUOp = 0;
			break;
		// Given invalid opcode, halt
		default:
			return 1;
	}
	
	return 0; 
}

// Sets the values given the register data 
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
	// Access the register using it's address and put its value in each data variable
	*data1 = Reg[r1];
	*data2 = Reg[r2];
	
	return;
}

// Takes 16 bit offset and extends it to 32 bits 
void sign_extend(unsigned offset,unsigned *extended_value)
{
	// If negative, ensure that it stays negative
	if (offset >> 15) 
	{
		*extended_value = offset | 0b11111111111111110000000000000000;
	}
	// If positive, pad with 0's 
	else
	{
		*extended_value = offset & 0b00000000000000001111111111111111;
	}
	
	return;
}

// Determines what to send to ALU 
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
	char tempALUOp;
	
	// If ALUSrc is asserted then use extended_value instead of data2 in the ALU function call
	if (ALUSrc != 0)
		data2 = extended_value;
	
	// If using an r-type function, interpret the ALU operation from funct
	if(ALUOp == 7)
	{
		// Addition
		if (funct == 32)
		{
			tempALUOp == 0;
		}
		// Subtraction
		else if (funct == 34)
		{
			tempALUOp == 1;
		}
		// Set Less Signed
		else if (funct == 42)
		{
			tempALUOp == 2;
		}
		// Set Less Unsigned
		else if (funct == 43)
		{
			tempALUOp == 3;
		}
		// And
		else if (funct == 36)
		{
			tempALUOp == 4;
		}
		// Or
		else if (funct == 37)
		{
			tempALUOp == 5;
		}
		// Shift Left extended value 16
		else if (funct == 6)
		{
			tempALUOp == 6;
		}
		// Bitwise Not
		else if (funct == 39)
		{
			tempALUOp == 7;
		}
		// Invalid funct, halt
		else
		{
			return 1;
		}
	}
	else 
	{
		tempALUOp = ALUOp;
	}
	// Passes along the operation to the ALU with all the data and addresses needed
	ALU(data1,data2,tempALUOp,ALUresult,Zero);
	return 0;
}

// Reads and writes to memory based MemRead and Memwrite
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{ 

	// If reading from memory, store data from Mem from the correct index
	if (MemRead)
		*memdata = Mem[ALUresult >> 2];
	
	// If writing to memory, store data to Mem in the correct index
	if (MemWrite)
		Mem[ALUresult >> 2] = data2;
	
	
	return 0;
}

// Determines which data to write to write to what register
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
	// If Writing to register
	if (RegWrite)
	{
		// If writing from Memory
		if (MemtoReg)
		{
			// Write to the correct destination
			if (RegDst)
			{
				Reg[r3] = memdata;
			}
			else
			{
				Reg[r2] = memdata;
			}
		}
		// If writing from ALUresult
		else
		{
			// Write to the correct destination
			if (RegDst)
			{
				Reg[r3] = ALUresult;
			}
			else
			{
				Reg[r2] = ALUresult;
			}
		}
	}
	return;
}

// Incrementer for program counter based on jump/branch 
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
	// Adds 4 to the program counter 
	*PC = *PC + 4;

	// If Branch and Zero is asserted, then we are taking a conditional jump
	if (Branch == 1 && Zero == 1)
	{
		extended_value = extended_value << 2;
		
		// Set Program Counter to the new offset *word aligned 
		*PC += extended_value;
	}
	// If Jump is asserted we are taking an unconditional jump
	else if (Jump == 1)
	{
		jsec = jsec << 2;
		
		// Set program counter to be jsec & combine with upper 4 bits of PC (ProjectDetails.pptx) 
		// When turning in code to a proffesor, is it necessary to cite the same professors powerpoint?
		*PC = (*PC & 0xF0000000) | jsec; 

	}
	return;
}
