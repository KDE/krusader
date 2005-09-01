Name:		krusader
Version:	1.60.0
Release:	3%{?dist}
Summary:	An advanced twin-panel (commander-style) file-manager for KDE

Group:		Applications/File
License:	GPL
URL:		http://krusader.sourceforge.net/
Source0:	http://prdownloads.sourceforge.net/%{name}/%{name}-%{version}.tar.gz
Patch0:		krusader-1.60.0-desktop.patch
Patch1:		krusader-1.60.0-gcc4.patch
Patch2:		krusader-1.60.0-newline.patch
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	kdelibs-devel >= 3.3.0 qt-devel >= 3.3.0 arts-devel >= 1.3.0
BuildRequires:	kdebase-devel >= 3.3.0 kdebindings-devel >= 3.3.0 gamin-devel
BuildRequires:	libpng-devel libselinux-devel automake desktop-file-utils
BuildRequires:	gettext

%description
Krusader is an advanced twin-panel (commander-style) file-manager for KDE
(similar to Midnight or Total Commander) but with many extras.
It provides all the file-management features you could possibly want.
Plus: extensive archive handling, mounted filesystem support, FTP, advanced
search module, viewer/editor, directory synchronisation, file content
comparisons, powerful batch renaming and much much more.
It supports the following archive formats: tar, zip, bzip2, gzip, rar, ace,
arj and rpm and can handle other KIOSlaves such as smb:// or fish://
It is (almost) completely customizable, very user friendly, fast and looks
great on your desktop! :-)

You should give it a try.

%prep
%setup -q
%patch0 -p1
%patch1 -p1
%patch2 -p1

%build
unset QTDIR || : ; . /etc/profile.d/qt.sh
export QTLIB=${QTDIR}/lib QTINC=${QTDIR}/include

%configure \
	--disable-rpath

make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

desktop-file-install --vendor fedora --delete-original \
	--dir $RPM_BUILD_ROOT%{_datadir}/applications \
	--add-category X-Fedora \
	$RPM_BUILD_ROOT%{_datadir}/applnk/Applications/*.desktop

# Workaround for some wicked install bug
rm -rf $RPM_BUILD_ROOT%{_tmppath}

# Make symlink relative
pushd $RPM_BUILD_ROOT%{_datadir}/doc/HTML/en/krusader/
ln -s -f ../common
popd

%find_lang %{name}

%clean
rm -rf $RPM_BUILD_ROOT

%files -f %name.lang
%defattr(-, root, root)
%doc doc/actions_tutorial.txt AUTHORS ChangeLog COPYING CVSNEWS FAQ README TODO
%{_bindir}/krusader
%{_libdir}/kde3/kio_iso.*
%{_libdir}/kde3/kio_krarc.*
%{_datadir}/applications/*krusader*.desktop
%{_datadir}/apps/konqueror/servicemenus/isoservice.desktop
%{_datadir}/apps/krusader/
%{_datadir}/config/kio_isorc
%{_datadir}/doc/HTML/en/*
%{_datadir}/icons/hicolor/*/apps/*
%{_mandir}/man1/krusader.1*
%{_datadir}/services/iso.protocol
%{_datadir}/services/krarc.protocol

%changelog
* Fri Aug 25 2005 Marcin Garski <mgarski@post.pl> 1.60.0-3
- Include .la files
- Include actions_tutorial.txt
- Fix krusader_root-mode.desktop file to show only in KDE and under System
  category
- Fix compile warnings

* Fri Aug 12 2005 Marcin Garski <mgarski@post.pl> 1.60.0-2
- Spec improvements for Fedora Extras

* Wed Aug 10 2005 Marcin Garski <mgarski@post.pl> 1.60.0-1
- Updated to version 1.60.0 & clean up for Fedora Extras

* Fri Dec 17 2004 Marcin Garski <mgarski@post.pl> 1.51.fc2kde331
- Updated to version 1.51

* Sat Nov 11 2004 Marcin Garski <mgarski@post.pl> 1.50.fc2kde331
- Added Requires:

* Tue Nov 02 2004 Marcin Garski <mgarski@post.pl> 1.50.fc2
- Updated to version 1.50 & spec cleanup

* Fri Aug 06 2004 Marcin Garski <mgarski@post.pl> 1.40-1.fc2
- Updated to version 1.40

* Wed Jun 23 2004 Marcin Garski <mgarski@post.pl> 1.40-beta2.fc2
- Updated to version 1.40-beta2

* Wed Jun 02 2004 Marcin Garski <mgarski@post.pl> 1.40-beta1.fc2
- Rebuild for Fedora Core 2 & huge spec cleanup

* Mon Nov 17 2003 11:05:00 Marian POPESCU <softexpert@libertysurf.fr> [1.30]
- Updated to 1.30 release + changed description to match the official one

* Tue Jul 03 2003 17:00:00 Marcin Garski <mgarski@post.pl> [1.20]
- Initial specfile
