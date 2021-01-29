#include <stdlib.h>
#include <stdbool.h>

#define MAX_LINE 100

enum Tokens {
	TEXT, ADDRESS,
	INTEGER, LABEL
};

enum Instructions {
	MOVB, // Move byte
	MOVS, // Move short
	MOVW, // Move word
	MOVL, // Move long
	JMP,
	LOAD,
	STORE,
	BYTE,
	WORD,
	HEADER,
	FOOTER
};

struct Token {
	int type;
	int first;
	int second;
	
	char text[20];
	size_t length;
};

struct Tree {
	struct Token tokens[5];
	size_t length;
};

bool isChar(char a);
bool isDec(char a);
int getTextType(char type[]);

static struct Text {
	char *name;
	int id;
}text[] = {
	// Multiply value for
	// the instructions
	// (make sure to do these first)
	{"r0", 0},
	{"r1", 1},
	{"r2", 2},
	{"r3", 3},
	{"r4", 4},
	{"r5", 5},
	{"r6", 6},
	{"r7", 7},
	{"r8", 8},
	{"r9", 9},
	{"r10", 10},
	{"r11", 11},

	{"movb", MOVB},
	{"movs", MOVS},
	{"movw", MOVW},
	{"movl", MOVL},
	{"head", HEADER},
	{"foot", FOOTER},
	{"jmp", JMP},
	{"load", LOAD},
	{"byte", BYTE},
	{"word", WORD},
	{"str", STORE}
};

#define TEXTLEN (int)(sizeof(text) / sizeof(text[0]))

// ARM Boot header
// Allocate 4k for stack, cpu defs, etc
static unsigned char header[] = {0xFF, 0xFF, 0xFF, 0xEA, 0xFA, 0xDE, 0xA0, 0xE3, 0xFF, 0xFF, 0xFF, 0xEB};
#define HEADERLEN (int)(sizeof(header))

// halt
static unsigned char footer[] = {0x20, 0x00, 0x01, 0x00};
#define FOOTERLEN (int)(sizeof(footer))
