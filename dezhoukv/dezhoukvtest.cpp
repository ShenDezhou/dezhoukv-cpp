#include <cassert>
#include <cstdio>
#include "ktest.h"

int main(int argc, char* argv[]) {
	printf("KTEST: %s\n", argv[0]);
	int r = kirby_dotest();
	assert(r);
	return 0;
}


