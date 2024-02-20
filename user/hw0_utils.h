#include "kernel/types.h"
#include "user/user.h"


#define BUFFER_SIZE 21 // space + \n + \0 from gets()

inline uint isEOL(const char c) {
	return (c == '\n' || c == '\r');
}

inline uint isDigit(const char c) {
	return (c >= '0' && c <= '9');
}

void errorMsg(const char *msg) {
	fprintf(2, msg);
}

void readToEnd() {
	char c;
	int	 cc;
	for (;;) {
		cc = read(0, &c, 1);
		if (cc < 1 || isEOL(c)) {
			break;
		}
	}
}
