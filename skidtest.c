/*
 * skidtest
 *
 * COMPILE: gcc -O0 -o skidtest skidtest.c
 *
 * USAGE: ./skidtest size_kb
 *    eg,
 *        perf record -vv -e r412e -c 1000 ./skidtest 1000000	# sample every 1000 LLC-miss
 *
 * Check the verbose output (-vv) to understand if precise_ip (PEBS) was auto-enabled or not.
 * Choose a size greater than the LLC cache to induce misses.
 *
 * For more instructions, see: https://github.com/brendangregg/skid-testing
 *
 * 23-Mar-2017	Brendan Gregg	Created this.
 */
 
#include <stdio.h>
#include <stdlib.h>

void
memreader(char *p, unsigned long long j) {
	char c;
	c = p[j];
}

#define NOP10	"nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
#define NOP100	NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10
#define NOP1000	NOP100 NOP100 NOP100 NOP100 NOP100 NOP100 NOP100 NOP100 NOP100 NOP100

void
noprunway() {
	/*
	 * A nop runway that is 5000 nops long.
	 * The aim is to span 1000 cycles on a 5-wide.
	 * Reduce to keep within one page if desired.
	 */
	asm(
		NOP1000
		NOP1000
		NOP1000
		NOP1000
		NOP1000
	);
}

int
main(int argc, char *argv[])
{
	unsigned long long size, j;
	char *p, c;

	if (argc != 2) {
		printf("USAGE: memstride size_KB\n");
		exit(1);
	}	

	size = atoi(argv[1]) * 1024ULL;

	if ((p = malloc(size)) == NULL) {
		printf("ERROR: malloc failed\n");
		exit(1);
	}

	printf("Populate...\n");
	for (j = 0; j < size; j += 32) {
		p[j] = 'a';
	}

	printf("Stride...\n");
	for (;;) {
		// 1 Kbyte stride, to walk past pages quickly
		for (j = 0ULL; j < size; j += 1024) {
			memreader(p, j);
			noprunway();
		}
	}

	return (0);
}
