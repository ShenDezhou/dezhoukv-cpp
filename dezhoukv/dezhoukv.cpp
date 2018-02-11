#include "stdafx.h"
#include "config-dummy.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "murmurhash3/MurmurHash3.h"
#include <boost/bloom_filter/basic_bloom_filter.hpp>
#include <boost/bloom_filter/counting_bloom_filter.hpp>
#include <boost/bloom_filter/dynamic_counting_bloom_filter.hpp>
#include "maglev/include/Lanton.h"
#include "hyperloglog/hyperloglog.h"
#include "redisio/redis_client.h"
#include <algorithm>
#define FULL_VERSION PACKAGE_VERSION "-" SVNVERSION

const int64_t BASE   = 15000000000llu;
const int64_t OFFSET = -7500000000ll;
const int64_t A1 = -10000000000ll;
const int64_t B1 =   1000000000ll;
const int64_t C1 =   5000000000ll;
const char* mem_ip = "39.104.58.183";
const char* ssd_ip = "10.134.110.46";
const char* hdd_ip = "39.104.63.175";
const double D1 = 0.03378;
const double E1 = 0.1009;
const double F1 = 1.0;

int foo()
{
	return 1;
}

/**
 * 转换 a.b.c.d 形式的版本号为32bit整数 0xaabbccdd
 */
uint32_t CONVERT_VERSION(const char* text_version) 
{
	uint32_t ret = 0;
	unsigned long v;
	const char* p = text_version;
	char* q = NULL;
	v = strtoul(p, &q, 16);
	ret |= (v & 0xfful) << 24;
	if (!*q) return ret;
	v = strtoul(q+1, &q, 16);
	ret |= (v & 0xfful) << 16;
	if (!*q) return ret;
	v = strtoul(q+1, &q, 16);
	ret |= (v & 0xfful) << 8;
	if (!*q) return ret;
	v = strtoul(q+1, &q, 16);
	ret |= (v & 0xfful);
	return ret;
}

/**
 * 转换 a.b.c.d-svn 形式的版本号为64bit整数 0xaabbccddssssssmm
 * s表示svn版本编号，m表示是否是一个干净的版本
 */
uint64_t CONVERT_FULL_VERSION(const char* text_svnversion) 
{
	const char* text_svn = strrchr(text_svnversion, '-');
	char* text_prefix;
	char* q;
	uint64_t ret;
	if (text_svn == NULL) {
		return CONVERT_VERSION(text_svnversion);
	}
	text_prefix = (char*)alloca(text_svn - text_svnversion + 1);
	memcpy(text_prefix, text_svnversion, text_svn - text_svnversion);
	text_prefix[text_svn - text_svnversion] = 0;
	ret = ((uint64_t)CONVERT_VERSION(text_prefix)) << 32ull;
	++text_svn;
	ret |= (strtoul(text_svn, &q, 16) & 0xffffff) << 8;
	if (*q)
		ret |= 0x0f;
	return ret;
}
RedisAdapter* *redis_mem; // redis lists
RedisAdapter* *redis_ssd; // redis lists
RedisAdapter* *redis_hdd; // redis lists
boost::bloom_filters::dynamic_counting_bloom_filter<int64_t, 2 > bloom; //g_bloom
Lanton_t* *lanton; //maglevs

HyperLogLog hyperll(4ll);

int init_mem() {
	printf("Hello %s %s-%s, %08x, %016llx\n", PACKAGE_NAME, PACKAGE_VERSION, SVNVERSION, CONVERT_VERSION(PACKAGE_VERSION), CONVERT_FULL_VERSION(FULL_VERSION));
  lanton = new Lanton_t*[3];
  
  int64_t backends[1];
	int64_t thread_num=1;
	redis_mem = new RedisAdapter*[1];
	
  for(int64_t i = 0; i < thread_num; i++)
	{
		backends[i] = i;
		redis_mem[i] = new RedisAdapter(mem_ip, 6379 + i);
		int b = redis_mem[i] -> Open();
		printf("port %d, open %d", 6379+i, b);
		backends[i] = (int64_t)redis_mem[i];
	}
 lanton[0] = new_lanton(backends, thread_num, DEFAULT_LOOKUPTABLE_SIZE); // MEM
  
  //for(int64_t i = 0; i < thread_num; i++)
  //{
  //   add_backend(lanton[0], backends[i]);
  //}
  
}

