#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "header.h"

struct Tree tree;

struct Runtime {
	struct Labels {
		char name[50];
		size_t location;
	}labels[20];
	size_t labelsLength;
}runtime;

int getLabel(char *name) {
	for (int i = 0; i < (int)runtime.labelsLength; i++) {
		if (!strcmp(name, runtime.labels[i].name)) {
			return i;
		}
	}

	return -1;
}

// ARM immediate value encoding
// Notes:
// https://stackoverflow.com/q/17763582
// https://alisdair.mcdiarmid.org/arm-immediate-value-encoding/
void rotateBits(int *rotate, unsigned int *number) {
	for (*rotate = 0; *rotate < 32; (*rotate)++) {
		// return if significant bits fit into 8 bit
		// immediate.
		if (!(*number & ~0xff)) {
			return;
		}

		// Rotate left by two
		*number = (*number << 2) | (*number >> 30);
	}
}

// return -1 for nothing line
// return 0 for normal parse
int lex(char *buffer) {
	tree.length = 0;
	for (size_t c = 0; buffer[c] != '\0';) {
		while (buffer[c] == ' ' || buffer[c] == '\t' || buffer[c] == '\n') {
			c++;
		}

		if (buffer[c] == ';') {
			while (buffer[c] != '\0') {
				c++;
			}
		}

		if (tree.length == 0 && buffer[c] == '\0') {
			return -1;
		}

		// Make a pointer to decrease repetition
		struct Token *current = &tree.tokens[tree.length];
		
		current->length = 0;
		current->first = 0;
		current->second = 0;
		
		if (isChar(buffer[c])) {
			current->type = TEXT;
			while (isChar(buffer[c]) || isDec(buffer[c])) {
				current->text[current->length] = buffer[c];
				current->length++;
				c++;
			}

			if (buffer[c] == ':') {
				current->type = LABEL;
				c++;
			}
		}

		while (isDec(buffer[c])) {
			current->type = INTEGER;
			current->first *= 10;
			current->first += buffer[c] - '0';
			c++;
		}

		if (buffer[c] == '\'') {
			c++;
			current->type = INTEGER;
			current->first = buffer[c];
			c++;
		}

		if (buffer[c] == '[') {
			c++;
			current->type = ADDRESS;
			while (buffer[c] != ']') {
				current->text[current->length] = buffer[c];
				current->length++;
				c++;
			}

			c++;
		}

		current->text[current->length] = '\0';
		tree.length++;
	}

	return 0;
}

int getInsLength() {
	int insType = getTextType(tree.tokens[0].text);
	switch (insType) {
	case JMP:
	case MOVB:
	case BYTE:
		return 1;
	case WORD:
		return 4;
	case HEADER:
		return HEADERLEN;
	case FOOTER:
		return FOOTERLEN;
	}

	return 0;
}

// Spit out byte
void out(unsigned char a) {
	printf("%c", a);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		puts("No arg");
		return 0;
	}
	
	char buffer[MAX_LINE];
	FILE *reader = fopen(argv[1], "r");
	if (reader == NULL) {
		puts("Bad file");
		return -1;
	}
	
	size_t currentLength = 0;

	// Pre Assemble lex for label locations
	while (fgets(buffer, MAX_LINE, reader) != NULL) {
		if (lex(buffer)) {
			continue;
		}

		if (tree.tokens[0].type == LABEL) {
			strcpy(runtime.labels[runtime.labelsLength].name, tree.tokens[0].text);
			runtime.labels[runtime.labelsLength].location = currentLength;
			runtime.labelsLength++;
			continue;
		}

		currentLength += getInsLength();
	}
	
	fclose(reader);
	currentLength = 0;

	// Reopen the pointer 
	reader = fopen(argv[1], "r");

	// Lex line by line
	while (fgets(buffer, MAX_LINE, reader) != NULL) {
		if (lex(buffer)) {
			continue;
		}

		if (tree.tokens[0].type == LABEL) {
			continue;
		}

		currentLength += getInsLength();
		
		int insType = getTextType(tree.tokens[0].text);
		if (insType == HEADER) {
			for (int i = 0; i < HEADERLEN; i++) {
				out(header[i]);
			}
		} else if (insType == FOOTER) {
			for (int i = 0; i < FOOTERLEN; i++) {
				out(footer[i]);
			}
		} else if (insType == BYTE) {
			out(tree.tokens[1].first);
		} else if (insType == WORD) {
			out((char)tree.tokens[1].first);
			out(tree.tokens[1].first >> 8);
			out(tree.tokens[1].first >> 16);
			out(tree.tokens[1].first >> 24);
		} else {
			unsigned char output[4] = {0};

			if (insType == STORE) {
				int regType = getTextType(tree.tokens[1].text);
				int secondRegType = getTextType(tree.tokens[2].text);
				output[1] = regType * 0x10;
				output[2] = 0x80 + secondRegType;
				output[3] = 0xE5;
			} else if (insType == MOVB) {
				int regType = getTextType(tree.tokens[1].text);
				int rotate;
				unsigned int number = tree.tokens[2].first;
				rotateBits(&rotate, &number);

				output[0] = number;
				output[1] = rotate;
				output[2] = 0xA0;
				output[3] = 0xE3;

				// Add label
				output[1] += 0x10 * regType;
			} else if (insType == JMP) {
				int label = getLabel(tree.tokens[1].text);

				output[0] = 255;
				output[1] = 255;
				output[2] = 255;
				output[3] = 0xEA; // branch optcode

				// Hacky label insertion
				if (runtime.labels[label].location < currentLength) {
					output[0] -= (currentLength - runtime.labels[label].location) / 4;
				} else {
					output[0] += (runtime.labels[label].location - currentLength) / 4;
					output[1] = 0;
					output[2] = 0;
				}
			} else if (insType == LOAD) {
				int regType = getTextType(tree.tokens[1].text);

				// More hacky 
				if (tree.tokens[2].type == ADDRESS) {
					int secondRegType = getTextType(tree.tokens[1].text);
					output[2] = 0x90 + secondRegType;
					output[3] = 0xE5; 
				} else {
					// Just use a move instruction here
					int label = getLabel(tree.tokens[2].text);
					output[0] = runtime.labels[label].location / 4;
					output[1] = 0x10 * regType;
					output[2] = 0xA0;
					output[3] = 0xEA; // Mov byte optcode
				}
			}

			out(output[0]);
			out(output[1]);
			out(output[2]);
			out(output[3]);
		}
	}

	fclose(reader);
	return 0;
}


/*
ldr r0, =UART0 (32)
14 00 9F E5

unsigned char instructions[] = {0xFF, 0xFF, 0xFF, 0xEA, 0xFA, 0xDE, 0xA0, 0xE3, 0xFF, 0xFF, 0xFF, 0xEB, 0x41, 0x30, 0xA0, 0xE3, 0x0C, 0x20, 0x9F, 0xE5, 0x00, 0x20, 0x92, 0xE5, 0x00, 0x30, 0x82, 0xE5, 0xFE, 0xFF, 0xFF, 0xEA, 0x00, 0x10, 0x1F, 0x10, 0x20, 0x00, 0x01, 0x00};

mov r0, #144
144   0   160
          176
          
44 1 == 300
(1 << 8) | 44


*/
