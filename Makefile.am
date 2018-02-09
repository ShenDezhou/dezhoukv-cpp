# http://svn.sogou-inc.com/svn/arch/public/MakefileDemo
#

include $(top_srcdir)/common.mk

SUBDIRS=@PACKAGE_DEPENDENCIES@ dezhoukv 
# it must be one line, because of MODULE_ROOT_VARS sometimes can bu empty
export @MODULE_ROOT_VARS@ BUILD_DEPENDENCIES

EXTRA_DIST+=GNUmakefile

pkgdata_DATA=SVNVERSION SVNINFO

RPM_VARIANT=
include $(top_srcdir)/buildrpm.mk

nobase_include_HEADERS= \
	#demo/stdafx.h

