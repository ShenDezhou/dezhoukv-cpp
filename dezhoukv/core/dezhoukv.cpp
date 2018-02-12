/*
Copyright (c) 2018-2019 Dezhou Shen,  Tsinghua
   Licensed under the Apache License,  Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License
       
       http://www.apache.org/licenses/LICENSE-2.0
       
   Unless required by applicable law or agreed to in writing,
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,  either express or implied.
   See the License for the specific language governing permissions
   limitations under the License.
*/
#include "dezhoukv.h"

#include <murmurhash3/MurmurHash3.h>
#include <boost/bloom_filter/dynamic_counting_bloom_filter.hpp>
#include <maglev/include/Lanton.h>
#include <hyperloglog/hyperloglog.h>
#include <redisio/redis_client.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include "stdafx.h"
#include "config-dummy.h"

#define FULL_VERSION PACKAGE_VERSION "-"

RedisAdapter* *redis_mem;
RedisAdapter* *redis_ssd;
RedisAdapter* *redis_hdd;
boost::bloom_filters::dynamic_counting_bloom_filter<int64_t, 2>  bloom;
Lanton_t* lanton[3];
HyperLogLog hyperll(4ll);

int foo() {
    return 1;
}

/**
 * 转换 a.b.c.d 形式的版本号为32bit整数
 */
uint32_t CONVERT_VERSION(const char* text_version) {
    uint32_t ret  =  0;
    uint64_t v;
    const char* p  =  text_version;
    char* q  =  NULL;
    v  =  strtoul(p,  &q,  16);
    ret |=  (v & 0xfful)  <<  24;
    if (!*q) return ret;
    v  =  strtoul(q+1,  &q,  16);
    ret |=  (v & 0xfful)  <<  16;
    if (!*q) return ret;
    v  =  strtoul(q+1,  &q,  16);
    ret |=  (v & 0xfful)  <<  8;
    if (!*q) return ret;
    v  =  strtoul(q+1,  &q,  16);
    ret |=  (v & 0xfful);
    return ret;
}

/**
 * 转换 a.b.c.d-svn 形式的版本号为64bit整数
 * s表示svn版本编号，
 */
uint64_t CONVERT_FULL_VERSION(const char* text_svnversion) {
    const char* text_svn  =  strrchr(text_svnversion,  '-');
    char* text_prefix;
    char* q;
    uint64_t ret;
    if (text_svn  ==  NULL) {
        return CONVERT_VERSION(text_svnversion);
    }
    text_prefix  =  (char*)alloca(text_svn - text_svnversion + 1);
    memcpy(text_prefix,  text_svnversion,  text_svn - text_svnversion);
    text_prefix[text_svn - text_svnversion]  =  0;
    ret  =  ((uint64_t)CONVERT_VERSION(text_prefix))  <<  32ull;
    ++text_svn;
    ret |=  (strtoul(text_svn,  &q,  16) & 0xffffff)  <<  8;
    if (*q)
        ret |=  0x0f;
    return ret;
}

int init_mem() {
    printf("Hello %s %s-%s,  %08x,  %016llx\n",  PACKAGE_NAME,  PACKAGE_VERSION,  SVNVERSION,  CONVERT_VERSION(PACKAGE_VERSION),  CONVERT_FULL_VERSION(FULL_VERSION));
    int64_t backends[1];
    int64_t thread_num = 1;
    redis_mem  =  new RedisAdapter*[1];
    for (int64_t i  =  0; i  <  thread_num; i++)
    {
        backends[i]  =  i;
        redis_mem[i]  =  new RedisAdapter(mem_ip,  6379 + i);
        int b  =  redis_mem[i] ->  Open();
        printf("port %d,  open %d\t",  6379+i,  b);
        backends[i]  =  (int64_t)redis_mem[i];
    }
    lanton[0]  =  new_lanton(backends,  thread_num,  DEFAULT_LOOKUPTABLE_SIZE);
    printf("\n");
}

int init_ssd() {
    printf("Hello %s %s-%s,  %08x,  %016llx\n",  PACKAGE_NAME,  PACKAGE_VERSION,  SVNVERSION,  CONVERT_VERSION(PACKAGE_VERSION),  CONVERT_FULL_VERSION(FULL_VERSION));
    int64_t backends[1];
    int64_t thread_num = 1;
    redis_ssd  =  new RedisAdapter*[1];
    for (int64_t i  =  0; i  <  1; i++) {
        backends[i]  =  i;
        redis_ssd[i]  =  new RedisAdapter(ssd_ip,  8323);
        int b  =  redis_ssd[i] ->  Open();
        printf("port %d,  open %d\t",  8323,  b);
        backends[i]  =  (int64_t)redis_ssd[i];
    }
    lanton[1]  =  new_lanton(backends,  thread_num,  DEFAULT_LOOKUPTABLE_SIZE);
    printf("\n");
}

int init_hdd() {
    printf("Hello %s %s-%s,  %08x,  %016llx\n",  PACKAGE_NAME,  PACKAGE_VERSION,  SVNVERSION,  CONVERT_VERSION(PACKAGE_VERSION),  CONVERT_FULL_VERSION(FULL_VERSION));
    int64_t backends[100];
    int64_t thread_num = 100;
    redis_hdd  =  new RedisAdapter*[100];
    for (int64_t i  =  0; i  <  thread_num; i++) {
        backends[i]  =  i;
        redis_hdd[i]  =  new RedisAdapter(hdd_ip,  10100 + i);
        int b  =  redis_hdd[i] ->  Open();
        printf("port %d,  open %d\t",  10100+i,  b);
        backends[i]  =  (int64_t)redis_hdd[i];
    }
    lanton[2]  =  new_lanton(backends,  thread_num,  DEFAULT_LOOKUPTABLE_SIZE);
    printf("\n");
}

