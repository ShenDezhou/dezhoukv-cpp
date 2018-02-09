
dnl defined by kirbyzhou

dnl AK_DETECT_LIB(package-name, pkg-search-paths, header-file, lib-names, 
dnl 	functions, action-if-found, action-if-not-found, 
dnl 	sub-includedirs, sub-libdirs)
dnl e.g:
dnl AK_DETECT_LIB(ace551, [/usr/local/src/ACE* /usr/local/ACE* /opt/ACE*],
dnl 	[ace/OS.h], [ACE], [ACE::init],
dnl 	[], [AC_MSG_ERROR(ACE not found)], [.], [lib])
AC_DEFUN([AK_DETECT_LIB], [
dnl 1	package-name
dnl 2	pkg-search-paths
dnl 3	header-file
dnl 4	lib-names
dnl 5	functions
dnl 6	action-if-found
dnl 7	action-if-not-found
dnl 8	sub-includedirs
dnl 9	sub-libdirs

	AKDL_INCFOUND=
	AKDL_LIBFOUND=
	AKDL_LIBNAME=
	AKDL_ROOT=
	AC_CHECK_HEADERS([$3], [AKDL_INCFOUND=yes])
	if test "$AKDL_INCFOUND" != yes; then
		for tmp_AKDL_ROOT in $2
		do
			AC_MSG_NOTICE(try path = $tmp_AKDL_ROOT)
			test -d "$tmp_AKDL_ROOT" || continue
			old_CPPFLAGS=$CPPFLAGS
			if test -n "$8"; then
				for i in $8; do CPPFLAGS="$CPPFLAGS -I$tmp_AKDL_ROOT/$i"; done
			else
				CPPFLAGS="$CPPFLAGS -I$tmp_AKDL_ROOT"
			fi
			
			unset AS_TR_SH([ac_cv_header_$3])
			AC_CHECK_HEADERS([$3], [AKDL_INCFOUND=yes])
			if test "$AKDL_INCFOUND" = yes; then
				AKDL_ROOT=$tmp_AKDL_ROOT
				break
			fi

			CPPFLAGS=$old_CPPFLAGS
		done
		unset old_CPPFLAGS
		unset tmp_AKDL_ROOT
	fi

	if test -d "$AKDL_ROOT" && test -n "$9"; then
		for i in $9; do LIBS="$LIBS -L$AKDL_ROOT/$i"; done
	elif test -n "$AKDL_ROOT"; then
		LIBS="$LIBS -L$AKDL_ROOT"
	fi
	AC_FOREACH([ak_func], [$5], [
		AC_FOREACH([ak_lib], [$4], [
			if test "$AKDL_LIBFOUND" != yes; then
				ak_saved_LIBS="$LIBS"
				LIBS="$LIBS -l[]ak_lib"
				AC_MSG_CHECKING([for library ak_lib with ak_func])
				AC_TRY_LINK(
					[
						@%:@include <stdio.h>
						@%:@include <$3>
					],
					[	printf("%p", &(] ak_func [) ) ],
					[
						AKDL_LIBFOUND=yes;
						AKDL_LIBNAME=ak_lib;
						AC_MSG_RESULT(ok);
					],
					[	AC_MSG_RESULT(failed); ]
					)
				LIBS="$ak_saved_LIBS"
			fi
		])
	])
	test -z "$5" && AKDL_LIBFOUND=yes
	
	if test "$AKDL_INCFOUND" = yes -a "$AKDL_LIBFOUND" = yes; then
		$6
		echo > /dev/null
	else
		$7
		echo > /dev/null
	fi
	test -n "$AKDL_LIBNAME" && eval "$1_LDADD=-l$AKDL_LIBNAME"
	AC_SUBST($1_LDADD)
	unset AKDL_INCFOUND AKDL_LIBFOUND AKDL_LIBNAME AKDL_ROOT
]) dnl AK_DETECT_LIB

