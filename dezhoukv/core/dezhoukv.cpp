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
#include <boost/bloom_filter/hash/murmurhash3.hpp>
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

const int NODE_NUM = 1;
const int PRIME_NODE = DEFAULT_LOOKUPTABLE_SIZE;
const int DRAM_PORT = 10100;
const int SSD_PORT = 10100;
const int HDD_PORT = 10100;
const uint64_t BASE   =  15000000000ll;
const int64_t OFFSET  = -10000000000ll;
const int64_t A1     = -10000000000ll;
const int64_t B1     =   1000000000ll;
const int64_t C1     =   5000000000ll;
const char* const mem_ip = "127.0.0.1";
const char* const ssd_ip = "127.0.0.1";
const char* const hdd_ip = "127.0.0.1";
const double D1 = 0.03378;
const double E1 = 0.1009;
const double F1 = 1.0;
const int SUCCESS = 0;
const int ERROR = -1;
RedisAdapter* *redis_mem;
RedisAdapter* *redis_ssd;
RedisAdapter* *redis_hdd;
typedef boost::mpl::vector<boost::bloom_filters::murmurhash3<uint64_t, 0> > murmurhash3_hash;
boost::bloom_filters::dynamic_counting_bloom_filter<uint64_t, 32, murmurhash3_hash, uint64_t>  bloom;
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
    int64_t thread_num = NODE_NUM;
    int64_t backends[thread_num];
    redis_mem  =  new RedisAdapter*[thread_num];
    for (int64_t i  =  0; i  <  thread_num; i++)
    {
        backends[i]  =  i;
        redis_mem[i]  =  new RedisAdapter(mem_ip,  DRAM_PORT + i);
        int b  =  redis_mem[i] ->  Open();
        printf("port %d,  open %d\t",  DRAM_PORT+i,  b);
        backends[i]  =  (int64_t)redis_mem[i];
    }
    lanton[0]  =  new_lanton(backends,  thread_num,  PRIME_NODE);
    printf("\n");
}

int init_ssd() {
    printf("Hello %s %s-%s,  %08x,  %016llx\n",  PACKAGE_NAME,  PACKAGE_VERSION,  SVNVERSION,  CONVERT_VERSION(PACKAGE_VERSION),  CONVERT_FULL_VERSION(FULL_VERSION));
    int64_t thread_num = NODE_NUM;
    int64_t backends[thread_num];
    redis_ssd  =  new RedisAdapter*[thread_num];
    for (int64_t i  =  0; i  < thread_num ; i++) {
        backends[i]  =  i;
        redis_ssd[i]  =  new RedisAdapter(ssd_ip,  SSD_PORT + i);
        int b  =  redis_ssd[i] ->  Open();
        printf("port %d,  open %d\t",  SSD_PORT + i,  b);
        backends[i]  =  (int64_t)redis_ssd[i];
    }
    lanton[1]  =  new_lanton(backends,  thread_num,  PRIME_NODE);
    printf("\n");
}

int init_hdd() {
    printf("Hello %s %s-%s,  %08x,  %016llx\n",  PACKAGE_NAME,  PACKAGE_VERSION,  SVNVERSION,  CONVERT_VERSION(PACKAGE_VERSION),  CONVERT_FULL_VERSION(FULL_VERSION));
    int64_t thread_num = NODE_NUM;
    int64_t backends[thread_num];
    redis_hdd  =  new RedisAdapter*[thread_num];
    for (int64_t i  =  0; i  <  thread_num; i++) {
        backends[i]  =  i;
        redis_hdd[i]  =  new RedisAdapter(hdd_ip,  HDD_PORT + i);
        int b  =  redis_hdd[i] ->  Open();
        printf("port %d,  open %d\t",  HDD_PORT + i,  b);
        backends[i]  =  (int64_t)redis_hdd[i];
    }
    lanton[2]  =  new_lanton(backends,  thread_num,  PRIME_NODE);
    printf("\n");
}

