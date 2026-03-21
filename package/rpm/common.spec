Name: %{_package_name}
Version: %{_product_version}
Release: %{_build_number}
Summary: Desktop editors for text docs, spreadsheets, presentations, PDFs, and PDF forms.
%if "%{_package_edition}" == "commercial"
License: Proprietary
%else
License: AGPLv3
%endif
Group: Applications/Office
URL: %{_publisher_url}
Vendor: %{_publisher_name}
Packager: %{_publisher_name} %{_support_mail}
AutoReq: no
AutoProv: no
Provides: %{_package_opensource}
%if "%{_package_edition}" == "commercial"
Conflicts: %{_package_opensource}
Obsoletes: %{_package_opensource}
%else
Conflicts: %{_package_commercial}
Obsoletes: %{_package_commercial}
%endif
Suggests: %{_package_opensource}-help

%description
Open-source office suite pack that comprises all the tools you need to
work offline with documents, spreadsheets, presentations, PDFs, and PDF forms.

%if "%{_company_name}" == "ONLYOFFICE"
%package help
Summary: Offline help for %{_company_name} %{_product_name}
BuildArch: noarch
Requires: %{_package_opensource}
Enhances: %{_package_opensource}

%description help
This package contains offline help files.
%endif

%prep
rm -rf "%{buildroot}"

%build

%install
BUILD_DIR=../../../build
BIN_DIR=%{buildroot}%{_bindir}
DATA_DIR=%{buildroot}%{_datadir}
OPT_DIR=%{buildroot}/opt

mkdir -p %{buildroot}
cp -rt %{buildroot}/ $BUILD_DIR/main/*
echo "package = rpm" > $OPT_DIR/%{_desktopeditors_prefix}/converter/package.config

%if "%{_company_name}" == "ONLYOFFICE"
ln -srf $BIN_DIR/%{_desktopeditors_exec} $BIN_DIR/desktopeditors
cp -rt %{buildroot}/ $BUILD_DIR/help/*
%else
ETC_DIR=%{buildroot}%{_sysconfdir}
mkdir -p $ETC_DIR/%{_package_name}
%endif

%clean
rm -rf "%{buildroot}"

%files
%attr(-, root, root) /opt/*
%attr(-, root, root) %{_datadir}/applications/*
%attr(-, root, root) %{_datadir}/doc/*
%attr(-, root, root) %{_datadir}/icons/*
%attr(-, root, root) %{_datadir}/licenses/*
%attr(755, root, root) %{_bindir}/%{_desktopeditors_exec}
%if "%{_company_name}" == "ONLYOFFICE"
%attr(-, root, root) %{_bindir}/desktopeditors
%exclude /opt/%{_desktopeditors_prefix}/editors/web-apps/apps/*/main/resources/help
%else
%attr(755, root, root) %{_bindir}/%{_imageviewer_exec}
%attr(755, root, root) %{_bindir}/%{_videoplayer_exec}
%attr(-, root, root) %{_bindir}/%{_package_name}
%attr(777, root, root) %{_sysconfdir}/%{_package_name}
%endif

%if "%{_company_name}" == "ONLYOFFICE"
%files help
%defattr(-, root, root, -)
/opt/%{_desktopeditors_prefix}/editors/web-apps/apps/*/main/resources/help
%endif

%postun
rm -f /usr/share/mime/packages/onlyoffice-docxf.xml
rm -f /usr/share/mime/packages/onlyoffice-oform.xml

%changelog

%include ../build/main/usr/share/doc/%{_package_name}/ChangeLog
