# Spec file for Krusader-2.0.0 for Fedora 12 by Marcin Garski <mgarski[AT]post.pl>
# http://cvs.fedoraproject.org/viewcvs/rpms/krusader/devel/?root=extras

Name:		krusader
Version:	2.0.0
Release:	2.1%{?dist}
Summary:	An advanced twin-panel (commander-style) file-manager for KDE

Group:		Applications/File
License:	GPLv2+
URL:		http://krusader.sourceforge.net/
Source0:	http://downloads.sourceforge.net/%{name}/%{name}-%{version}%{?beta:-%{beta}}.tar.gz
Patch0:		krusader-2.0.0-gcc-4.4.patch
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	cmake
BuildRequires:	kdelibs-devel >= 4.1.0 phonon-devel
BuildRequires:	libjpeg-devel libpng-devel giflib-devel
BuildRequires:	zlib-devel bzip2-devel
BuildRequires:	pcre-devel gamin-devel libacl-devel
BuildRequires:	xdg-utils gettext

%description
Krusader is an advanced twin panel (commander style) file manager for KDE and
other desktops in the *nix world, similar to Midnight or Total Commander.
It provides all the file management features you could possibly want.
Plus: extensive archive handling, mounted filesystem support, FTP, advanced
search module, an internal viewer/editor, directory synchronisation,
file content comparisons, powerful batch renaming and much much more.
It supports a wide variety of archive formats and can handle other KIO slaves
such as smb or fish. It is (almost) completely customizable, very user
friendly, fast and looks great on your desktop! You should give it a try.

%prep
%setup -q -n %{name}-%{version}%{?beta:-%{beta}}
%patch0 -p1

%build
mkdir -p %{_target_platform}
pushd %{_target_platform}
%{cmake_kde4} ..
popd

make %{?_smp_mflags} -C %{_target_platform}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot} -C %{_target_platform}

# Make symlink relative and remove wrong EOL
pushd $RPM_BUILD_ROOT%{_docdir}/HTML/
for i in *
do
	pushd $RPM_BUILD_ROOT%{_docdir}/HTML/$i/krusader/
	for j in *.docbook
	do
		tr -d '\r' < $j > ${j}.tmp
		mv -f ${j}.tmp $j
	done
	ln -sf ../common
	popd
done
popd

%find_lang %{name}

%post
xdg-icon-resource forceupdate --theme hicolor 2> /dev/null || :
xdg-icon-resource forceupdate --theme locolor 2> /dev/null || :
xdg-desktop-menu forceupdate 2> /dev/null || :

%postun
xdg-icon-resource forceupdate --theme hicolor 2> /dev/null || :
xdg-icon-resource forceupdate --theme locolor 2> /dev/null || :
xdg-desktop-menu forceupdate 2> /dev/null || :

%clean
rm -rf %{buildroot}

%files -f %{name}.lang
%defattr(-,root,root,-)
%doc doc/actions_tutorial.txt AUTHORS ChangeLog COPYING FAQ README SVNNEWS TODO
%{_kde4_bindir}/*
%{_kde4_libdir}/kde4/*.so
%{_kde4_datadir}/applications/kde4/krusader*.desktop
%{_kde4_datadir}/config/kio_isorc
%{_kde4_docdir}/HTML/en/krusader/
%{_kde4_iconsdir}/hicolor/*/apps/*.png
%{_kde4_iconsdir}/locolor/*/apps/*.png
%{_kde4_datadir}/kde4/apps/krusader/
%{_kde4_datadir}/kde4/services/*.protocol

%changelog
* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.0.0-2.1
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Mon Apr 20 2009 Marcin Garski <mgarski[AT]post.pl> 2.0.0-1.1
- Update to final 2.0.0

* Sun Mar 15 2009 Marcin Garski <mgarski[AT]post.pl> 2.0.0-0.5.beta2
- Fix compile error with Qt 4.5

* Wed Feb 25 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.0.0-0.4.beta2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Sun Dec 28 2008 Marcin Garski <mgarski[AT]post.pl> 2.0.0-0.3.beta2
- Update to 2.0.0-beta2

* Sat Oct 18 2008 Marcin Garski <mgarski[AT]post.pl> 2.0.0-0.2.beta1
- Incorporate minor bug fixes from Krusader's SVN

* Thu Oct 16 2008 Marcin Garski <mgarski[AT]post.pl> 2.0.0-0.1.beta1
- Update to 2.0.0-beta1

* Wed Oct 15 2008 Rex Dieter <rdieter@fedoraproject.org> 1.90.0-3
- s/crystalsvg/hicolor/ icon theme, so they show for everyone (#467076)

* Sun Apr 13 2008 Marcin Garski <mgarski[AT]post.pl> 1.90.0-2
- Update to 1.90.0
- Remove krusader-1.80.0-gcc43-compile-fix.patch, merged upstream

* Thu Feb 14 2008 Marcin Garski <mgarski[AT]post.pl> 1.80.0-5
- GCC 4.3 compile fix

* Sat Dec 15 2007 Marcin Garski <mgarski[AT]post.pl> 1.80.0-4
- Remove kdebindings-devel dependency (#425081)

* Sun Dec 09 2007 Marcin Garski <mgarski[AT]post.pl> 1.80.0-3
- BR: kdelibs3-devel kdebase3-devel

* Fri Aug 31 2007 Marcin Garski <mgarski[AT]post.pl> 1.80.0-2
- Fix license tag

* Thu Aug 02 2007 Marcin Garski <mgarski[AT]post.pl> 1.80.0-1
- Update to 1.80.0 (#249903)
- Preserve upstream .desktop vendor

* Fri Apr 20 2007 Marcin Garski <mgarski[AT]post.pl> 1.80.0-0.1.beta2
- Updated to version 1.80.0-beta2
- Drop X-Fedora category

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
- Change "/etc/profile.d/qt.sh" to "%%{_sysconfdir}/profile.d/qt.sh"
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
