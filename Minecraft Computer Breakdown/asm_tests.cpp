
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

void test_neg_OF()
{
	int a = 0, b = 0, c = 0;
    	
    __asm volatile (
    	"movl %2, %%eax\n\t"
        "movl $0, %%edx\n\t"
    	"negl %%eax\n\t"
        "jno nof\n\t"
        "orl $0x2, %%edx\n\t"
        "nof:\n\t"
        "jnc ncf\n\t"
        "orl $0x1, %%edx\n\t"
        "ncf:\n\t"
    	"movl %%eax, %0\n\t"
        "movl %%edx, %1\n\t"
    	: "=m" (b), "=m" (c)
        : "m" (a)
    );
                                              	
	printf("a: %d, b: %d, c: %d\n", a, b, c);
}

void test_SBB()
{
	char a = 40, b = 60, c = 80, d = 0, e = 0, carry = 0, of = 0;
	
	__asm volatile (
		"movb %4, %%ah\n\t"
		"movb %5, %%al\n\t"
		"movb %6, %%dl\n\t"
		"subb %%dl, %%al\n\t"
		"sbbb $0, %%ah\n\t"
		"movb %%ah, %0\n\t"
		"movb %%al, %1\n\t"
		"jnc noc\n\t"
		"movb $1, %2\n\t"
		"noc:\n\t"
		"jno nof\n\t"
		"movb $1, %3\n\t"
		"nof:\n\t"
		: "=m" (d), "=m" (e), "=m" (carry), "=m" (of)
		: "m" (a), "m" (b), "m" (c)
	);
	
	printf("a: %d, b: %d, c: %d\n", a, b, c);
	printf("d: %d, e: %d\n", d, e);
	printf("carry: %d, of: %d\n", carry, of);
}

int main ()
{
	test_SBB();
	return 0;
}