int init_ssd() {
	printf("Hello %s %s-%s, %08x, %016llx\n", PACKAGE_NAME, PACKAGE_VERSION, SVNVERSION, CONVERT_VERSION(PACKAGE_VERSION), CONVERT_FULL_VERSION(FULL_VERSION));
  
  
  int64_t backends[1];
	int64_t thread_num=1;
	redis_ssd = new RedisAdapter*[1];
	for(int64_t i = 0; i < 1; i++)
	{
		backends[i] = i;
		redis_ssd[i] = new RedisAdapter(ssd_ip, 8323);
		int b = redis_ssd[i] -> Open();
		printf("port %d, open %d", 8323, b);
		backends[i] = (int64_t)redis_ssd[i];
	}
 lanton[1] = new_lanton(backends, thread_num, DEFAULT_LOOKUPTABLE_SIZE); // SSD
  
  	//for(int64_t i = 0; i < thread_num; i++)
  //{
  //   add_backend(lanton[1], backends[i]);
  //}
  
}
 
int init_hdd() {
	printf("Hello %s %s-%s, %08x, %016llx\n", PACKAGE_NAME, PACKAGE_VERSION, SVNVERSION, CONVERT_VERSION(PACKAGE_VERSION), CONVERT_FULL_VERSION(FULL_VERSION));
  
  int64_t backends[100];
	int64_t thread_num=100;
	redis_hdd = new RedisAdapter*[100];
	for(int64_t i = 0; i < thread_num; i++)
	{
		backends[i] = i;
		redis_hdd[i] = new RedisAdapter(hdd_ip, 10100 + i);
		int b = redis_hdd[i] -> Open();
		printf("port %d, open %d", 10100+i, b);
		backends[i] = (int64_t)redis_hdd[i];
	}
   lanton[2] = new_lanton(backends, thread_num, DEFAULT_LOOKUPTABLE_SIZE); // HDD
  //for(int64_t i = 0; i < thread_num; i++)
  //{
  //   add_backend(lanton[2], backends[i]);
  //}
}

