#RPM_VARIANT=

SPECFILE=$(PACKAGE).spec
EXTRA_DIST+=$(SPECFILE) rpm.spec.in SVNVERSION SVNINFO
MOSTLYCLEANFILES+=$(SPECFILE)

AM_RPMBUILDFLAGS=--define="_topdir `pwd`/_rpmbuild" --define="_srcrpmdir $(CURDIR)"

rpm: dist
	rm -fr _rpmbuild/RPMS
	mkdir -p _rpmbuild/{SPECS,SOURCES,BUILD,RPMS}
	rm -f $(PACKAGE)$(RPM_VARIANT)-$(VERSION).$(SVNVERSION)-*.src.rpm
	rpmbuild $(AM_RPMBUILDFLAGS) $(RPMBUILDFLAGS) -ta $(distdir).tar.gz

srpm: dist
	mkdir -p _rpmbuild/{SPECS,SOURCES,BUILD,RPMS}
	rm -f $(PACKAGE)$(RPM_VARIANT)-$(VERSION).$(SVNVERSION)-*.src.rpm
	rpmbuild $(AM_RPMBUILDFLAGS) $(RPMBUILDFLAGS) --nodeps -ts $(distdir).tar.gz

$(SPECFILE): dist-svn
	( \
	  echo "%define __NAME $(PACKAGE)" ; \
	  if [ -z "$(RPM_VARIANT)" ]; then \
	    echo "%define __VARIANT %nil" ; \
	  else \
	    echo "%define __VARIANT $(RPM_VARIANT)" ; \
	  fi; \
	  echo "%define __LONG_NAME $(PACKAGE_NAME)" ; \
	  echo "%define __VERSION $(VERSION)" ; \
	  echo "%define __URL `$(DOLOCALE) svn info $(top_srcdir) | fgrep URL: | grep -o 'http://.*'`" ; \
	  echo "%define __SVNVERSION $(SVNVERSION)" \
	) > $@
	cat $(srcdir)/rpm.spec.in >> $@

fastrpm: all $(SPECFILE)
	mkdir -p _rpmbuild/{SPECS,SOURCES,BUILD,RPMS,SRPMS}
	ln -sf `pwd` _rpmbuild/BUILD/$(distdir)-BUILT
	rpmbuild $(AM_RPMBUILDFLAGS) $(RPMBUILDFLAGS) --define="skip_make 1" --define="skip_nosrc 0" -bb $(SPECFILE)

.PHONY: rpm srpm fastrpm

