MOSTLYCLEANFILES=
BUILT_SOURCES=

CFLAGS+=$(if $(DEBUG), -g -O0)
CXXFLAGS+=$(if $(DEBUG), -g -O0)

CPPFLAGS+=$(if $(OPTIMIZE), -DNDEBUG)
CFLAGS+=$(if $(OPTIMIZE), -O3)
CXXFLAGS+=$(if $(OPTIMIZE), -O3)

CFLAGS+=$(if $(PROFILE), -pg)
CXXFLAGS+=$(if $(PROFILE), -pg)

AM_CPPFLAGS=-D'SVNVERSION="$(SVNVERSION)"'
AM_LDFLAGS=-Wl,-no-undefined
AM_CFLAGS=-Werror-implicit-function-declaration
AM_CXXFLAGS=
AM_MAKEFLAGS=

DOLOCALE=LC_ALL= LC_CTYPE=zh_CN.gbk LC_MESSAGES=en_US

SVNVERSION:=$(strip $(subst exported, 0, $(shell \
	if [ -d $(top_srcdir)/.svn ]; then \
		if $(DOLOCALE) svnversion -nc $(top_srcdir) | fgrep 'M' &> /dev/null; then \
			$(DOLOCALE) svnversion -n $(top_srcdir) | sed 's/^.*://' | tr -cd '[0-9]'; \
		else \
			$(DOLOCALE) svnversion -nc $(top_srcdir)| sed 's/^.*://' | tr -cd '[0-9]'; \
		fi; \
	else \
		cat $(top_srcdir)/SVNVERSION | sed 's/^.*://' | tr -cd '[0-9]'; \
	fi \
	) ) )
CONFIGURE_DEPENDENCIES=$(subst @@@,,$(subst CONFIGURE_DEPENDENCIES,@,@CONFIGURE_DEPENDENCIES@))
CONFIGURE_DEPENDENCIES+=$(top_srcdir)/acsite.m4

distdir=$(PACKAGE)-$(VERSION).$(SVNVERSION)

EXTRA_DIST=

SVNVERSION SVNINFO: dist-svn

dist-svn:
	if [ $(top_srcdir) = $(srcdir) -a -d $(top_srcdir)/.svn ]; then $(MAKE) dist-svn-doit; fi;

dist-svn-doit:
	if $(DOLOCALE) svnversion -nc $(top_srcdir) | fgrep 'M' &> /dev/null; then \
		$(DOLOCALE) svnversion -nc $(top_srcdir) > SVNVERSION; \
		echo -n ':' >> SVNVERSION; \
		$(DOLOCALE) svnversion -n $(top_srcdir) | sed 's/^.*://' | tr -cd '[0-9]' >> SVNVERSION; \
	else \
		$(DOLOCALE) svnversion -nc $(top_srcdir) > SVNVERSION; \
	fi; \
	$(DOLOCALE) svn info -R $(top_srcdir) > SVNINFO;

clean-local:
	if [ -d $(srcdir)/.svn ]; then rm -f SVNVERSION SVNINFO; fi

.PHONY: dist-svn dist-svn-doit

