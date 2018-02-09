#!/bin/bash
BASEDIR=$(cd $(dirname $0); pwd)
if [ "$PWD" = "$BASEDIR" ]; then
	echo 1>&2 "You cannot run $0 under this directory!!"
	exit 1
fi

if ! [ -f "./configure.ac" -a -d "./.svn" ]; then
	echo 1>&2 "You cannot run $0 under this directory!!"
	exit 1
fi

# update for pkg_config
FILES=(acsite.m4 buildrpm.mk collect.mk common-rules.mk pkg.pc.in GNUmakefile) 
for f in "${FILES[@]}"; do
	if [ -f "$f" ] && ! diff -q $f $BASEDIR/$f &> /dev/null; then
		(set -x; cp $BASEDIR/$f ./ ) 2>&1
	fi
done

if ! fgrep -q PKG_PROG_PKG_CONFIG configure.ac; then
	( set -x; sed -i '/AC_PROG_RANLIB/ i\PKG_PROG_PKG_CONFIG' configure.ac ) 2>&1
fi
if ! fgrep -q PKG_PROG_PKG_CONFIG configure.ac; then
	( set -x; sed -i '/AC_PROG_AS/ a\PKG_PROG_PKG_CONFIG' configure.ac ) 2>&1
fi
if ! fgrep -q PKG_PROG_PKG_CONFIG configure.ac; then
	( set -x; sed -i '/AC_PROG_CXX/ a\PKG_PROG_PKG_CONFIG' configure.ac ) 2>&1
fi
if ! fgrep -q PKG_PROG_PKG_CONFIG configure.ac; then
	echo 1>&2 "!!! You must insert PKG_PROG_PKG_CONFIG into configure.ac manually!!!"
	exit 1
fi

if fgrep -q Library/Linux configure.ac; then
	( set -x; sed -i '/AK_CHECK_MODULE_ROOT/ s/\[\], \[Library\/Linux\])/[_include], [_lib])/g' configure.ac ) 2>&1
fi

if fgrep -q ', [include], [lib])'  configure.ac; then
	( set -x; sed -i '/AK_CHECK_MODULE_ROOT/ s/\[include\], \[lib\])/[_include], [_lib])/g' configure.ac ) 2>&1
fi

# update for collect _include _lib _bin
if [ -f collect.mk ] && \
	{ ! fgrep -q collectinclude_DIR common.mk || \
		fgrep -q 'collectinclude_DIR=$(top_builddir)/include' common.mk; } then
	( set -x;
		sed -i '/^collect.*_DIR=/ d' common.mk; 
		sed -i '/^include \$(top_srcdir)\/collect.mk/ {
			i collectlib_DIR=$(top_builddir)/_lib
			i collectbin_DIR=$(top_builddir)/_bin
			i collectinclude_DIR=$(top_builddir)/_include
		}' common.mk
	) 2>&1 
fi

if [ ! -f collect.mk ]; then
	echo "@@@ I strongly suggest you adding 'collect.mk' into your project!"
	echo "@@@     1. do cp my 'collect.mk' here."
	echo "@@@     2. do modify your 'common.mk'."
fi

# update for MODULE_ROOT_VARS
if grep -q '^export @MODULE_ROOT_VARS@$' Makefile.am; then
	( set -x; sed -i '/^export @MODULE_ROOT_VARS@$/d; s/export BUILD_DEPENDENCIES/export @MODULE_ROOT_VARS@ BUILD_DEPENDENCIES/;' Makefile.am ) 2>&1
fi

# update for module encoding
if grep -q '(ENCODING_ROOT.*, *encoding.*)' configure.ac; then
	sed -i '/(ENCODING_ROOT.*, *encoding.*)/ {
		c\AK_CHECK_MODULE_ROOT(SSENCODING_ROOT, [the root of module encoding], ssplatform-encoding, [_include], [_lib])
	}' configure.ac
fi

#for libtool REVISION NUMBER bug
if ! grep -q 'AK_PATCH_LIBTOOL' configure.ac; then 
	sed -i '/^AC_OUTPUT/ a \AK_PATCH_LIBTOOL' configure.ac 
fi

#show diffs
for f in *; do
	if [ -f "$f" ] && svn info "$f" &> /dev/null; then
		svn diff "$f"
	fi
done

