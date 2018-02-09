SELFMAKEFILE = $(word 1, $(MAKEFILE_LIST))
_srcdir = $(dir $(SELFMAKEFILE))
srcdir = $(shell $(if $(_srcdir), cd $(_srcdir) && ) pwd)
top_srcdir = $(srcdir)
top_builddir = $(shell pwd)

NORMAL_TARGETS = all all-am am--refresh check \
	check-am ctags ctags-recursive \
	dist dist-all dist-bzip2 dist-gzip dist-shar dist-tarZ \
	dist-zip distcheck distdir \
	distuninstallcheck dvi dvi-am html html-am info info-am \
	install install-am install-data install-data-am install-exec \
	install-exec-am install-info install-info-am install-man \
	install-strip installcheck installcheck-am installdirs \
	installdirs-am uninstall uninstall-am uninstall-info-am \
	pdf pdf-am ps ps-am tags tags-recursive	CTAGS GTAGS \
	rpm fastrpm srpm

RECURSIVE_TARGETS = all-recursive check-recursive dvi-recursive \
	html-recursive info-recursive install-data-recursive \
	install-exec-recursive install-info-recursive \
	install-recursive installcheck-recursive installdirs-recursive \
	pdf-recursive ps-recursive uninstall-info-recursive \
	uninstall-recursive

CLEAN_TARGETS = clean clean-am clean-generic clean-libtool \
	clean-noinstPROGRAMS clean-recursive distclean distclean-am distclean-compile \
	distclean-generic distclean-hdr distclean-libtool \
	distclean-recursive distclean-tags distcleancheck \
	maintainer-clean maintainer-clean-am maintainer-clean-generic \
	maintainer-clean-recursive mostlyclean mostlyclean-compile \
	mostlyclean-generic mostlyclean-libtool mostlyclean-recursive  

ALL_TARGETS = $(NORMAL_TARGETS) $(RECURSIVE_TARGETS) $(CLEAN_TARGETS)

.PHONY: $(ALL_TARGETS) __FORCE

$(NORMAL_TARGETS) $(RECURSIVE_TARGETS) .DEFAULT: Makefile __FORCE
	$(MAKE) -f Makefile $@

__FORCE: ;

$(CLEAN_TARGETS):
	@test -f Makefile && $(MAKE) -f Makefile $@ || echo "No Makefile exist"

Makefile: $(top_srcdir)/Makefile.in $(top_srcdir)/configure
	@if [ ! -f ./Makefile ]; then \
		rootlist=`grep AK_CHECK_MODULE_ROOT configure.ac | sed 's/AK_CHECK_MODULE_ROOT(\([^[:space:]]*\),.*/\1/'`; \
		for i in $$rootlist; do \
			default=`fgrep "AK_CHECK_MODULE_ROOT($$i," configure.ac | sed 's/AK_CHECK_MODULE_ROOT([^,]*,[^,]*,\([^,]*\)\(,.*\)\?)/\1/'`; \
			eval echo $$i='$$'{$$i} [default = $$default \\\| $$default.default]; \
		done; \
		for i in $$rootlist; do if test -z "`eval echo '$$'{$$i}`"; then \
			default=`fgrep "AK_CHECK_MODULE_ROOT($$i," configure.ac | sed 's/AK_CHECK_MODULE_ROOT([^,]*,[^,]*,\([^,]*\)\(,.*\)\?)/\1/'`; \
			test -d $$default && continue; \
			test -d $$default.default && continue; \
			echo $$i is empty and no default; \
			sleep 5; \
		fi done;\
		$(top_srcdir)/configure $(MAKEOVERRIDES); \
	fi

$(top_srcdir)/Makefile.in $(top_srcdir)/configure:
	cd $(top_srcdir) && autoreconf -isv

.NOTPARALLEL:

