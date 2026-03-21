PWD := $(shell pwd)

COMPANY_NAME ?= ONLYOFFICE
PRODUCT_NAME ?= Desktop Editors
PRODUCT_NAME_SHORT ?= $(subst $(empty) $(empty),,$(PRODUCT_NAME))

COMPANY_NAME_LOW = $(shell echo $(COMPANY_NAME) | tr A-Z a-z)
PRODUCT_NAME_LOW = $(shell echo $(PRODUCT_NAME_SHORT) | tr A-Z a-z)

PUBLISHER_NAME ?= Ascensio System SIA
PUBLISHER_URL ?= http://onlyoffice.com
SUPPORT_URL ?= http://support.onlyoffice.com
SUPPORT_MAIL ?= support@onlyoffice.com
SCHEME_HANDLER ?= oo-office

PRODUCT_VERSION ?= 0.0.0
BUILD_NUMBER ?= 0

BRANDING_DIR ?= .

PACKAGE_NAME := $(COMPANY_NAME_LOW)-$(PRODUCT_NAME_LOW)
PACKAGE_OPENSOURCE := $(PACKAGE_NAME)
PACKAGE_COMMERCIAL := $(PACKAGE_NAME)-enterprise
PACKAGE_EDITION ?= opensource
ifeq ($(PACKAGE_EDITION), commercial)
PACKAGE_NAME := $(PACKAGE_COMMERCIAL)
endif
PACKAGE_VERSION := $(PRODUCT_VERSION)-$(BUILD_NUMBER)

UNAME_M ?= $(shell uname -m)
ifeq ($(UNAME_M),x86_64)
	RPM_ARCH = x86_64
	DEB_ARCH = amd64
	TAR_ARCH = x86_64
	PLATFORM = linux_64
endif
ifneq ($(filter aarch%,$(UNAME_M)),)
	RPM_ARCH = aarch64
	DEB_ARCH = arm64
	TAR_ARCH = aarch64
	PLATFORM = linux_arm64
endif

RPM_BUILD_DIR = rpm/build
RPM_SUSE_BUILD_DIR = rpm-suse/build

RPM_PACKAGE_DIR = $(RPM_BUILD_DIR)/RPMS/$(RPM_ARCH)
RPM_SUSE_PACKAGE_DIR = $(RPM_SUSE_BUILD_DIR)/RPMS/$(RPM_ARCH)

DEB = deb/$(PACKAGE_NAME)_$(PACKAGE_VERSION)$(DEB_RELEASE_SUFFIX)_$(DEB_ARCH).deb
RPM = $(RPM_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)$(RPM_RELEASE_SUFFIX).$(RPM_ARCH).rpm
RPM_SUSE = $(RPM_SUSE_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)$(RPM_SUSE_RELEASE_SUFFIX).$(RPM_ARCH).rpm
TAR = tar/$(PACKAGE_NAME)-$(PACKAGE_VERSION)$(TAR_RELEASE_SUFFIX)-$(TAR_ARCH).tar.xz
TAR_HELP = tar/$(PACKAGE_NAME)-help-$(PACKAGE_VERSION)$(TAR_RELEASE_SUFFIX)-any.tar.xz

NEWS_RPM = build/main/usr/share/doc/$(PACKAGE_NAME)/NEWS
CHANGELOG_RPM = build/main/usr/share/doc/$(PACKAGE_NAME)/ChangeLog
CHANGELOG_DEB = deb/build/debian/changelog

DESKTOPEDITORS_PREFIX := $(COMPANY_NAME_LOW)/$(PRODUCT_NAME_LOW)
DESKTOPEDITORS_EXEC := $(PACKAGE_NAME)

