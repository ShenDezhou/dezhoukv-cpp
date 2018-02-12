#include <cassert>
#include <cstdio>
#include "ktest.h"
#include "core/dezhoukv.h"

const int64_t thread_num = 1000;
int main(int argc, char* argv[]) {
	printf("KTEST: %s\n", argv[0]);
	int r = kirby_dotest();
	assert(r);
  init();
  
  char key[20];
  double start = get_time();
  
  for (int64_t i  =  0; i  <  thread_num; i++) {
      sprintf(key,"key:%012d", (int)i);
      set(key, "yyy");
  }  
  printf("\ndone\n");
  printf("time:%f",get_time()-start);
  //std::string value;
  //get("key:00000000001", value);
  //printf("value:%s\n", value.c_str());
  start = get_time();
  for (int64_t i  =  0; i  <  thread_num; i++) {
      sprintf(key,"key:%012d", (int)i);
      std::string value;
      get(key, value);
      printf("value:%s\n", value.c_str());
  }
  printf("time:%f",get_time()-start);  
  printf("\ndone\n");
  
	return 0;
}


