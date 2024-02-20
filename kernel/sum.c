#include "types.h"
#include "riscv.h"
#include "defs.h"

uint64 sys_sum(void) {
	int a, b;

	argint(0, &a);
	argint(1, &b);

	return a + b;
}
