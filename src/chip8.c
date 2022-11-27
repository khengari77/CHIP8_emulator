#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <stdint.h>
#include <errno.h>

#define log(fmt, ...)   if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__)

typedef uint8_t (*arithmetic_op)(uint8_t,uint8_t);

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
uint8_t NOP(uint8_t, uint8_t);
uint8_t RND();
void DRW(uint8_t,uint8_t,uint8_t);
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

//Operations
operation operations[16] = {opcode_0x0, 
			    opcode_0x1, 
			    opcode_0x2, 
			    opcode_0x3, 
			    opcode_0x4, 
			    opcode_0x5, 
			    opcode_0x6, 
			    opcode_0x7, 
			    opcode_0x8, 
			    opcode_0x9, 
			    opcode_0xA, 
			    opcode_0xB, 
			    opcode_0xC, 
			    opcode_0xD, 
			    opcode_0xE, 
			    opcode_0xF};
// All ALU operations. 
// Note the NOP operation represents no operation 
// which means that the index of the that operation is unused.
arithmetic_op arithmetic_ops[16] = {LD, OR, AND, XOR, ADD, SUB, SHR, SUBN, //7
				    NOP, NOP, NOP, NOP, NOP, NOP, //D
				    SHL, NOP};

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

//Keypad 16 keys
uint8_t keypad[16] = {0};

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


// helper functions
uint8_t select_4_bit(uint16_t opcode, uint8_t index){
	return (opcode & (0xF << (index)*4 )) >> (index)*4;
}

uint16_t get_opcode(){
	return (memory[PC] << 8) + memory[PC + 1] ;
}

// Initialize CHIP by loading constants (fontset -> memory)
void init_chip(){
	srand((unsigned int) time(NULL));

	//load fonts
	memcpy(memory, fontset, sizeof(fontset));
}

// load the program to memory
int load_rom(char* filename){
	FILE* file = fopen(filename, "rb");
	
	if(file == NULL) return errno;
	
	struct stat status;
	stat(filename, &status);
	size_t fsize = status.st_size;
	
	size_t bytes_read = fread(memory + 0x200, 1, sizeof(memory) - 0x200, file);
	
	// check if the whole file is read
	if(bytes_read != fsize) return -1;
	
	// release the file pointer
	fclose(file);
	return 0;
}
// emulate cycle (this could be considered the most important function)
void execute_cycle(){
	uint16_t OP = get_opcode();
	uint8_t op_index = select_4_bit(OP, 3);
	operations[op_index]();
	if(dt > 0) dt--;
	if(st > 0) st--;
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
}

void opcode_0x8(){
	uint8_t x = select_4_bit(OP, 2);
	uint8_t y = select_4_bit(OP, 1);
	uint8_t n = select_4_bit(OP, 0);
	arithmetic_ops[n](x,y);
}

void opcode_0x9(){
	uint8_t x = select_4_bit(OP, 2);
	uint8_t y = select_4_bit(OP, 1);
	SNE(V[x], V[y]);
	PC += 2;
}

void opcode_0xA(){
	I = LD(OP & 0xFFF, 0);
}

void opcode_0xB(){
	JP((OP & 0xFFF) + V[0]);
}

void opcode_0xC(){
	uint8_t x = select_4_bit(OP, 2);
	V[x] = OP & RND();
}
void opcode_0xD(){
	uint8_t x = select_4_bit(OP, 2);
	uint8_t y = select_4_bit(OP, 1);
	uint8_t n = select_4_bit(OP, 0);
    DRW(x,y,n);
}

void opcode_0xE(){
	uint8_t x = select_4_bit(OP, 2);
    	switch (OP & 0xFF){
	case 0x9E:
		SKP(x);
		break;
	case 0xA1:	
		SKNP(x);
		break;
	default:
        exit(1);
		break;
	}
}
void opcode_0xF(){
    uint8_t x = select_4_bit(OP, 2); 
    switch (OP & 0xFF)
	{
    case 0x07:
        V[x] = dt;
        break;
    case 0x0A:
        for(uint8_t i = 0; i < 16; i++) {
            if(keypad[i]) {
                V[x]  = i;
                break;
            }
        }
        break;
    case 0x15:
        dt = V[x];
        break;
    case 0x18:
        st = V[x];
        break;
    case 0x1E:
        I += V[x];
        break;
    case 0x29:
        I = V[x]*5;
        break;
    case 0x33:
        uint8_t number = V[x];
        for(uint8_t i = 2; i >= 0 ; i--){
            memory[I + i] = number % 10;
            number /= 10;
        } 
        break;
    case 0x55:
        for(uint8_t i = 0; i <= x; i++){
            memory[I + i] = V[i];
        }
        break;
    case 0x65:
        for(uint8_t i = 0; i <= x; i++){
            V[i] = memory[I + i];
        }
        break;
    }
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
void SNE(uint8_t a, uint8_t b){
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
	V[0xF] = a & 0x01;
	return a >> 1;
}
uint8_t SHL(uint8_t a, uint8_t _){
	V[0xF] = a & 0xA0;
	return a << 1;
}
uint8_t NOP(uint8_t a, uint8_t b){}

uint8_t RND(){
	return (uint8_t)(rand()/256);
}
void DRW(uint8_t x, uint8_t y, uint8_t n){ 
	// Set collision to 0
	V[0xF] = 0;
	
	uint8_t px = 0;
	for(uint8_t row = 0; row < n; row++){
		px = memory[I + row];
		for(uint8_t column = 0; column < 8; column++){
			// check if bit == 1 do some operation 
			// else goto next iteration.
			if((px & (0x80 >> column)) != 0){
				if(display[V[x] + column + ((V[y] + row) * 64)] == 1) V[0xF] = 1;
			    display[V[x] + column + ((V[y] + row) * 64)] ^= 1;
			}
		} 
	}
}
void SKP(uint8_t x){
    if(keypad[V[x]]) PC += 2;
}
void SKNP(uint8_t x){
    if(!keypad[V[x]]) PC += 2;
}
