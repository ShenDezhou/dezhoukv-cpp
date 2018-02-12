#include <cassert>
#include <cstdio>
#include "ktest.h"
#include "core/dezhoukv.h"

int main(int argc, char* argv[]) {
	printf("KTEST: %s\n", argv[0]);
	int r = kirby_dotest();
	assert(r);
  init();
  set("key:00000000001", "yyy");
  std::string value;
  get("key:00000000001", value);
  printf("value:%s\n", value.c_str());
	return 0;
}


