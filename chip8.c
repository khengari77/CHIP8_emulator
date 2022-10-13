#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <stdint.h>


#define log(fmt, ...)   if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__)

typedef void (*arithmetic_op)(uint8_t,uint8_t);

typedef void (*operation)();

// Declare operational functions
void CLS();
void RET();
void JP(uint16_t);
void CALL(uint16_t);
void SE(uint8_t, uint8_t);
void SNE(uint8_t, uint8_t);
uint8_t LD(uint8_t,uint8_t);
uint8_t ADD(uint8_t,uint8_t);
uint8_t OR(uint8_t,uint8_t);
uint8_t AND(uint8_t,uint8_t);
uint8_t XOR(uint8_t,uint8_t);
uint8_t SUB(uint8_t,uint8_t);
uint8_t SUBN(uint8_t,uint8_t);
uint8_t SHR(uint8_t, uint8_t);
uint8_t SHL(uint8_t, uint8_t);
uint8_t RND();
void DRW(uint8_t,uint8_t);
void SKP(uint8_t);
void SKNP(uint8_t);
// end

// Opcodes
void opcode_0x0();
void opcode_0x1();
void opcode_0x2();
void opcode_0x3();
void opcode_0x4();
void opcode_0x5();
void opcode_0x6();
void opcode_0x7();
void opcode_0x8();
void opcode_0x9();
void opcode_0xA();
void opcode_0xB();
void opcode_0xC();
void opcode_0xD();
void opcode_0xE();
void opcode_0xF();
//end

// debug flag 
bool  DEBUG = true;

// A memory with 4096 bytes.
uint8_t memory[0x1000] = {0};

// 16 8-bit general purpose registers.
uint8_t V[0x10] = {0};

// the I register
uint16_t I = 0;

// delay time special register.
uint8_t dt = 0;

// sound time special register.
uint8_t st = 0;

// Program counter.
uint16_t PC = 0x200;

// Stack pointer.
uint8_t SP = 0;

// The Stack.
uint16_t stack[0x10] = {0};

// The fonstet that is used by the CHIP8
uint8_t fontset[80] = {
	0xF0,0x90,0x90,0x90,0xF0,  //0
	0x20,0x60,0x20,0x20,0x70,  //1
	0xF0,0x10,0xF0,0x80,0xF0,  //2
	0xF0,0x10,0xF0,0x10,0xF0,  //3
	0x90,0x90,0xF0,0x10,0x10,  //4
	0xF0,0x80,0xF0,0x10,0xF0,  //5
	0xF0,0x80,0xF0,0x90,0xF0,  //6
	0xF0,0x10,0x20,0x40,0x40,  //7
	0xF0,0x90,0xF0,0x90,0xF0,  //8
	0xF0,0x90,0xF0,0x10,0xF0,  //9
	0xF0,0x90,0xF0,0x90,0x90,  //A
	0xE0,0x90,0xE0,0x90,0xE0,  //B
	0xF0,0x80,0x80,0x80,0xF0,  //C
	0xE0,0x90,0x90,0x90,0xE0,  //D
	0xF0,0x80,0xF0,0x80,0xF0,  //E
	0xF0,0x80,0xF0,0x80,0x80   //F
};

// The Display: a 64 x 32 px display.
uint8_t display[0x800] = {0};

// Current operation
uint16_t OP;

uint8_t draw_flag = 0;

// helper functions
uint8_t select_4_bit(uint16_t opcode, uint8_t index){
	return (opcode & (0xF << (index)*4 )) >> (index)*4;
}

uint16_t get_opcode(){
	return (memory[PC] << 8) + memory[PC + 1] ;
}

void init_chip(){
	srand((unsigned int) time(NULL));

	//load fonts
	memcpy(memory, fontset, sizeof(fontset));
}

void opcode_0x0(){
	switch(OP & 0xFF){
	case 0xE0:
		CLS();
		break;
	case 0xEE:
		RET();
		break;
	default:
		break;
	}
	PC += 2;
}	
void opcode_0x1(){
	JP(OP & 0xFFF);
}

void opcode_0x2(){
	CALL(OP & 0xFFF);
}

void opcode_0x3(){
	SE(V[select_4_bit(OP, 2)], OP & 0xFF);
	PC += 2;
}
void opcode_0x4(){
	SNE(V[select_4_bit(OP, 2)], OP & 0xFF);
	PC += 2;
}

void opcode_0x5(){
	uint8_t x = select_4_bit(OP, 2);
	uint8_t y = select_4_bit(OP, 1);
	SE(V[x], V[y]);
	PC += 2;
}

void opcode_0x6(){
	uint8_t x = select_4_bit(OP, 2);
	V[x] = LD(OP & 0xFF, 0);
}

void opcode_0x7(){
	uint8_t x = select_4_bit(OP, 2);
	V[x] = ADD(V[x], OP & 0xFF);
	PC += 2;
}

void opcode_0x8(){}

void opcode_0x9(){
	uint8_t x = select_4_bit(OP, 2);
	uint8_t y = select_4_bit(OP, 1);
	SNE(V[x], V[y]);
	PC += 2;
}

void opcode_0xA(){
	I = LD(op & 0xFFF, 0);
	PC += 2;
}

void opcode_0xB(){
	JP((OP & 0xFFF) + V[0]);
}

void opcode_0xC(){
	uint8_t x = select_4_bit(OP, 2);
	V[x] = OP & RND();
	PC += 2;
}
void opcode_0xD(){
	//TODO
}

void opcode_0xE(){
	switch (OP & 0xFF)
	{
	case 0x9E:
		//TODO
		break;
	case 0xA1:
		//TODO
		break;
	default:
		break;
	}
}
void opcode_0xF(){

}

int main(){
	uint16_t a = 0xABCD;
	uint8_t b = select_4_bit(a, 2);
	printf("%X\n", b);
	log("[2] 0x%X -> 0x%X\n", a,b);
}

// Operations

void CLS(){
	memset(display, 0, sizeof(display));
}

void RET(){
	PC = stack[SP];
	SP--;
}

void JP(uint16_t addr){
	PC = addr;
}
void CALL(uint16_t addr){
	SP++;
	stack[SP] = PC;
	JP(addr);
}
void SE(uint8_t a, uint8_t b){
	if (a == b) PC += 2;
}
void SE(uint8_t a, uint8_t b){
	if (a != b) PC += 2;
}
uint8_t LD(uint8_t a, uint8_t _){
	return a;
}
uint8_t ADD(uint8_t a, uint8_t b){
	uint16_t out = a + b;
	if (0xFF00 & out){
		V[0xF] = 1;
		out &= 0xFF;
	}
	return (uint8_t) out;
}
uint8_t OR(uint8_t a, uint8_t b){
	return a | b;
}
uint8_t AND(uint8_t a, uint8_t b){
	return a & b;
}
uint8_t XOR(uint8_t a, uint8_t b){
	return a ^ b;
}
uint8_t SUB(uint8_t a, uint8_t b){
	V[0xF] = a > b; 
	return a - b;
}
uint8_t SUBN(uint8_t a, uint8_t b){
	V[0xF] = b > a;
	return b - a;
}
uint8_t SHR(uint8_t a, uint8_t _){
	V[0xF] = a & 0x0001;
	return a >> 1;
}
uint8_t SHL(uint8_t a, uint8_t _){
	V[0xF] = a & 0xA000;
	return a << 1;
}
uint8_t RND(){
	return (uint8_t)(rand()/256);
}
void DRW(uint8_t,uint8_t);
void SKP(uint8_t);
void SKNP(uint8_t);