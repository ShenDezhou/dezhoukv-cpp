/* <DEZHOUKV>
 DEZHOUKV is a C implement of Google's consistent hashing algorithm used in Maglev system
 It is under support of siphash(https://github.com/majek/csiphash)
 The code is released under the MIT License

 <MIT License>
 Copyright (c) 2013  Marek Majkowski <marek@popcount.org>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 </MIT License>
*/



#ifndef __DEZHOUKV_H__
#define __DEZHOUKV_H__
#include <stdint.h>
#include <string>

#ifdef __cplusplus
extern "C"
{
#endif

const uint64_t BASE   =  7500000000ll;
const int64_t OFFSET  = -2500000000ll;
const int64_t A1     = -10000000000ll;
const int64_t B1     =   1000000000ll;
const int64_t C1     =   5000000000ll;
const char* mem_ip = "10.134.100.140";
const char* ssd_ip = "10.134.110.46";
const char* hdd_ip = "39.104.63.175";
const double D1 = 0.03378;
const double E1 = 0.1009;
const double F1 = 1.0;


int init_mem();
int init_ssd();
int init_hdd();

inline int init() {
  init_mem();
  init_ssd();
  init_hdd();
}

#include <sys/time.h>

inline double get_time() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + (tv.tv_usec / 1e6);
}

int get(const char* key, std::string &value);
int set(const char* key, std::string value);

#ifdef __cplusplus
}
#endif

#endif

