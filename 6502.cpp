#include <iostream>
#include <stdlib.h>
#include <thread>
#include <chrono>
#include <fstream>


using Byte = unsigned char;  // Erstelle Byte als 8-Bit
using Word = unsigned short;  //Erstelle Word als 16-Bit
using u32 = unsigned int;  // Erstelle u32 als 32-Bit
using s32 = signed int;  // Erstelle s32 als 32-Bit mit vorzeichen

struct MEM
{
	static constexpr u32 MAX_MEM = 1024*64; //Erstelle constante MAX_MEM typ u32/int mit wert 1024*64
	Byte Data[MAX_MEM];  // Erstelle Data als 8 stellige adressen von denen es MAX_MEM gibt
	void Initialize(){
		for (u32 i=0;i<MAX_MEM;i++){
			Data[i]=0;
		}
	}
	//read 1 Byte
	Byte operator[](u32 Addres) const{
		//assert here Addres is < MAX_MEM
		return Data[Addres];
	}
	//write 1 Byte
	Byte& operator[](u32 Addres){
		//assert here Addres is < MAX_MEM
		return Data[Addres];
	}
	//Write 2 bytes
	void WriteWord(Word Value, u32 Address, s32& Cycles){
		Data[Address]	=Value & 0xFF;
		Data[Address+1]	=(Value >> 8);
		Cycles-=2;
	}
	//print what is in memory
	void printMem(){
		std::ofstream outFile("memory_dump.txt");
		if (!outFile) {
			std::cerr << "Error opening file!" << std::endl;
		}
		for(u32 i=0;i<MAX_MEM;i++){
			if (Data[i]!=0xEA){
		//		printf("Address:  %X", i);
		//		printf("	Data:  %X", Data[i]);
		//		printf("\n");
		//make a dump file with all not 0xEA addresses in memory after executing programm
				outFile << "Address: " << std::hex << i  << "\tData: " << std::hex << static_cast<int>(Data[i]) << "\n";
			}
		}
		outFile.close();
	}
}; 

struct CPU
{
	Word PC;  //PROGRAM COUNTER
	Word SP;  //STACK POINTER

	Byte A, X, Y; //register

	unsigned int C : 1;  //carry flag
	unsigned int Z : 1;  //zero flag
	unsigned int I : 1;  //interupt disable
	unsigned int D : 1;  //decimal mode
	unsigned int B : 1;  //break command
	unsigned int V : 1;  //overflow flag
	unsigned int N : 1;  //negative flag

	void Reset( MEM& memory ){
		PC=0xFFFC;
		SP=0x0100;

		A=0;  //register A
		X=0;  //register X 
		Y=0;  //register Y

		C=0;  //carry flag
		Z=0;  //zero flag
		I=0;  //interupt disab
		D=0;  //decimal mode
		B=0;  //break command
		V=0;  //overflow flag
		N=0;  //negative flag
		memory.Initialize();
	}
	Byte FetchByte(s32& Cycles, MEM& memory){
		Byte Data=memory[PC];
		PC++;
		Cycles--;
		return Data;
	}
	Word FetchWord(s32& Cycles, MEM& memory){
		//6503 is little endian !!!!!
		Word Data=memory[PC];
		PC++;
		Data|=(memory[PC] << 8);
		PC++;
		Cycles-=2;
		return Data;
	}
	Byte ReadByte(s32& Cycles, Byte Address, MEM& memory){
		Byte Data=memory[Address];
		Cycles--;
		return Data;
	}
	Byte ReadFromWord(s32& Cycles, Word Address, MEM& memory){
		Byte Data=memory[Address];
		Cycles--;
		return Data;
	}

	void printRegister(){
		printf("A = %X \n", A);
		printf("X = %X \n", X);
		printf("Y = %X \n", Y);
		printf("PC = 0x%X \n", PC);
		printf("SP = 0x%X \n", SP);
		printf("C = %X \n", C);
		printf("Z = %X \n", Z);
		printf("I = %X \n", I);
		printf("D = %X \n", D);
		printf("B = %X \n", B);
		printf("V = %X \n", V);
		printf("N = %X \n", N);
	}
	int AddrVerteiler(Word Addr){
		if (Addr<0x7FFF){
			printf("\e[0;32m"  "Succesfuly wrote to Addr: 0x%X \n" "\e[0m", Addr);
			return 1;
		}
		printf("\e[0;31m"  "ERROR: Tried to write to unwritable memory Addr: 0x%X \n" "\e[0m", Addr);
		return 0;
	}

