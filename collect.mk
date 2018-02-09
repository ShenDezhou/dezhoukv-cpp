# collect built files into one place
# You should add the follwing lines into your 'common.mk'
#
#   collectlib_DIR=$(top_builddir)/_lib
#   collectbin_DIR=$(top_builddir)/_bin
#   collectinclude_DIR=$(top_builddir)/_include
#   include $(top_srcdir)/collect.mk

collectLIBRARIES=$(lib_LIBRARIES) $(pkglib_LIBRARIES)
collectLTLIBRARIES=$(lib_LTLIBRARIES) $(pkglib_LTLIBRARIES)
collectPROGRAMS=$(bin_PROGRAMS) $(sbin_PROGRAM) $(pkglib_PROGRAMS) $(libexec_PROGRAMS) \
	$(bin_SCRIPTS) $(sbin_SCRIPTS) $(libexec_SCRIPTS)
collectHEADERS=$(include_HEADERS) $(pkginclude_HEADERS) $(nobase_include_HEADERS) $(nobase_pkginclude_HEADERS)
collectHEADERS_T=\
	$(addprefix $(collectinclude_DIR)/, $(notdir $(include_HEADERS))) \
	$(addprefix $(collectinclude_DIR)/$(PACKAGE)/, $(notdir $(pkginclude_HEADERS))) \
	$(addprefix $(collectinclude_DIR)/, $(nobase_include_HEADERS)) \
	$(addprefix $(collectinclude_DIR)/$(PACKAGE)/, $(nobase_pkginclude_HEADERS))

all-local: $(collectLIBRARIES) $(collectLTLIBRARIES) $(collectPROGRAMS) $(collectHEADERS)
	test -z "$(collectbin_DIR)" || $(MAKE) collectbin-local
	test -z "$(collectlib_DIR)" || $(MAKE) collectlib-local
	test -z "$(collectinclude_DIR)" || $(MAKE) collectinclude-local

collectlib-local: $(addprefix $(collectlib_DIR)/, $(collectLIBRARIES) $(collectLTLIBRARIES))
collectbin-local: $(addprefix $(collectbin_DIR)/, $(collectPROGRAMS))
collectinclude-local: $(collectHEADERS_T)

define DoLinkTemplate
#args:(target, source)
$(1): $(2)
	mkdir -p "$$(dir $(1))"
	ln -sf "$$(abspath $(2))" "$(strip $(1))"
endef

$(foreach t, $(include_HEADERS), $(eval $(call DoLinkTemplate, $(collectinclude_DIR)/$(notdir $(t)), $(t))))
$(foreach t, $(pkginclude_HEADERS), $(eval $(call DoLinkTemplate, $(collectinclude_DIR)/$(PACKAGE)/$(notdir $(t)), $(t))))
$(foreach t, $(nobase_include_HEADERS), $(eval $(call DoLinkTemplate, $(collectinclude_DIR)/$(t), $(t))))
$(foreach t, $(nobase_pkginclude_HEADERS), $(eval $(call DoLinkTemplate, $(collectinclude_DIR)/$(PACKAGE)/$(t), $(t))))
	
$(collectlib_DIR)/%.a: %.a
	mkdir -p $(collectlib_DIR)
	ln -sf --target-directory=$(collectlib_DIR)/ $(addprefix $(CURDIR)/, $<)

$(collectlib_DIR)/%.la: %.la
	mkdir -p $(collectlib_DIR)/.libs
	rm -f $(collectlib_DIR)/$(<:%.la=%.so.*) $(collectlib_DIR)/$(<:%.la=%.so)
	@rm -f $(collectlib_DIR)/.libs/$(<:%.la=%.so.*) $(collectlib_DIR)/.libs/$(<:%.la=%.so)
if ENABLE_STATIC
	ln -sf $(CURDIR)/.libs/$(<:%.la=%.a) $(collectlib_DIR)/
	@ln -sf $(CURDIR)/.libs/$(<:%.la=%.a) $(collectlib_DIR)/.libs/
endif
	ln -sf $(CURDIR)/.libs/$(<:%.la=%.so*) $(collectlib_DIR)/
	@ln -sf $(CURDIR)/.libs/$(<:%.la=%.so*) $(collectlib_DIR)/.libs/
	install $(CURDIR)/$< $(collectlib_DIR)/

$(collectbin_DIR)/%: %
	mkdir -p $(collectbin_DIR)
	ln -sf --target-directory=$(collectbin_DIR)/ $(addprefix $(CURDIR)/,$<)

MOSTLYCLEANFILES+=$(foreach t, $(collectLTLIBRARIES), \
	$(collectlib_DIR)/$(t) $(collectlib_DIR)/$(t:%.la=%.so*) $(collectlib_DIR)/$(t:%.la=%.a) \
	$(collectlib_DIR)/.libs/$(t:%.la=%.so*)	$(collectlib_DIR)/.libs/$(t:%.la=%.a))
MOSTLYCLEANFILES+=$(foreach t, $(collectLIBRARIES), $(collectlib_DIR)/$(t))
MOSTLYCLEANFILES+=$(foreach t, $(collectPROGRAMS), $(collectbin_DIR)/$(t))
MOSTLYCLEANFILES+=$(collectHEADERS_T)

.PHONY: mkdir-collect

all-am: mkdir-collect

mkdir-collect: $(collectlib_DIR) $(collectbin_DIR) $(collectinclude_DIR)

$(collectlib_DIR) $(collectbin_DIR) $(collectinclude_DIR):
	mkdir -p $@

mostlyclean-local:
	@if [ $(srcdir) == $(top_srcdir) ]; then rm -fr $(collectlib_DIR) $(collectbin_DIR) $(collectinclude_DIR); fi

