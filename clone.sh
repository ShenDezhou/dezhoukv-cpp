#!/bin/bash
BASEDIR=$(dirname $0)
if [ -f configure.ac -o -f Makefile.am -o -f configure.in -o "$PWD" = "$HOME" -o "$PWD" = "/" ]; then
	echo >&2 "I can not clone to here."
	exit 1
fi
echo clone from $BASEDIR to $PWD
set -e 
(set -x;
cp $BASEDIR/configure.ac .
cp $BASEDIR/Makefile.am .
cp $BASEDIR/acsite.m4 .
cp $BASEDIR/*.mk .
cp $BASEDIR/GNUmakefile .
cp $BASEDIR/pkg.pc.in .
cp $BASEDIR/rpm.spec.in .
)
echo remove rpm support
(set -x;
rm rpm.spec.in
rm buildrpm.mk
)
echo patching
echo '
--- x/configure.ac	2012-05-22 12:46:00.000000000 +0800
+++ y/configure.ac	2012-07-26 13:58:45.000000000 +0800
@@ -3,10 +3,10 @@
 
 AC_PREREQ(2.57)
-AC_INIT(Makefile Demo, 0.10.0, kirbyzhou@sogou-inc.com, mkdemo)
+AC_INIT([Your Project Long Name], Version, MailAddress, ShortNameWithoutSpace)
 AC_CONFIG_SRCDIR([common.mk])
 AC_CONFIG_AUX_DIR(_aux)
 AM_INIT_AUTOMAKE([foreign])
 AC_CONFIG_HEADER([config-dummy.h])
-AC_PREFIX_DEFAULT(/opt/demo)
+AC_PREFIX_DEFAULT(/opt/sogou)
 #AM_MAINTAINER_MODE
 
@@ -97,5 +97,4 @@
 	$PACKAGE.pc:pkg.pc.in
 	Makefile
-	demo/Makefile
 ])
 AC_OUTPUT
--- x/Makefile.am	2011-08-24 20:31:00.000000000 +0800
+++ y/Makefile.am	2012-07-26 13:59:01.000000000 +0800
@@ -1,5 +1,5 @@
 include $(top_srcdir)/common.mk
 
-SUBDIRS=@PACKAGE_DEPENDENCIES@ demo
+SUBDIRS=@PACKAGE_DEPENDENCIES@
 # it must be one line, because of MODULE_ROOT_VARS sometimes can bu empty
 export @MODULE_ROOT_VARS@ BUILD_DEPENDENCIES
@@ -9,5 +9,5 @@
 pkgdata_DATA=SVNVERSION SVNINFO
 
-RPM_VARIANT=
-include $(top_srcdir)/buildrpm.mk
+#RPM_VARIANT=
+#include $(top_srcdir)/buildrpm.mk
' | patch -p1
echo done, OK
echo You SHOULD DO: svn add configure.ac Makefile.am acsite.m4 *.mk GNUmakefile pkg.pc.in rpm.spec.in