int get(const char* key,  std::string &value) {
    printf("Get %s %s-%s,  %08x,  %016llx\n",  PACKAGE_NAME,  PACKAGE_VERSION,  SVNVERSION,  CONVERT_VERSION(PACKAGE_VERSION),  CONVERT_FULL_VERSION(FULL_VERSION));
    char out[17];
    MurmurHash3_x64_128(key, strlen(key), 0, out);
    int64_t hash  = ((int64_t*)out)[0] ^ ((int64_t*)out)[1];
    int64_t result  =  hash % BASE + OFFSET;
    for (int i = 0; i < 1 ; i++) {
        printf("hashed:%016llx\t", result);
    }
    char z;
    if ( A1  <  result && result  <= 0 ) {
        printf("i\n");
        z = 'i';
    }
    else if ( 0  <  result && result  <=  B1 ) {
        printf("j\n");
        z = 'j';
    }
    else if ( B1  <  result && result  <=  C1 ) {
        printf("k\n");
        z = 'k';
    }
    else {
        printf("Error\n");
        exit(-1);
    }
    if (bloom.probably_contains(result)) {
      for (size_t i  =  0; i  <  1; ++i) {
            printf("false positive rate:%.4f\n", bloom.false_positive_rate());
      }
    int choose  =  z-'i';
    double current_count  =  hyperll.raw_estimate();
    printf("estimate distinct key:%.1d\n",  current_count);
    if ( current_count  <  D1 * BASE ) {
        printf("max_possible requests:1\n");
        int64_t endpoint  =  lookup_backend(lanton[choose],  (void*)(&result),  sizeof(result));
        printf("Endpoint :%016llx\n",  endpoint);
        int has_result  =  ((RedisAdapter*)endpoint)-> Get(key, value);
        printf("Got:%s\n", value.c_str());
        if (has_result)
            return has_result;
    }
    else if ( current_count  >=  D1 * BASE && current_count  <  E1 * BASE ) {
        printf("max_possible requests:2\n");
        for (size_t i  =  0; i  <  2; ++i) {
          int64_t endpoint  =  lookup_backend(lanton[choose],  (void*)(&result),  sizeof(result));
          printf("Endpoint :%016llx\n",  endpoint);
          int has_result  =  ((RedisAdapter*)endpoint)-> Get(key, value);
          printf("Got:%s\n", value.c_str());
          if (has_result)
            return has_result;
          if( (char)(z+i) < 'k' ) {
            choose++;
            break;
          }  
      }
    }
    else if ( current_count  >=  E1 * BASE && current_count  <=  F1 * BASE ) {
        printf("max_possible requests:3\n");
        for (size_t i  =  0; i  <  3; ++i) {
          int64_t endpoint  =  lookup_backend(lanton[choose],  (void*)(&result),  sizeof(result));
          printf("Endpoint :%016llx\n",  endpoint);
          int has_result  =  ((RedisAdapter*)endpoint)-> Get(key, value);
          printf("Got:%s\n", value.c_str());
          if (has_result)
            return has_result;
          if( (char)(z+i) < 'k' ) {
            choose++;
            break;
          }
      }
    }
  }
  return 0;
}

int set(const char* key,  std::string value) {
    printf("Set %s %s-%s,  %08x,  %016llx\n",  PACKAGE_NAME,  PACKAGE_VERSION,  SVNVERSION,  CONVERT_VERSION(PACKAGE_VERSION),  CONVERT_FULL_VERSION(FULL_VERSION));
    char out[17];
    MurmurHash3_x64_128(key, strlen(key), 0, out);
    int64_t hash  = ((int64_t*)out)[0] ^ ((int64_t*)out)[1];
    int64_t result  =  (int64_t)(hash % BASE) + OFFSET;
    for (int i = 0; i < 1 ; i++) {
        printf("hashed:%016llx\t", result);
    }
    char z;
    if ( A1  <  result && result  <= 0 ) {
        printf("i\n");
        z = 'i';
    }
    else if ( 0  <  result && result  <=  B1 ) {
        printf("j\n");
        z = 'j';
    }
    else if ( B1  <  result && result  <=  C1 ) {
        printf("k\n");
        z = 'k';
    }
    else {
        printf("Error\n");
        exit(-1);
    }
    bloom.insert(result);
    for (size_t i  =  0; i  <  1; ++i) {
          printf("false positive rate:%.4f\n", bloom.false_positive_rate());
    }
    double current_count  =  hyperll.raw_estimate();
    printf("estimate distinct key:%.1d\n",  current_count);
    int choose;
    if ( current_count  <  D1 * BASE ) {
      choose  =  0;
    }
    else if ( current_count >=  D1 * BASE && current_count <  E1 * BASE && z == 'i' || z == 'j'
    ) {
       choose  =  1;
    }
    else if ( current_count >=  E1 * BASE && current_count <  F1 * BASE && z == 'i' ) {
       choose  =  2;
    } else {
       choose  =  -1;
    }
    char z_;
    if ( z+choose  >= 'i' && z+choose  <= 'k' ) {
      z_  =  (char)(z+choose);
    }
    if ( choose  ==  -1 ) {
      z_  =  'k';
    }
    choose  =  z_-'i';
    int64_t endpoint  =  lookup_backend(lanton[choose],  (void*)(&result),  sizeof(result));
    printf("Endpoint :%016llx\n",  endpoint);
    int has_result  =  ((RedisAdapter*)endpoint)-> Set(key, value);
    printf("Set=%c,Result=%d\n",  z,  has_result);
    return has_result;
    return 0;
}

