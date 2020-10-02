
// gcc options: -m32

#include <stdio.h>

void test_AAA()
{
	int a = 0, b = 0;
__asm__ volatile (
	"movw $5, %%ax;"
	"addw $9, %%ax;"
	"aaa;"
	"movzx %%al, %0;"
	"movzx %%ah, %1;"
	: "=r" (a), "=r" (b)
	);

    printf("a=%d, b=%d\n", a, b);
}

void test_cwd()
{
    int a = 10, b, c;
    __asm volatile (
        "movl %2, %%eax;"
        "movl $0, %%edx;"
        "notl %%edx;"
        "movl %%edx, %1;"
        "cwd;"
        "movl %%edx, %0;"
        : "=r" (b), "=m" (c)
        : "rm" (a)
    );

    printf("a: %d, b: %d, c: %d\n", a, b, c);
}

void test_daa()
{
	unsigned char a = 0b10011001, b = 0b10011001, c = 0, d = 0;

	__asm volatile (
		"movb %2, %%al;\n\t"
		"movb %3, %%dl;\n\t"
		"addb %%dl, %%al;\n\t"
		"movb %%al, %0;\n\t"
		"daa;\n\t"
		"movb %%al, %1;\n\t"
		: "=m" (c), "=m" (d)
		: "rm" (a), "rm" (b)
	);
	
	printf("a: %x, b: %x, c: %x, d: %x\n", a, b, c, d);
	printf("raw: %x, expected: %d\n", (a + b), (99 + 99) % 100);
	// a: 99, b: 99, c: 32, d: 98
}

void test_bound()
{
	int bounds = 0x00100005; // min: 5, max: 10 (les deux sont inclus)
	short index = 7;
	
	__asm volatile (
		"boundw %1, %0;"
		:
		: "m" (bounds), "r" (index)
	);
}

int main ()
{
	test_AAA();
	return 0;
}