//"key:000000059611"
int get(const char* key, std::string &value)
{
  // 0. calc key mm3 hash
	char out[17];
	MurmurHash3_x64_128(key,strlen(key),0,out);
	int64_t hash =((int64_t*)out)[0] ^ ((int64_t*)out)[1];
	int64_t result = hash % BASE + OFFSET;
	for(int i=0; i<1 ; i++) {
		printf("%016llx\t",result);
	}
  // 1. calc z
  char z;
	if ( A1 < result && result <=0) {
		printf("i\n");
    z='i';
	} 
	if ( 0 < result && result <= B1) {
		printf("j\n");
    z='j';
	}
	if ( B1 < result && result <= C1) {
		printf("k\n");
    z='k';
	}
  // 2. check g bloom
  if (bloom.probably_contains(result)) {
    // 2. insert into bloom
  	//for (size_t i = 0; i < 1; ++i) {
    //  		bloom.insert(result);
  	//}
   
  	//for (size_t i = 0; i < 1; ++i) {
    //    bloom.remove(result);
    //}
  	//int collisions = 0;
  	//for (size_t i = 0; i < 1; ++i) {
  	//    if (bloom.probably_contains(result)) ++collisions;
    //	}
    // 2. print bloom false positive rate
  	for (size_t i = 0; i < 1; ++i) {
    		printf("false positive rate:%.4f\n",bloom.false_positive_rate());
  	}
  	//printf("contains:%d\n", collisions);
  	//printf("\n");
  
  
    //printf("\n");
    int choose = z-'i';
  	// 3. check machine
  	//int64_t endpoint = lookup_backend(lanton[choose], (void*)(&result), sizeof(result));
	  // 4. print which endpoint
    //printf("Endpoint :%lld\n", endpoint);
  	
    
  	//for (size_t i = 0; i < 1; ++i) {
    //   hyperll.update(result);
    //}
    // 5. get current distinct count
    double current_count = hyperll.raw_estimate();
  	printf("estimate distinct key:%.1d\n", current_count);
    if ( hyperll.raw_estimate() < D1 * BASE ) {
      printf("max_possible requests:1\n");
      // 3. check machine
    	  int64_t endpoint = lookup_backend(lanton[choose], (void*)(&result), sizeof(result));
  	    // 4. print which endpoint
        printf("Endpoint :%lld\n", endpoint);
        int has_result = ((RedisAdapter*)endpoint)->Get(key,value);
        // 7.print values
        printf("Got:%s",value.c_str());
        if (has_result)
          return has_result;
    }
    if ( hyperll.raw_estimate() > D1 * BASE && hyperll.raw_estimate() < E1 * BASE ) {
      printf("max_possible requests:2\n");
      for (size_t i = 0; i < 2; ++i) {
        // 3. check machine
    	  int64_t endpoint = lookup_backend(lanton[choose], (void*)(&result), sizeof(result));
  	    // 4. print which endpoint
        printf("Endpoint :%lld\n", endpoint);
        int has_result = ((RedisAdapter*)endpoint)->Get(key,value);
        // 7.print values
        printf("Got:%s",value.c_str());
        if (has_result)
          return has_result;
        if( (char)(z+i)<'k' )
          choose++;
        else
          break;
      }
    
    }
    if ( hyperll.raw_estimate() > E1 * BASE && hyperll.raw_estimate() < F1 * BASE ) {
      printf("max_possible requests:3\n");
      for (size_t i = 0; i < 3; ++i) {
        // 3. check machine
    	  int64_t endpoint = lookup_backend(lanton[choose], (void*)(&result), sizeof(result));
  	    // 4. print which endpoint
        printf("Endpoint :%lld\n", endpoint);
        int has_result = ((RedisAdapter*)endpoint)->Get(key,value);
        // 7.print values
        printf("Got:%s",value.c_str());
        if (has_result)
          return has_result;
        if( (char)(z+i)<'k' )
          choose++;
        else
          break;
      }
    }
    
    // 6.get values
    //std::string value;
    //int has_result = ((RedisAdapter*)endpoint)->Get(key,value);
    // 7.print values
    //printf("Got:%s",value.c_str());
    //return has_result;
  }
  return 0;
}
//"key:000000059611"
int set(const char* key, std::string value)
{
  // 0. calc key mm3 hash
	char out[17];
	MurmurHash3_x64_128(key,strlen(key),0,out);
	int64_t hash =((int64_t*)out)[0] ^ ((int64_t*)out)[1];
	int64_t result = hash % BASE + OFFSET;
	for(int i=0; i<1 ; i++) {
		printf("%016llx\t",result);
	}
  // 1. calc z
  char z;
	if ( A1 < result && result <=0) {
		printf("i\n");
    z='i';
	} 
	if ( 0 < result && result <= B1) {
		printf("j\n");
    z='j';
	}
	if ( B1 < result && result <= C1) {
		printf("k\n");
    z='k';
	}
  //if (bloom.probably_contains(result)) {
  //  printf("contains:%d\n", collisions);
  //} 
  // 2. insert bloom
  bloom.insert(result);
  // 2. check g bloom
  //if (bloom.probably_contains(result)) {
    // 2. insert into bloom
  	//for (size_t i = 0; i < 1; ++i) {
    //  		bloom.insert(result);
  	//}
   
  	//for (size_t i = 0; i < 1; ++i) {
    //    bloom.remove(result);
    //}
  	//int collisions = 0;
  	//for (size_t i = 0; i < 1; ++i) {
  	//    if (bloom.probably_contains(result)) ++collisions;
    //	}
    // 2. print bloom false positive rate
  	for (size_t i = 0; i < 1; ++i) {
    		printf("false positive rate:%.4f\n",bloom.false_positive_rate());
  	}
  	//printf("contains:%d\n", collisions);
  	//printf("\n");
    
    //printf("\n");
  	//for (size_t i = 0; i < 1; ++i) {
    //   hyperll.update(result);
    //}
    // 3. get current distinct count
    double current_count = hyperll.raw_estimate();
  	printf("estimate distinct key:%.1d\n", current_count);
    int choose;
    if ( hyperll.raw_estimate() < D1 * BASE ) {
      choose = 0;
      //printf("max_possible requests:1\n");
    }
    else if ( hyperll.raw_estimate() > D1 * BASE && hyperll.raw_estimate() < E1 * BASE
      && z=='i' || z=='j'
    ) {
       choose = 1;
      //printf("max_possible requests:2\n");
    }
    else if ( hyperll.raw_estimate() > E1 * BASE && hyperll.raw_estimate() < F1 * BASE && z=='i' ) {
       //printf("max_possible requests:3\n");
       choose = 2;
    } else {
       choose = -1;    
    }
    // 4.choose storage device (i,j,k)
    char z_;
    if ( z+choose >='i' && z+choose <='k') {
      z_ = (char)(z+choose);
    }
    if (choose == -1) {
      z_ = 'k';
    }
    choose = z_-'i';
    // 5. check machine
  	int64_t endpoint = lookup_backend(lanton[choose], (void*)(&result), sizeof(result));
	  // 6. print which endpoint
    printf("Endpoint :%lld\n", endpoint);
    // 7.set values
    //std::string value;
    int has_result = ((RedisAdapter*)endpoint)->Set(key,value);
    // 8.print values
    printf("Set:%c-%d", z, has_result);
    return has_result;
  //}
  return 0;
}

int main()
{
  init_mem();
  init_ssd();
  init_hdd();
  set("key:00000000001","xxx");
  std::string value;
  get("key:00000000001",value);
  printf("value:%s\n",value.c_str());
}