	static constexpr Byte
		INS_NOP=0xEA,
		// load registers:
		INS_LDA_IM=0xA9,
		INS_LDA_ZP=0xA5,
		INS_LDA_ZPX=0xB5,
		INS_LDA_ABS=0xAD,
		INS_LDX_IM=0xA2,
		INS_LDX_ABS=0xAE,
		INS_LDY_IM=0xA0,
		INS_LDY_ABS=0xAC,
		// Arithmetic
		INS_ADC_IM=0x69,
		INS_ADC_ABS=0x6D,
		INS_SBC_IM=0xE9,
		INS_SBC_ABS=0xED,
		// Jumps and Subroutine
		INS_JSR=0x20,
		INS_JMP_ABS=0x4C,
		INS_JMP_IN=0x6C,  //!!!
		// Store Registers in memory
		INS_STA_ABS=0x8D,
		INS_STA_ZP=0x85,
		// Flag controll
		INS_CLC=0x18,
		INS_SEC=0x38,
		INS_CLI=0x58,
		INS_SEI=0x78,
		// Transfer register
		INS_TXA=0x8A,
		INS_TAX=0xAA,
		INS_TYA=0x98,
		INS_TAY=0xA8;

	void Execute(s32 Cycles, MEM& memory, bool wait){
		while(Cycles>0){
			if (wait){
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
			Byte Ins=FetchByte(Cycles, memory);
			switch (Ins){
				case INS_LDA_IM: //2 Cycles
				{
					Byte Value=FetchByte(Cycles, memory);
					A=Value;
					Z=(A==0);
					N=(A&0b10000000)>0;
					printf("INS_LDA_IM %X \n", A);
					//memory.printMem();
				}
				break;
				case INS_LDA_ZP: //3 Cycles
				{
					Byte ZeroPageAddress=FetchByte(Cycles, memory);
					A=ReadByte(Cycles, ZeroPageAddress, memory);
					Z=(A==0);
					N=(A&0b10000000)>0;
					printf("INS_LDA_ZP %X \n", A);
					//memory.printMem();
				}
				break;
				case INS_LDA_ZPX: //4 Cycles
				{
					Byte ZeroPageAddress=FetchByte(Cycles, memory);
					ZeroPageAddress+=X;
					Cycles--;
					A=ReadByte(Cycles, ZeroPageAddress, memory);
					Z=(A==0);
					N=(A&0b10000000)>0;
					printf("INS_LDA_ZPX %X \n", A);
					//memory.printMem();
					//memory.printMem();
				}
				break;
				case INS_LDA_ABS:  //4 Cycles
				{
					Word Addr=FetchWord(Cycles, memory);
					//printf("Addr: %X \n", Addr);
					A=ReadFromWord(Cycles, Addr, memory);
					//printf("vaule: %X \n", Value);
					Cycles--;
					Z=(A==0);
					N=(A&0b10000000)>0;
					printf("INS_LDA_ABS %X \n", A);
					//memory.printMem();

				}
				break;
				case INS_JSR:  //6 Cycles
				{
					printf("INS_JSR %X \n", PC);
					Word SubAddr=FetchWord(Cycles, memory);
					memory.WriteWord(PC-1, SP, Cycles);
					SP+=2;
					PC=SubAddr;
					Cycles--;
					//memory.printMem();
				}
				break;
				case INS_JMP_ABS:  //3 Cycles
				{
					Word Addr=FetchWord(Cycles, memory);
					//printf("Addr: %X \n", Addr);
					PC=Addr;
					Cycles--;
					printf("INS_JMP_ABS %X \n", PC);
				}
				break;
				case INS_STA_ABS:  //3 Cycles
				{
					Word Addr=FetchWord(Cycles, memory);
//					Byte Value=A;
					printf("INS_STA_ABS Data:%X Addr:%X \n", A, Addr);
					if(AddrVerteiler(Addr)==1){
//						memory.WriteWord(A, Addr, Cycles);
						memory[Addr]=A;
					}
					Cycles--;
				}
				break;
				case INS_STA_ZP:  //2 Cycles
				{
					Byte Addr=FetchByte(Cycles, memory);
//					Byte Value=A;
					if(AddrVerteiler(Addr)==1){
//						memory.WriteWord(A, Addr, Cycles);
						memory[Addr]=A;
					}
					Cycles--;
					printf("INS_STA_ZP Data:%X Addr:%X \n", A, Addr);
				}
				break;
				case INS_LDX_IM:  //2 Cycles
				{
					Byte Value=FetchByte(Cycles, memory);
					X=Value;
					Z=(X==0);
					N=(X&0b10000000)>0;
					Cycles--;
					printf("INS_LDX_IM %X \n", X);
				}
				break;
				case INS_LDX_ABS:  //4 Cycles
				{
					Word Addr=FetchWord(Cycles, memory);
					X=ReadFromWord(Cycles, Addr, memory);
					Z=(X==0);
					N=(X&0b10000000)>0;
					Cycles--;
					printf("INS_LDX_ABS %X \n", X);
				}
				break;
				case INS_LDY_IM:  //2 Cycles
				{
					Byte Value=FetchByte(Cycles, memory);
					Y=Value;
					Z=(Y==0);
					N=(Y&0b10000000)>0;
					Cycles--;
					printf("INS_LDY_IM %X \n", Y);
				}
				break;
				case INS_LDY_ABS:  //4 Cycles 	brrrrrrrrrrrr
				{
					Word Addr=FetchWord(Cycles, memory);
					Y=ReadFromWord(Cycles, Addr, memory);
					Z=(Y==0);
					N=(Y&0b10000000)>0;
					Cycles--;
					printf("INS_LDY_ABS %X \n", Y);
				}
				break;
				case INS_CLC:  //1 Cycles
				{
					PC+=1;
					C=0;
					Cycles--;
					printf("INS_CLC  \n");
				}
				break;
				case INS_CLI:  //1 Cycles
				{
					PC+=1;
					I=0;
					Cycles--;
					printf("INS_CLI  \n");
				}
				break;
				case INS_SEC:  //1 Cycles
				{
					//PC+=1;
					C=1;
					Cycles--;
					printf("INS_SEC  \n");
				}
				break;
				case INS_SEI:  //1 Cycles
				{
					PC+=1;
					I=1;
					Cycles--;
					printf("INS_SEI  \n");
				}
				break;
				case INS_NOP:  //1 Cycles
				{
					//PC+=1;
					Cycles--;
					printf("INS_NOP  \n");
				}
				break;
				case INS_TAX:  //2 Cycles
				{
					PC+=1;
					X=A;
					Cycles--;
					Cycles--;
					printf("INS_TAX %X  \n", X);
				}
				break;
				case INS_TXA:  //2 Cycles
				{
					PC+=1;
					A=X;
					Cycles--;
					Cycles--;
					printf("INS_TXA %X \n", A);
				}
				break;
				case INS_TAY:  //2 Cycles
				{
					PC+=1;
					Y=A;
					Cycles--;
					Cycles--;
					printf("INS_TAY %X  \n", Y);
				}
				break;
				case INS_TYA:  //2 Cycles
				{
					PC+=1;
					A=Y;
					Cycles--;
					Cycles--;
					printf("INS_TYA %X \n", A);
				}
				break;
				case INS_ADC_IM:
				{
					Byte Data=FetchByte(Cycles, memory);
					Word Value=A+Data+static_cast<Byte>(C);
					printf("INS_ADC_IM %X + %X = %X \n", A, Data, Value);
					C=(Value>255);
					Z=(Value==0);
					N=(Value&0b10000000)>0;
					V=((Data&0b10000000)==(A&0b10000000) && (Value&0b10000000)!=(A&0b10000000));
					A=Value;
				}
				break;
				case INS_ADC_ABS:
				{
					Byte Data=ReadFromWord(Cycles, FetchWord(Cycles, memory), memory);
					Word Value=A+Data+static_cast<Byte>(C);
					printf("INS_ADC_ABS %X + %X = %X \n", A, Data, Value);
					C=(Value>255);
					Z=(Value==0);
					N=(Value&0b10000000)>0;
					V=((Data&0b10000000)==(A&0b10000000) && (Value&0b10000000)!=(A&0b10000000));
					A=Value;
				}
				break;
				case INS_SBC_IM:
				{
					Byte Data=FetchByte(Cycles, memory);
					Word Value=A-Data-(1-C);
					printf("INS_SBC_IM %X - %X = %X \n", A, Data, Value);
					Z=(Value==0);
					N=(Value&0b10000000)>0;
					V=((A ^ Value) & (A ^ Data) & 0x80) != 0;
					C=(A>=(Data-(1-C)));
					A=Value;
				}
				break;
				case INS_SBC_ABS:
				{
					Byte Data=ReadFromWord(Cycles, FetchWord(Cycles, memory), memory);
					Word Value=A-Data-(1-C);
					printf("INS_SBC_ABS %X - %X = %X \n", A, Data, Value);
					Z=(Value==0);
					N=(Value&0b10000000)>0;
					V=((A ^ Value) & (A ^ Data) & 0x80) != 0;
					C=(A>=(Data-(1-C)));
					A=Value;
				}
				break;
				default:
				{
					printf("Instruction not handelt 0x%X %d \n", Ins, Cycles);
				}
				break;
			}
		}
	}
};