dnl AK_DETECT_LIB2(package-name, pkg-search-paths, header-file, lib-names, 
dnl 	functions, is-pkg-required, extra-includedirs, extra-libdirs)
dnl e.g:
dnl AK_DETECT_LIB2(ace551, [/usr/local/src/ACE* /usr/local/ACE* /opt/ACE*],
dnl 	[ace/ACE.h], [ACE], [ACE::init],
dnl 	[optional], [.], [lib])
AC_DEFUN([AK_DETECT_LIB2], [
dnl 1	package-name
dnl 2	pkg-search-paths
dnl 3	header-file
dnl 4	lib-names
dnl 5	functions
dnl 6	is-pkg-required
dnl 7	sub-includedirs
dnl 8	sub-libdirs
libRequired="$6"

test "$libRequired" = "required" && libRequired=yes

AC_ARG_WITH([$1],
	AS_HELP_STRING([--with-$1[=DIR]], [tell me the $1 directory.]),
	AK_LIB_WITH="$withval", AK_LIB_WITH="default-yes")

if test "$AK_LIB_WITH" = no ; then
	AC_MSG_WARN([=== $1 is missing ===])
else 
	if ! test -d "$AK_LIB_WITH"; then
		AK_LIB_PATHS="/usr/local/src/$1* /usr/local/$1* /opt/$1* $2"
	elif test -d "$AK_LIB_WITH"; then
		AK_LIB_PATHS=$AK_LIB_WITH
	fi

	if test "$libRequired" = yes; then
		AK_DETECT_LIB([$1],[$2],[$3],[$4],[$5],
		[	AC_MSG_CHECKING($1 directory)
			AC_MSG_RESULT($1_ROOT=$AKDL_ROOT)
		],
		[	AC_MSG_ERROR([=== detect $1 fail $1_ROOT=$AKDL_ROOT ===]);
		],
		[$7],[$8])
	else
		AK_DETECT_LIB([$1],[$2],[$3],[$4],[$5],
		[	AC_MSG_CHECKING($1 directory)
			AC_MSG_RESULT($1_ROOT=$AKDL_ROOT)
		],
		[	test "$AK_LIB_WITH" = default-yes && AC_MSG_WARN([=== detect $1 fail $1_ROOT=$AKDL_ROOT ===]);
			test "$AK_LIB_WITH" != default-yes && AC_MSG_ERROR([=== detect $1 fail $1_ROOT=$AKDL_ROOT ===]);
		],
		[$7],[$8])
	fi
fi

]) dnl AK_DETECT_LIB2

dnl ======================================================================

dnl AK_STD_FEATURES()
AC_DEFUN([AK_STD_FEATURES], [

	AC_REQUIRE([AC_CANONICAL_HOST])
	AC_REQUIRE([AC_PROG_CC])
	AC_REQUIRE([AC_PROG_CXX])
	AC_REQUIRE([AC_PROG_LIBTOOL])
	AC_REQUIRE([AC_HEADER_STDC])
	AC_REQUIRE([PKG_PROG_PKG_CONFIG])

	AC_LANG_PUSH(C++)
	AC_CHECK_HEADERS([iostream])
	AC_LANG_POP()

	dnl check platform
	AC_MSG_CHECKING(platform)
	case $host_os in
		@%:@(
		cygwin* )
			;;
		@%:@(
		solaris* )
			CXXFLAGS="$CXXFLAGS -pthreads"
			CFLAGS="$CFLAGS -pthreads"
			LIBS="$LIBS -lsocket -lrt -lnsl"
			;;
		@%:@(
		linux* )
			CXXFLAGS="$CXXFLAGS -pthread"
			CFLAGS="$CFLAGS -pthread"
			LIBS="$LIBS -lnsl"
			;;
	esac
	AC_MSG_RESULT($host_os)

	dnl set x86_64 lib64
	if test "$libdir" = '${exec_prefix}/lib'; then
		case $host in
		@%:@(
		x86_64-*-linux* )
			libdir='${exec_prefix}/lib64';
			;;
		esac
	fi

	CPPFLAGS="$CPPFLAGS -D_REENTRANT"

	dnl check for debug
	AC_ARG_ENABLE(debug,
		AS_HELP_STRING([--disable-debug], [disable the debug feature.]),
		[], [])

	AC_MSG_CHECKING(debug feature)
	AC_MSG_RESULT($enable_debug)
	if test "$enable_debug" = "yes"; then
		CXXFLAGS="$CXXFLAGS -g"
		CFLAGS="$CFLAGS -g"
	elif test "$enable_debug" = "no"; then
		CPPFLAGS="$CPPFLAGS -DNDEBUG"
	fi

	dnl check for optimize
	AC_ARG_ENABLE(optimize,
		AS_HELP_STRING([--disable-optimize],[disable the optimize feature.]),
		[], [])

	AC_MSG_CHECKING(optimize feature)
	AC_MSG_RESULT($enable_optimize)
	if test "$enable_optimize" = "yes"; then
		CXXFLAGS="$CXXFLAGS -O3"
		CFLAGS="$CFLAGS -O3"
	elif test "$enable_optimize" = "no"; then
		CXXFLAGS="$CXXFLAGS -O0"
		CFLAGS="$CFLAGS -O0"
	fi

	dnl check for profile
	AC_ARG_ENABLE(profile,
		AS_HELP_STRING([--enable-profile],[enable the gprof profile feature.]),
		[], [])

	AC_MSG_CHECKING(profile feature)
	AC_MSG_RESULT($enable_profile)
	if test "$enable_profile" = "yes"; then
		CXXFLAGS="$CXXFLAGS -pg"
		CFLAGS="$CFLAGS -pg"
	fi

	dnl check for pre-compiled header 
	AC_ARG_ENABLE(gch,
		AS_HELP_STRING([--disable-gch], [disable the pre-compiled header feature.]),
		[], [enable_gch=yes])

	AC_MSG_CHECKING(precompiled headers feature)
	AC_MSG_RESULT($enable_gch)
	if test "$enable_gch" = "yes"; then
	    AM_CONDITIONAL(WITH_GCH, true)
	elif test "$enable_gch" = "no"; then
	    AM_CONDITIONAL(WITH_GCH, false)
	fi

	dnl check FD_SETSIZE
	AC_COMPILE_IFELSE(
	    [AC_LANG_PROGRAM([[
		@%:@include <sys/select.h>
		typedef int foo[(FD_SETSIZE==1024)?1:-1];
	    ]])],
	    ,
	    AC_MSG_WARN(FD_SETSIZE!=1024) 
	)

	AC_CACHE_SAVE

	AM_CONDITIONAL(ENABLE_STATIC, [ test "$enable_static" = "yes" ])

]) dnl AK_STD_FEATURES

