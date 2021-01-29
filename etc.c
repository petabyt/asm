// Some other random "etc" functions
#include <stdbool.h>
#include <string.h>
#include "header.h"

bool isChar(char a) {
	return ((a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z') || a == '_');
}

bool isDec(char a) {
	return (a >= '0' && a <= '9');
}

int getTextType(char type[]) {
	for (size_t i = 0; i < TEXTLEN; i++) {
		if (!strcmp(type, text[i].name)) {
			return text[i].id;
		}
	}

	return -1;
}
