#include "kernel/types.h"
#include "user/user.h"

#define NUMBER_SIZE 9
#define BUFFER_SIZE (NUMBER_SIZE * 2 + 3) // space + \n + \0 from gets()

inline uint isEOL(const char c) {
	return (c == '\n' || c == '\r');
}

inline uint isDigit(const char c) {
	return (c >= '0' && c <= '9');
}

uint parseInt(char *buffer, const uint start, int *num, const uint count) {
	uint i;

	for (i = start; i < BUFFER_SIZE && i < start + NUMBER_SIZE; ++i) {
		if (!((i == start && buffer[i] == '-') || isDigit(buffer[i]))) {
			break;
		}
	}

	if (i < BUFFER_SIZE && !(buffer[start] == '-' && i == start + 1) &&
		((count == 1 && buffer[i] == ' ') || (count == 2 && isEOL(buffer[i])))) {
		buffer[i] = '\0'; // better would be to make another buffer for number and
						  // this buffer const, but I think this is ok
		if (buffer[start] == '-') {
			*num = -1 * atoi(buffer + start + 1);
		} else {
			*num = atoi(buffer + start);
		}
		return i + 1;
	} else {
		fprintf(2, "Wrong input format\n");
		exit(1);
	}
}

void parseNumbers(char *buffer, int *a, int *b) {
	uint i = parseInt(buffer, 0, a, 1);
	parseInt(buffer, i, b, 2);
}

int main(int argc, char *argv[]) {
	char buffer[BUFFER_SIZE];
	gets(buffer, BUFFER_SIZE);

	int a, b;
	parseNumbers(buffer, &a, &b);

	printf("%d\n", sum(a, b));

	exit(0);
}