dnl check a sogou module's root
dnl AK_CHECK_MODULE_ROOT(varname, description, pkgname, includepath-suffix, libpath-suffix)
dnl the macro will append values to CPPFLAGS LIBS MODULE_ROOT_VALUES MODULE_ROOT_VARS
AC_DEFUN([AK_CHECK_MODULE_ROOT], [
dnl $1 varname
dnl $2 description
dnl $3 pkgname
dnl $4 includepath-suffix
dnl $5 libpath-suffix 
	AS_VAR_PUSHDEF([ak_pkgvarprefix], [m4_toupper($3_PKG)])
	AS_VAR_PUSHDEF([ak_pkgvarprefix_CPPFLAGS], ak_pkgvarprefix[_CFLAGS])
	AS_VAR_PUSHDEF([ak_pkgvarprefix_LIBS], ak_pkgvarprefix[_LIBS])
	AC_ARG_VAR([$1], [$2])
	AC_MSG_CHECKING([$1])
	if test -z "${$1}" && { test -d "$3" || test -d "$3.default"; } then
		# use default directory
		test -d "$3.default" && { test -d "$3" || ln -s "$3.default" "$3" &> /dev/null; }
		$1="$(pwd)/$3"
		AC_CONFIG_SUBDIRS([$3])
	else
		# use external directory
		test -d "${$1}" && $1="$(cd ${$1} && pwd)"
	fi

	if test -z "${$1}"; then
		# use pkg-config
		AC_MSG_RESULT(failed)
		PKG_CHECK_MODULES(ak_pkgvarprefix, [$3])
		AS_VAR_SET(CPPFLAGS, "$CPPFLAGS $ak_pkgvarprefix_CPPFLAGS")
		AS_VAR_SET(LIBS, "$LIBS $ak_pkgvarprefix_LIBS")
	else
		test -d "${$1}" || { AC_MSG_FAILURE($1=${$1}) }
		AC_MSG_RESULT(${$1})
		test -f "${$1}/Makefile.in"  -o ! -f "${$1}/Makefile.am" || ( cd "${$1}" && autoreconf -isv ) || { AC_MSG_FAILURE($1=${$1} autoreconf failed) }

		test -f "${$1}/Makefile" || ( cd "${$1}" && ./configure ) || { AC_MSG_FAILURE($1=${$1} configure failed) } 

		if test -f "${$1}/$3.pc"; then

			saved_PKG_CONFIG_PATH=$PKG_CONFIG_PATH
			export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:${$1}
			PKG_CHECK_MODULES(ak_pkgvarprefix, [$3])
			AS_VAR_SET(CPPFLAGS, "$CPPFLAGS $ak_pkgvarprefix_CPPFLAGS")
			AS_VAR_SET(LIBS, "$LIBS $ak_pkgvarprefix_LIBS")
			export -n PKG_CONFIG_PATH
			PKG_CONFIG_PATH=$saved_PKG_CONFIG_PATH

		else

			test -z "$4" && CPPFLAGS="$CPPFLAGS -I${$1}"
			test -z "$5" && LIBS="$LIBS -L${$1}"

			AC_FOREACH([suffix], [$4], [ 
					CPPFLAGS=["$CPPFLAGS -I${$1}/]suffix["];
				])
			AC_FOREACH([suffix], [$5], [
					LIBS=["$LIBS -L${$1}/]suffix["];
				])
		fi

		export $1
		MODULE_ROOT_VARS="$MODULE_ROOT_VARS $1"
		AC_SUBST(MODULE_ROOT_VARS)
		MODULE_ROOT_VALUES="$MODULE_ROOT_VALUES ${$1}"
	fi
	AS_VAR_POPDEF([ak_pkgvarprefix])
]) dnl AK_CHECK_MODULE_ROOT