BIN_FILES += build/main/usr/bin/$(DESKTOPEDITORS_EXEC)
DESKTOP_FILES += build/main/usr/share/applications/$(DESKTOPEDITORS_EXEC).desktop
LINUX_DEPS += $(BIN_FILES)
LINUX_DEPS += $(DESKTOP_FILES)
LINUX_DEPS += $(CHANGELOG_RPM)
LINUX_DEPS += $(patsubst $(BRANDING_DIR)/common/linux/icons/%.png, \
	build/main/usr/share/icons/hicolor/%/apps/$(DESKTOPEDITORS_EXEC).png, \
	$(wildcard $(BRANDING_DIR)/common/linux/icons/*.png))
LINUX_DEPS += build/main/usr/share/licenses/$(PACKAGE_NAME)/LICENSE
LINUX_DEPS += build/main/usr/share/licenses/$(PACKAGE_NAME)/3DPARTYLICENSE

DEB_DEPS += $(CHANGELOG_DEB)
DEB_DEPS += deb/build/debian/compat
DEB_DEPS += deb/build/debian/control
DEB_DEPS += deb/build/debian/copyright
DEB_DEPS += deb/build/debian/postrm
DEB_DEPS += deb/build/debian/rules
DEB_DEPS += deb/build/debian/source/format
DEB_DEPS += deb/build/debian/$(PACKAGE_NAME).dirs
DEB_DEPS += deb/build/debian/$(PACKAGE_NAME).install
DEB_DEPS += deb/build/debian/$(PACKAGE_NAME).links
DEB_DEPS += deb/build/debian/$(PACKAGE_NAME)-help.install
DEB_DEPS += deb/build/package.config

SOURCE_DIR := ../../build_tools/out/$(PLATFORM)/$(COMPANY_NAME_LOW)

M4_PARAMS += -D M4_COMPANY_NAME='$(COMPANY_NAME)'
M4_PARAMS += -D M4_COMPANY_NAME_LOW='$(COMPANY_NAME_LOW)'
M4_PARAMS += -D M4_PRODUCT_NAME='$(PRODUCT_NAME)'
M4_PARAMS += -D M4_PUBLISHER_NAME='$(PUBLISHER_NAME)'
M4_PARAMS += -D M4_PUBLISHER_URL='$(PUBLISHER_URL)'
M4_PARAMS += -D M4_SUPPORT_MAIL='$(SUPPORT_MAIL)'
M4_PARAMS += -D M4_SUPPORT_URL='$(SUPPORT_URL)'
M4_PARAMS += -D M4_PACKAGE_NAME='$(PACKAGE_NAME)'
M4_PARAMS += -D M4_PACKAGE_OPENSOURCE='$(PACKAGE_OPENSOURCE)'
M4_PARAMS += -D M4_PACKAGE_COMMERCIAL='$(PACKAGE_COMMERCIAL)'
M4_PARAMS += -D M4_PACKAGE_EDITION='$(PACKAGE_EDITION)'
M4_PARAMS += -D M4_DESKTOPEDITORS_PREFIX='$(DESKTOPEDITORS_PREFIX)'
M4_PARAMS += -D M4_DESKTOPEDITORS_EXEC='$(DESKTOPEDITORS_EXEC)'
M4_PARAMS += -D M4_SCHEME_HANDLER='$(SCHEME_HANDLER)'
M4_PARAMS += -D M4_DEB_ARCH='$(DEB_ARCH)'
M4_PARAMS += -D M4_CURRENT_YEAR='$(shell date +"%Y")'

M4_DEB_PARAMS = $(M4_PARAMS)
M4_DEB_PARAMS += -D M4_PACKAGE_VERSION='$(PACKAGE_VERSION)$(DEB_RELEASE_SUFFIX)'

XZ_OPT ?= -9 -T$(shell nproc)
RPMBUILD_BINARY_PAYLOAD ?= w9T$(shell nproc).xzdio

RPM_PARAMS += --define '_company_name $(COMPANY_NAME)'
RPM_PARAMS += --define '_product_name $(PRODUCT_NAME)'
RPM_PARAMS += --define '_publisher_name $(PUBLISHER_NAME)'
RPM_PARAMS += --define '_publisher_url $(PUBLISHER_URL)'
RPM_PARAMS += --define '_support_mail $(SUPPORT_MAIL)'
RPM_PARAMS += --define '_package_name $(PACKAGE_NAME)'
RPM_PARAMS += --define '_package_opensource $(PACKAGE_OPENSOURCE)'
RPM_PARAMS += --define '_package_commercial $(PACKAGE_COMMERCIAL)'
RPM_PARAMS += --define '_package_edition $(PACKAGE_EDITION)'
RPM_PARAMS += --define '_product_version $(PRODUCT_VERSION)'
RPM_PARAMS += --define '_desktopeditors_prefix $(DESKTOPEDITORS_PREFIX)'
RPM_PARAMS += --define '_desktopeditors_exec $(DESKTOPEDITORS_EXEC)'
RPM_PARAMS += --define '_scheme_handler $(SCHEME_HANDLER)'
RPM_PARAMS += --define '_binary_payload $(RPMBUILD_BINARY_PAYLOAD)'
ifeq ($(shell rpm --eval %_arch):$(RPM_ARCH),x86_64:aarch64)
	RPM_PARAMS += --define '__strip /usr/bin/aarch64-linux-gnu-strip'
endif

CHANGES_DIR := $(BRANDING_DIR)/common/changelog
CHANGES_HTML := changes.html
CHANGES_VERSIONS := $(shell echo $(PRODUCT_VERSION); { ls -1 $(CHANGES_DIR) \
	| grep -E '^[0-9]+' | grep -v $(PRODUCT_VERSION) | sort -Vur; })

HELP_GLOB := opt/$(DESKTOPEDITORS_PREFIX)/editors/web-apps/apps/*/main/resources/help

-include $(BRANDING_DIR)/branding.mk

all : deb rpm rpm-suse tar
deb : $(DEB)
rpm : $(RPM)
rpm-suse : $(RPM_SUSE)
tar : $(TAR) $(TAR_HELP)

clean :
	rm -rf \
		build \
		deb*/build \
		deb*/*.buildinfo \
		deb*/*.changes \
		deb*/*.ddeb \
		deb*/*.deb \
		rpm*/build \
		rpm*/$(PACKAGE_NAME).spec \
		tar \
		$(CHANGES_DIR)/*/changes_??? \
		$(CHANGES_DIR)/*/news_???

build : $(LINUX_DEPS)
	mkdir -p $@/main/opt/$(DESKTOPEDITORS_PREFIX)
	cp -rft $@/main/opt/$(DESKTOPEDITORS_PREFIX) $(SOURCE_DIR)/desktopeditors/*
	chmod 755 $@/main/opt/$(DESKTOPEDITORS_PREFIX)/DesktopEditors

ifneq ($(PACKAGE_EDITION), commercial)
	cp -f $(BRANDING_DIR)/common/license/$(PACKAGE_EDITION)/LICENSE.txt \
		$@/main/opt/$(DESKTOPEDITORS_PREFIX)/LICENSE.txt
else
	cp -f $(BRANDING_DIR)/common/license/$(PACKAGE_EDITION)/LICENSE.txt \
		$@/main/opt/$(DESKTOPEDITORS_PREFIX)/EULA.txt
endif

	cd $@; \
	for path in main/$(HELP_GLOB); do \
		path_new=$$(echo $$path | sed 's|^main/|help/|'); \
		mkdir -p $$path_new; \
		mv -fT $$path $$path_new; \
	done

ifdef MEDIAVIEWER_PREFIX
	mkdir -p $@/main/opt/$(MEDIAVIEWER_PREFIX)
	cp -rft $@/main/opt/$(MEDIAVIEWER_PREFIX) $(SOURCE_DIR)/mediaviewer/*
	chmod 755 $@/main/opt/$(MEDIAVIEWER_PREFIX)/ImageViewer
	chmod 755 $@/main/opt/$(MEDIAVIEWER_PREFIX)/VideoPlayer
endif

$(RPM) : build rpm/$(PACKAGE_NAME).spec
	cd rpm && rpmbuild -bb \
		--define "_topdir $(PWD)/$(RPM_BUILD_DIR)" \
		--define "_build_number $(BUILD_NUMBER)$(RPM_RELEASE_SUFFIX)" \
		--define "_package_arch $(RPM_ARCH)" \
		--target $(RPM_ARCH) \
		$(RPM_PARAMS) \
		$(PACKAGE_NAME).spec

build/main/usr/share/icons/hicolor/%/apps/$(DESKTOPEDITORS_EXEC).png : $(BRANDING_DIR)/common/linux/icons/%.png
	install -m 644 -D $< $@

build/main/usr/share/licenses/$(PACKAGE_NAME)/LICENSE : $(BRANDING_DIR)/common/license/$(PACKAGE_EDITION)/LICENSE.txt
build/main/usr/share/licenses/$(PACKAGE_NAME)/3DPARTYLICENSE : $(BRANDING_DIR)/common/license/3dparty/3DPARTYLICENSE

build/main/usr/share/licenses/% :
	install -m 644 -D $< $@

deb/build/debian/% : deb/template/%
	mkdir -pv $(@D) && cp -fv $< $@

deb/build/debian/% : deb/template/%.m4
	mkdir -pv $(@D) && m4 $(M4_DEB_PARAMS) $< > $@

$(CHANGELOG_DEB) :
	mkdir -pv $(@D)
	for v in $(CHANGES_VERSIONS); do \
		if [ -f $(CHANGES_DIR)/$$v/$(CHANGES_HTML) ]; then \
			date=$$(sed -n $(CHANGES_DIR)/$$v/$(CHANGES_HTML) \
				-e 's/{{DATE}}/'$$(date -I)'/' \
				-e 's/.*<span class="releasedate"> - \(.*\)<\/span>.*/\1/p' \
				| date -R -f -); \
		else \
			date=$$(date -R); \
		fi; \
		echo "VERSION: $$v; DATE: $$date"; \
		mkdir -pv $(CHANGES_DIR)/$$v; \
		if [ -f $(CHANGES_DIR)/$$v/$(CHANGES_HTML) ]; then \
			sed $(CHANGES_DIR)/$$v/$(CHANGES_HTML) \
				-e '/DOCTYPE/,/div class/d' \
				-e '/div class/,/html/d' \
				-e '/<h1>/d' \
				-e '/<ul>/d' \
				-e '/<\/ul>/d' \
				-e 's/<\/li>//g' \
				-e 's/<\/h4>/:/g' \
				-e 's/<\/h2>/ ]/g' \
				-e 's/<code>//g' \
				-e 's/<\/code>//g' \
				-e 's/ \{,\}<li>/    - /g' \
				-e 's/ \{,\}<h4>/  * /g' \
				-e 's/ \{,\}<h2>/\n  [ /g' \
				-e '/^$$/d' \
				| sed '1{/^$$/d}' \
				> $(CHANGES_DIR)/$$v/changes_deb; \
		else \
			echo "  * $(PRODUCT_VERSION)" > $(CHANGES_DIR)/$$v/changes_deb; \
		fi; \
		if [ $$v = $(PRODUCT_VERSION) ]; then \
			m4 $(M4_DEB_PARAMS) -D M4_DATE="$$date" \
				deb/template/changelog.m4 \
				| sed '2r '$(CHANGES_DIR)/$$v/changes_deb >> $@; \
		else \
			m4 $(M4_DEB_PARAMS) -D M4_PACKAGE_VERSION=$$v -D M4_DATE="$$date" \
				deb/template/changelog.m4 \
				| sed '2r '$(CHANGES_DIR)/$$v/changes_deb >> $@; \
		fi; \
	done

$(NEWS_RPM) :
	mkdir -pv $(@D)
	for v in $(CHANGES_VERSIONS); do \
		if [ -f $(CHANGES_DIR)/$$v/$(CHANGES_HTML) ]; then \
			date=$$(sed -n $(CHANGES_DIR)/$$v/$(CHANGES_HTML) \
				-e 's/{{DATE}}/'$$(date -I)'/' \
				-e 's/.*<span class="releasedate"> - \(.*\)<\/span>.*/\1/p' \
				| date -I -f -); \
		else \
			date=$$(date -I); \
		fi; \
		echo "VERSION: $$v; DATE: $$date"; \
		mkdir -pv $(CHANGES_DIR)/$$v; \
		if [ -f $(CHANGES_DIR)/$$v/$(CHANGES_HTML) ]; then \
			sed $(CHANGES_DIR)/$$v/$(CHANGES_HTML) \
				-e '/DOCTYPE/,/div class/d' \
				-e '/div class/,/html/d' \
				-e 's/<\/li>//g' \
				-e 's/<\/h4>/:/g' \
				-e 's/<\/h2>//g' \
				-e 's/<code>//g' \
				-e 's/<\/code>//g' \
				-e 's/ \{,\}<li>/  - /g' \
				-e 's/ \{,\}<h4>/- /g' \
				-e 's/ \{,\}<h2>//g' \
				-e '/<ul>/d' \
				-e '/<\/ul>/d' \
				-e '/<h1>/d' \
				-e '/^$$/d' \
				> $(CHANGES_DIR)/$$v/news_rpm; \
		else \
			echo "$(PRODUCT_VERSION)" > $(CHANGES_DIR)/$$v/news_rpm; \
		fi; \
		m4 $(M4_PARAMS) -D M4_VERSION=$$v -D M4_DATE_SHORT="$$date" \
			common/linux/NEWS.m4 \
			| sed '2r '$(CHANGES_DIR)/$$v/news_rpm >> $@; \
	done

$(CHANGELOG_RPM) : $(NEWS_RPM)
	mkdir -pv $(@D)
	for v in $(CHANGES_VERSIONS); do \
		if [ -f $(CHANGES_DIR)/$$v/$(CHANGES_HTML) ]; then \
			date=$$(sed -n $(CHANGES_DIR)/$$v/$(CHANGES_HTML) \
				-e 's/{{DATE}}/'$$(date -I)'/' \
				-e 's/.*<span class="releasedate"> - \(.*\)<\/span>.*/\1/p' \
				| date +'%a %b %d %Y' -f -); \
		else \
			date=$$(date +'%a %b %d %Y'); \
		fi; \
		echo "VERSION: $$v; DATE: $$date"; \
		sed $(CHANGES_DIR)/$$v/news_rpm \
			-e 's/^    - /      - /' \
			-e 's/^  - /    - /' \
			-e 's/^- /  - /' \
			-e 's/^[^ *]/- &/' \
			> $(CHANGES_DIR)/$$v/changes_rpm; \
		m4 $(M4_PARAMS) -D M4_VERSION=$$v -D M4_DATE="$$date" \
			common/linux/ChangeLog.m4 \
			| sed '1r '$(CHANGES_DIR)/$$v/changes_rpm >> $@; \
	done

deb/build/debian/$(PACKAGE_NAME).% : deb/template/package.%.m4
	mkdir -pv $(@D) && m4 $(M4_DEB_PARAMS) $< > $@

deb/build/debian/$(PACKAGE_NAME)-help.% : deb/template/package-help.%.m4
	mkdir -pv $(@D) && m4 $(M4_DEB_PARAMS) $< > $@

deb/build/package.config :
	mkdir -p $(@D) && echo "package = deb" > $@

$(DEB) : build $(DEB_DEPS)
	+cd deb/build && dpkg-buildpackage -b -uc -us -a$(DEB_ARCH)

$(RPM_SUSE) : build rpm-suse/$(PACKAGE_NAME).spec
	cd rpm-suse && rpmbuild -bb \
		--define "_topdir $(PWD)/$(RPM_SUSE_BUILD_DIR)" \
		--define "_build_number $(BUILD_NUMBER)$(RPM_SUSE_RELEASE_SUFFIX)" \
		--define "_package_arch $(RPM_ARCH)" \
		--target $(RPM_ARCH) \
		$(RPM_PARAMS) \
		$(PACKAGE_NAME).spec

$(TAR) : build
	mkdir -p tar
	XZ_OPT="$(XZ_OPT)" tar --create --xz \
		--owner=root --group=root \
		--directory=build/main --file=$@ \
		$(patsubst build/main/%,%,$(wildcard build/main/*))

$(TAR_HELP) : build
	mkdir -p tar
	XZ_OPT="$(XZ_OPT)" tar --create --xz \
		--owner=root --group=root \
		--directory=build/help --file=$@ \
		$(patsubst build/help/%,%,$(wildcard build/help/*))

build/main/usr/bin/$(DESKTOPEDITORS_EXEC) : common/linux/desktopeditors.sh.m4
build/main/usr/share/applications/$(DESKTOPEDITORS_EXEC).desktop : common/linux/desktopeditors.desktop.m4

$(BIN_FILES) $(DESKTOP_FILES) :
	mkdir -p $(@D) && m4 -I "$(BRANDING_DIR)/common/linux" $(M4_PARAMS) $< > $@

% : %.m4
	m4 $(M4_PARAMS)	$< > $@

%/$(PACKAGE_NAME).spec : %/package.spec 
	cp $< $@

.PHONY : all clean deb rpm rpm-suse tar
