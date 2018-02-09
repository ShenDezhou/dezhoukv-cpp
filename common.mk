# http://svn.sogou-inc.com/svn/arch/public/MakefileDemo
#

include $(top_srcdir)/common-rules.mk

collectlib_DIR=$(top_builddir)/_lib
collectbin_DIR=$(top_builddir)/_bin
collectinclude_DIR=$(top_builddir)/_include
include $(top_srcdir)/collect.mk