dnl check whether should build dependencies
dnl AK_CHECK_MODULE_ROOT(default-value, dependencies)
dnl the macro read arg-var BUILD_DEPENDENCIES
dnl the macro will subst PACKAGE_DEPENDENCIES
dnl the macro will set condtion BUILD_DEPENDENCIES
AC_DEFUN([AK_CHECK_BUILD_DEPENDENCIES], [
	AC_ARG_VAR(BUILD_DEPENDENCIES, [to build dependent module? default = $1])
	AC_MSG_CHECKING(BUILD_DEPENDENCIES)
	AC_MSG_RESULT(${BUILD_DEPENDENCIES=$1})
	test "$BUILD_DEPENDENCIES" = "yes" -o "$BUILD_DEPENDENCIES" = $(pwd) && 
		{	PACKAGE_DEPENDENCIES="$2"; export BUILD_DEPENDENCIES=$(pwd); }
	AC_SUBST(PACKAGE_DEPENDENCIES) 
	AC_SUBST(BUILD_DEPENDENCIES) 
	AM_CONDITIONAL(BUILD_DEPENDENCIES, [test "$BUILD_DEPENDENCIES" = "yes"])
]) dnl AK_CHECK_BUILD_DEPENDENCIES


dnl detect for pointer size in bits
dnl AK_CHECK_PTRBITS(bits, [branch on matching], [branch on mismatching])
dnl e.g:
dnl    AK_CHECK_PTRBITS(64, [], AC_MSG_FAILURE([64-bit environment is necessary]))
AC_DEFUN([AK_CHECK_PTRBITS], [
	AC_MSG_CHECKING($1-bits pointer)
	AC_TRY_COMPILE(
		[], 
		[ typedef int statict_assert[sizeof(void*)*8==$1?1:-1]; ],
		[AC_MSG_RESULT(ok); $2], [AC_MSG_RESULT(failed); $3])
]) dnl AK_CHECK_PTRBITS

dnl check
AC_DEFUN([AK_ENSURE_64BIT], [
	AK_CHECK_PTRBITS(64, [], AC_MSG_FAILURE([64-bit environment is necessary]))
]) dnl AK_ENSURE_64BIT

dnl patch libtool to enabled 10 digits REVISION
AC_DEFUN([AK_PATCH_LIBTOOL], [
	AC_REQUIRE([AC_PROG_LIBTOOL])
	AC_MSG_NOTICE([patching libtool])
	if ! test -x ./libtool; then
		AC_MSG_FAILURE([can not find libtool])
	fi
	if ! sed -i.bak ['s/\(0|[][019|-].*\)) ;;/\1|[1-9][0-9][0-9][0-9][0-9][0-9]) ;;/'] libtool; then
		AC_MSG_FAILURE([can not patch libtool])
	fi
])

