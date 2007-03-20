# Spec file for Krusader-1.70.1 on Fedora Core 6 (Fedora Extras) by Marcin Garski <mgarski[AT]post.pl>
# http://cvs.fedora.redhat.com/viewcvs/rpms/krusader/FC-6/?root=extras 
# http://fedoraproject.org/extras/6/SRPMS/repoview/krusader.html
# http://fedoraproject.org/extras/6/i386/repoview/krusader.html

Name:		krusader
Version:	1.70.1
Release:	2%{?dist}
Summary:	An advanced twin-panel (commander-style) file-manager for KDE

Group:		Applications/File
License:	GPL
URL:		http://krusader.sourceforge.net/
Source0:	http://dl.sourceforge.net/%{name}/%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	qt-devel kdelibs-devel kdebase-devel kdebindings-devel
BuildRequires:	desktop-file-utils automake gettext
BuildRequires:	arts-devel libpng-devel gamin-devel
Requires(post): desktop-file-utils
Requires(postun): desktop-file-utils

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

%build
unset QTDIR || : ; . %{_sysconfdir}/profile.d/qt.sh
export QTLIB=${QTDIR}/lib QTINC=${QTDIR}/include

%configure \
	--disable-rpath \
	--disable-debug \
	--disable-dependency-tracking
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

desktop-file-install --vendor fedora --delete-original \
	--dir $RPM_BUILD_ROOT%{_datadir}/applications \
	--add-category X-Fedora \
	$RPM_BUILD_ROOT%{_datadir}/applnk/*.desktop

# Workaround for some wicked install bug
rm -rf $RPM_BUILD_ROOT%{_tmppath}

# Make symlink relative
pushd $RPM_BUILD_ROOT%{_docdir}/HTML/en/krusader/
ln -sf ../common
popd

%find_lang %{name}

%post
update-desktop-database &> /dev/null ||:

touch --no-create %{_datadir}/icons/crystalsvg || :
if [ -x %{_bindir}/gtk-update-icon-cache ]; then
	%{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/crystalsvg || :
fi

touch --no-create %{_datadir}/icons/locolor || :
if [ -x %{_bindir}/gtk-update-icon-cache ]; then
	%{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/locolor || :
fi

%postun
update-desktop-database &> /dev/null ||:

touch --no-create %{_datadir}/icons/crystalsvg || :
if [ -x %{_bindir}/gtk-update-icon-cache ]; then
	%{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/crystalsvg || :
fi

touch --no-create %{_datadir}/icons/locolor || :
if [ -x %{_bindir}/gtk-update-icon-cache ]; then
	%{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/locolor || :
fi

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
%{_docdir}/HTML/en/krusader/
%{_datadir}/icons/crystalsvg/*/apps/*
%{_datadir}/icons/locolor/*/apps/*
%{_mandir}/man1/krusader.1*
%{_datadir}/services/iso.protocol
%{_datadir}/services/krarc.protocol

%changelog
* Fri Sep 01 2006 Marcin Garski <mgarski[AT]post.pl> 1.70.1-2
- Rebuild for Fedora Core 6
- Spec tweak

* Sat Jul 29 2006 Marcin Garski <mgarski[AT]post.pl> 1.70.1-1
- Updated to version 1.70.1 which fix CVE-2006-3816 (#200323)

* Mon Feb 13 2006 Marcin Garski <mgarski[AT]post.pl> 1.70.0-1
- Remove all patches (merged upstream)
- Updated to version 1.70.0

* Mon Jan 16 2006 Marcin Garski <mgarski[AT]post.pl> 1.60.1-6
- Remove --enable-final

* Mon Jan 16 2006 Marcin Garski <mgarski[AT]post.pl> 1.60.1-5
- Remove --disable-dependency-tracking

* Sun Jan 15 2006 Marcin Garski <mgarski[AT]post.pl> 1.60.1-4
- Change "/etc/profile.d/qt.sh" to "%{_sysconfdir}/profile.d/qt.sh"
- Add --disable-debug --disable-dependency-tracking & --enable-final

* Wed Dec 14 2005 Marcin Garski <mgarski[AT]post.pl> 1.60.1-3
- Add to BR libacl-devel

* Tue Dec 13 2005 Marcin Garski <mgarski[AT]post.pl> 1.60.1-2
- Fix for modular X.Org

* Mon Dec 12 2005 Marcin Garski <mgarski[AT]post.pl> 1.60.1-1
- Updated to version 1.60.1 which fix CVE-2005-3856

* Sun Oct 23 2005 Marcin Garski <mgarski[AT]post.pl> 1.60.0-4
- Added update-mime-database and gtk-update-icon-cache (bug #171547)

* Thu Aug 25 2005 Marcin Garski <mgarski[AT]post.pl> 1.60.0-3
- Include .la files
- Include actions_tutorial.txt
- Fix krusader_root-mode.desktop file to show only in KDE and under System
  category
- Fix compile warnings

* Fri Aug 12 2005 Marcin Garski <mgarski[AT]post.pl> 1.60.0-2
- Spec improvements for Fedora Extras

* Wed Aug 10 2005 Marcin Garski <mgarski[AT]post.pl> 1.60.0-1
- Updated to version 1.60.0 & clean up for Fedora Extras

* Fri Dec 17 2004 Marcin Garski <mgarski[AT]post.pl> 1.51.fc2kde331
- Updated to version 1.51

* Sat Nov 11 2004 Marcin Garski <mgarski[AT]post.pl> 1.50.fc2kde331
- Added Requires:

* Tue Nov 02 2004 Marcin Garski <mgarski[AT]post.pl> 1.50.fc2
- Updated to version 1.50 & spec cleanup

* Fri Aug 06 2004 Marcin Garski <mgarski[AT]post.pl> 1.40-1.fc2
- Updated to version 1.40

* Wed Jun 23 2004 Marcin Garski <mgarski[AT]post.pl> 1.40-beta2.fc2
- Updated to version 1.40-beta2

* Wed Jun 02 2004 Marcin Garski <mgarski[AT]post.pl> 1.40-beta1.fc2
- Rebuild for Fedora Core 2 & huge spec cleanup

* Mon Nov 17 2003 11:05:00 Marian POPESCU <softexpert[AT]libertysurf.fr> [1.30]
- Updated to 1.30 release + changed description to match the official one

* Tue Jul 03 2003 17:00:00 Marcin Garski <mgarski[AT]post.pl> [1.20]
- Initial specfile
