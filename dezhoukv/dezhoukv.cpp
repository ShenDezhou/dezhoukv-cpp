#include "stdafx.h"
#include "config-dummy.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define FULL_VERSION PACKAGE_VERSION "-" SVNVERSION

int foo()
{
	return 1;
}

/**
 * ת�� a.b.c.d ��ʽ�İ汾��Ϊ32bit���� 0xaabbccdd
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
 * ת�� a.b.c.d-svn ��ʽ�İ汾��Ϊ64bit���� 0xaabbccddssssssmm
 * s��ʾsvn�汾��ţ�m��ʾ�Ƿ���һ���ɾ��İ汾
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
}