int get(const char* key,  std::string &value) {
    //printf("Get %s %s-%s,  %08x,  %016llx\n",  PACKAGE_NAME,  PACKAGE_VERSION,  SVNVERSION,  CONVERT_VERSION(PACKAGE_VERSION),  CONVERT_FULL_VERSION(FULL_VERSION));
    char out[17];
    double start = get_time();
    MurmurHash3_x64_128(key, strlen(key), 0, out);
    int64_t hash  = ((int64_t*)out)[0] ^ ((int64_t*)out)[1];
    int64_t result  =  (int64_t)(hash % BASE) + OFFSET;
    for (int i = 0; i < 1 ; i++) {
        //printf("hashed:%016llx,t=%f\t", result,  get_time()-start);
    }
    char z;
    if ( A1  <  result && result  <= 0 ) {
        //printf("i\n");
        z = 'j';
    }
    else if ( 0  <  result && result  <=  B1 ) {
        //printf("j\n");
        z = 'k';
    }
    else if ( B1  <  result && result  <=  C1 ) {
        //printf("k\n");
        z = 'i';
    }
    else {
        //printf("Error\n");
        return ERROR;
    }
    start = get_time();
    if (bloom.probably_contains(result)) {
      for (size_t i  =  0; i  <  1; ++i) {
            //printf("false positive rate:%.4f,t=%f\n", bloom.false_positive_rate(), get_time()-start);
      }
    int choose  =  z-'i';
    start = get_time();
    // double current_count  =  hyperll.raw_estimate();
    //printf("estimate distinct key:%.1f,t=%f\n",  current_count, get_time()-start);
    // if ( current_count  <  D1 * BASE ) {
        //printf("max_possible requests:1\n");
        start = get_time();
        int64_t endpoint  =  lookup_backend(lanton[choose],  (uint64_t) result);
        //printf("Endpoint :%016llx,t=%f\n",  endpoint, get_time()-start);
        double start = get_time();
        int has_result  =  ((RedisAdapter*)endpoint)-> Get(key, value);
        printf("u=%c,r=%d,t=%f\n",z, has_result, get_time()-start);
        if (has_result>0)
          return SUCCESS;
    // }
    // else if ( current_count  >=  D1 * BASE && current_count  <  E1 * BASE ) {
    //     printf("max_possible requests:2\n");
    //     for (size_t i  =  0; i  <  2; ++i) {
    //       double start = get_time();
    //       int64_t endpoint  =  lookup_backend(lanton[choose],   (uint64_t) result);
    //       printf("Endpoint :%016llx,t=%f\n",  endpoint, get_time()-start);
    //       start = get_time();
    //       int has_result  =  ((RedisAdapter*)endpoint)-> Get(key, value);
    //       printf("Got:%s,t=%f\n", value.c_str(), get_time()-start);
    //       if (has_result>0)
    //         return SUCCESS;
    //       if ( (char)(z+i) < 'k' ) {
    //         choose++;
    //         break;
    //       }  
    //   }
    // }
    // else if ( current_count  >=  E1 * BASE && current_count  <=  F1 * BASE ) {
    //     printf("max_possible requests:3\n");
    //     for (size_t i  =  0; i  <  3; ++i) {
    //       double start = get_time();
    //       int64_t endpoint  =  lookup_backend(lanton[choose],   (uint64_t) result);
    //       printf("Endpoint :%016llx,t=%f\n",  endpoint, get_time()-start);
    //       start = get_time();
    //       int has_result  =  ((RedisAdapter*)endpoint)-> Get(key, value);
    //       printf("Got:%s,t=%f\n", value.c_str(), get_time()-start);
    //       if (has_result>0)
    //         return SUCCESS;
    //       if ( (char)(z+i) < 'k' ) {
    //         choose++;
    //         break;
    //       }
    //   }
    // }
  }
  return ERROR;
}

int set(const char* key,  std::string value) {
    //printf("Set %s %s-%s,  %08x,  %016llx\n",  PACKAGE_NAME,  PACKAGE_VERSION,  SVNVERSION,  CONVERT_VERSION(PACKAGE_VERSION),  CONVERT_FULL_VERSION(FULL_VERSION));
    char out[17];
    double start = get_time();
    MurmurHash3_x64_128(key, strlen(key), 0, out);
    int64_t hash  = ((int64_t*)out)[0] ^ ((int64_t*)out)[1];
    int64_t result  =  (int64_t)(hash % BASE) + OFFSET;
    for (int i = 0; i < 1 ; i++) {
        //printf("hashed:%016llx,t=%f\t", result, get_time()-start);
    }
    start = get_time();
    char z;
    if ( A1  <  result && result  <= 0 ) {
        //printf("i\n");
        z = 'j';
    }
    else if ( 0  <  result && result  <=  B1 ) {
        //printf("j\n");
        z = 'k';
    }
    else if ( B1  <  result && result  <=  C1 ) {
        //printf("k\n");
        z = 'i';
    }
    else {
        //printf("Error1\n");
        return ERROR;
    }
    bloom.insert(result);
    for (size_t i  =  0; i  <  1; ++i) {
          //printf("false positive rate:%.4f,t=%f\n", bloom.false_positive_rate(), get_time()-start);
    }
    start = get_time();
    hyperll.update(result);
    // double current_count  =  hyperll.raw_estimate();
    // //printf("estimate distinct key:%.1f,t=%f\n",  current_count, get_time()-start);
    // start = get_time();
    // int choose;
    // if ( (uint64_t)current_count  <  D1 * BASE ) {
    //    choose  =  0;
    // }
    // else if ( (uint64_t)current_count >=  D1 * BASE && (uint64_t)current_count <  E1 * BASE) {
    //    choose  =  1;
    // }
    // else if ( (uint64_t)current_count >=  E1 * BASE && (uint64_t)current_count <  F1 * BASE) {
    //    choose  =  2;
    // } else {
    //    //printf("Error2\n");
    //    return ERROR;
    // }
    // char z_;
    // if ( z+choose  >= 'i' && z+choose <= 'k' ) {
    //   z_  =  (char)(z+choose);
    // } else {
    //   z_  =  'k';
    // }
    int choose  =  z-'i';
    int64_t endpoint  =  lookup_backend(lanton[choose], (uint64_t) result);
    //printf("Endpoint :%016llx,t=%f\n",  endpoint, get_time()-start);
    start = get_time();
    int has_result  =  ((RedisAdapter*)endpoint)-> Set(key, value);
    printf("u=%c,r=%d,t=%f\n",  z,  has_result, get_time()-start);
    if (has_result == 0)
        return SUCCESS;
    return ERROR;

}

