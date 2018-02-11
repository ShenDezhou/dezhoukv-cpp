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


int main()
{
	printf("Hello %s %s-%s, %08x, %016llx\n", PACKAGE_NAME, PACKAGE_VERSION, SVNVERSION, CONVERT_VERSION(PACKAGE_VERSION), CONVERT_FULL_VERSION(FULL_VERSION));
	char out[17];
	MurmurHash3_x64_128("key:001",strlen("key:001"),0,out);
	int64_t hash =((int64_t*)out)[0] ^ ((int64_t*)out)[1];
	const int64_t BASE   = 15000000000llu;
	const int64_t OFFSET = -7500000000ll;
	int64_t result = hash % BASE + OFFSET;
	const int64_t A1 = -10000000000ll;
	const int64_t B1 =   1000000000ll;
	const int64_t C1 =   5000000000ll;
	const char* hdd_ip = "39.104.63.175";
	for(int i=0; i<1 ; i++) {
		printf("%016llx\t",result);
	}
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
	boost::bloom_filters::dynamic_counting_bloom_filter<int64_t, 2 > bloom;
	for (size_t i = 0; i < 1; ++i) {
    		bloom.insert(result);
	}
	for (size_t i = 0; i < 1; ++i) {
                bloom.remove(result);
        }
	int collisions = 0;
	for (size_t i = 0; i < 1; ++i) {
	    if (bloom.probably_contains(result)) ++collisions;
  	}
	for (size_t i = 0; i < 1; ++i) {
  		printf("false positive rate:%.4f\n",bloom.false_positive_rate());
	}
	printf("contains:%d\n", collisions);
	printf("\n");

	int64_t backends[100];
	int64_t thread_num=100;
	RedisAdapter* *redis_ = new RedisAdapter*[100];
	for(int64_t i = 0; i < thread_num; i++)
	{
		backends[i] = i;
		redis_[i] = new RedisAdapter(hdd_ip, 10100 + i);
		int b = redis_[i] -> Open();
		printf("port %d, open %d", 10100+i, b);
		backends[i] = (int64_t)redis_[i];
	}
  printf("\n");
	Lanton_t *lanton = new_lanton(backends, thread_num, DEFAULT_LOOKUPTABLE_SIZE);
	for(int64_t i = 0; i < thread_num; i++)
        {
                add_backend(lanton, backends[i]);
        }
	int64_t endpoint = lookup_backend(lanton, (void*)(&result), sizeof(result));
	printf("Endpoint :%lld\n", endpoint);
	HyperLogLog hyperll(4ll);
	for (size_t i = 0; i < 1; ++i) {
                hyperll.update(result);
        }
	printf("estimate distinct key:%.1d\n", hyperll.raw_estimate());
  std::string value;
  int has_result = ((RedisAdapter*)endpoint)->Get("key:000000059611",value);
  printf("Got:%s",value.c_str());
  // #
  const double D1 = 0.03378;
	const double E1 = 0.1009;
	const double F1 = 1.0;
  if ( hyperll.raw_estimate() < D1 * BASE ) {
    return has_result;
  }
  if ( hyperll.raw_estimate() > D1 * BASE && hyperll.raw_estimate() < E1 * BASE
    && z=='i' || z=='j'
  ) {
    // 二次回捞
  }
  if ( hyperll.raw_estimate() > E1 * BASE && hyperll.raw_estimate() < F1 * BASE && z=='i' ) {
    // 三次回捞
  }
  if (has_result)
    return 1;
  return 0;
}
