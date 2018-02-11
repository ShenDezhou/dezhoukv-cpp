include $(top_srcdir)/common.mk

#for executable
bin_PROGRAMS=dezhoukv 
dezhoukv_SOURCES=dezhoukv.cpp stdafx.cpp stdafx.h 
dezhoukv_LDADD=libdezhoukv2.la libmurmurhash3.la libbloomfilter.la libmaglev.la libredisio.la
#dezhoukv_LDADD=libdezhoukv2.la @db4_LDADD@

# Custom flags
#dezhoukv_LDADD=-lz
dezhoukv_CPPFLAGS=$(AM_CPPFLAGS) -I$(top_srcdir)/dezhoukv/bloomfilters/ -I$(top_srcdir)/dezhoukv/maglev/ 
#dezhoukv_LDFLAGS=$(AM_LDFLAGS)
#dezhoukv_CFLAGS=$(AM_CFLAGS) -O3 
#dezhoukv_CXXFLAGS=$(AM_CXXFLAGS) -g

#for dynamic libarary
lib_LTLIBRARIES=libdezhoukv2.la libmurmurhash3.la libbloomfilter.la libmaglev.la libhyperloglog.la libredisio.la
libdezhoukv2_la_SOURCES=dezhoukv2.cpp ktest.h 
libdezhoukv2_la_LDFLAGS=-version-number 0:5:$(SVNVERSION) -lhiredis -lboost_system -Ibloomfilters/

libmurmurhash3_la_SOURCES=murmurhash3/MurmurHash3.cpp murmurhash3/MurmurHash3.h
libmurmurhash3_la_LDFLAGS=-version-number 0:5:$(SVNVERSION)

libbloomfilter_la_SOURCES=bloomfilters/boost/bloom_filter/basic_bloom_filter.hpp
libbloomfilter_la_LDFLAGS=-version-number 0:5:$(SVNVERSION) 

libmaglev_la_SOURCES=maglev/src/csiphash.c maglev/src/Lanton.c
libmaglev_la_LDFLAGS=-version-number 0:5:$(SVNVERSION) -Imaglev/include/

libhyperloglog_la_SOURCES=hyperloglog/hyperloglog.h
libhyperloglog_la_LDFLAGS=-version-number 0:5:$(SVNVERSION)

libredisio_la_SOURCES=redisio/redis_client.cpp redisio/redis_client.h
libredisio_la_LDFLAGS=-version-number 0:5:$(SVNVERSION) -lhiredis 

#for static libraray
noinst_LIBRARIES=libdezhoukv3.a
libdezhoukv3_a_SOURCES=dezhoukv3.cpp ktest.h

#list all tests, "make check" will execute programs below
TESTS=dezhoukvtest dezhoukvtest2
#dezhoukvtest2 are expected to fail.
XFAIL_TESTS=dezhoukvtest2

#list PROGRAMS just built for test, "make check" will build programs below
noinst_PROGRAMS=dezhoukvtest dezhoukvtest2

dezhoukvtest_SOURCES=dezhoukvtest.cpp ktest.h
dezhoukvtest_LDADD=libdezhoukv2.la

dezhoukvtest2_SOURCES=dezhoukvtest.cpp
dezhoukvtest2_LDADD=libdezhoukv3.a

# include $(top_srcdir)/util-gch-dbg.mk
# 
# # seperated debug info
# bindbg_DATA=dezhoukv.dbg 
# libdbg_DATA=libdezhoukv2.so.dbg
# 
# # precomplied header
# BUILT_SOURCES+=stdafx.h.gch

