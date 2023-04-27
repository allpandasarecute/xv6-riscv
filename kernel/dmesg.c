#include "types.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "param.h"
#include "fs.h"
#include "file.h"
#include "riscv.h"
#include "defs.h"
#include "memlayout.h"
#include "stdarg.h"
#include "proc.h"

#define BUF_SIZE 1000

static const char numerals[] = "0123456789abcdef";

static char buf[BUF_SIZE];
static struct spinlock msg_lock;
static const char *prefix = "[", *postfix = "] ";
static int flags[] = { 1, 1, 1, 1};
static int front = 0, top = 0, words = 0;

void msg_init() {
	initlock(&msg_lock, "msg_lock");
}

static void pushback_char(char c) {
	buf[top] = c;
	top = (top + 1) % BUF_SIZE;
	if (top == front) {
		++front;
		if (words == 0)
			return;
		while (buf[front] != '\0')
			front = (front + 1) % BUF_SIZE;
		--words;
	}
}

static void pushback_ptr(uint64 x) {
	pushback_char('0');
	pushback_char('x');

	for (int i = 0; i < (sizeof(uint64) * 2); ++i, x <<= 4)
		pushback_char(numerals[x >> (sizeof(uint64) * 8 - 4)]);
}

static void pushback_int(int x, int base, int sign) {
	char to_string_buf[16];
	int i = 0;
	uint converted;

	if (sign && (sign = x < 0))
		converted = -x;
	else
		converted = x;

	do {
		to_string_buf[i++] = numerals[converted % base];
	} while ((converted /= base) != 0);

	if (sign)
		to_string_buf[i++] = '-';

	while (--i >= 0)
		pushback_char(to_string_buf[i]);
}

void pushback_fmt_str(const char *fmt, va_list va_list) {
	int i = 0;
    char c, *str;

	while ((c = fmt[i] & 0xff) != 0) {
		if (c != '%') {
			pushback_char(c);
			continue;
		}
		c = fmt[++i] & 0xff;
		if (c == '\0')
			break;
		switch (c) {
			case 'd':
				pushback_int(va_arg(va_list, int), 10, 1);
				break;
			case 'x':
				pushback_int(va_arg(va_list, int), 16, 1);
				break;
			case 'p':
				pushback_ptr(va_arg(va_list, uint64));
				break;
			case 's':
				if ((str = va_arg(va_list, char *)) == 0)
					str = "(null)";
				for (; *str; ++str)
					pushback_char(c);
				break;
			case '%':
				pushback_char('%');
				break;
			default:
				pushback_char('%');
				pushback_char(c);
				break;
		}
		++i;
	}
	va_end(va_list);
}

void pushback_str(const char *str, int len) {
    for (int i = 0; i < len; i++) {
        buf[top++] = str[i];
        top = (top + 1) % BUF_SIZE;
    }
}

uint64 sys_dmesg(void) {
	acquire(&msg_lock);

	uint64 address;
	argaddr(0, &address);

	if (front >= top) {
        if (copyout(myproc()->pagetable, address, buf + front, sizeof(char) * (BUF_SIZE - front)) < 0) {
			release(&msg_lock);
			return -1;
		}

		if (copyout(myproc()->pagetable, address + sizeof(char) * (BUF_SIZE - front), buf, sizeof(char) * top) < 0) {
			release(&msg_lock);
			return -1;
		}

		release(&msg_lock);
		return BUF_SIZE - front + top;
	} else {
		if (copyout(myproc()->pagetable, address, buf + front, sizeof(char) * (top - front)) < 0) {
			release(&msg_lock);
			return -1;
		}

		release(&msg_lock);
		return top - front;
	}
}

void pr_msg(int t, const char *fmt, ...) {
	acquire(&msg_lock);
    if (t >= 0 && t < 4 && !flags[t]) {
        release(&msg_lock);
        return;
    }
	acquire(&tickslock);

	int cur_ticks = ticks;
	release(&tickslock);

	va_list va_list;
	va_start(va_list, fmt);

	pushback_fmt_str(prefix, va_list);
	pushback_int(cur_ticks, 10, 0);
	pushback_fmt_str(postfix, va_list);
	pushback_fmt_str(fmt, va_list);

	va_end(va_list);

	pushback_char('\0');
	++words;

	release(&msg_lock);
}

uint64 sys_flags(void) {
    acquire(&msg_lock);
    argint(0, &flags[1]);
    argint(1, &flags[2]);
    argint(2, &flags[3]);
    release(&msg_lock);
    return 0;
}