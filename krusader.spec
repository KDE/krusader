
%define name	krusader
%define version	2.0
%define snapshot 6014
%define rel	1

%if %snapshot
%define release	%mkrel 0.svn%snapshot.%rel
%else
%define release %mkrel %rel
%endif

Summary: 	Advanced KDE twin-panel file-manager
Name: 		%{name}
Version: 	%{version}
Release: 	%{release}
%if %snapshot
# http://krusader.svn.sourceforge.net/svnroot/krusader/trunk/krusader_kde4
Source:		%{name}-%{snapshot}.tar.bz2
%else
Source: 	http://downloads.sourceforge.net/krusader/%{name}-%{version}.tar.gz
%endif
License: 	GPL
Group: 		File tools
BuildRoot: 	%{_tmppath}/%{name}-buildroot
URL: 		http://krusader.sourceforge.net/
Epoch:		3
BuildRequires:  kdelibs4-devel
Obsoletes:	kde3-krusader < 2:1.02-4
Provides:	kde3-krusader
# crystalsvg:
Requires(post):	kdelibs-common
Requires(postun): kdelibs-common

%description
Krusader is an advanced twin panel (commander style) file manager
for KDE and other desktops in the *nix world, similar to Midnight or
Total Commander. It provides all the file management features you
could possibly want.

Plus: extensive archive handling, mounted filesystem support, FTP,
advanced search module, an internal viewer/editor, directory
synchronisation, file content comparisons, powerful batch renaming
and much much more. It supports a wide variety of archive formats
and can handle other KIO slaves such as smb or fish.

%prep
%if %snapshot
%setup -q -n %name-%snapshot
%else
%setup -q
%endif

%build
%cmake_kde4
%make

%install
rm -rf %{buildroot}
%makeinstall_std -Cbuild

%find_lang krusader

%clean
rm -rf $RPM_BUILD_ROOT

%post
%update_menus
%update_desktop_database
%update_icon_cache hicolor

%postun
%clean_menus
%clean_icon_cache hicolor
%clean_desktop_database

%files -f krusader.lang
%defattr(-,root,root)
%doc README AUTHORS ChangeLog TODO COPYING krusader.lsm
%doc %{_kde_datadir}/doc/HTML/en/*
%{_kde_bindir}/krusader
%{_kde_datadir}/applications/kde4/krusader*.desktop
%{_kde_datadir}/apps/krusader
%{_kde_datadir}/kde4/services/*.protocol
%{_kde_datadir}/config/kio_isorc
%{_kde_iconsdir}/hicolor/*/apps/krusader*.png
%{_kde_iconsdir}/locolor/*/apps/krusader*.png
%{_kde_libdir}/kde4/*.so



%changelog
* Thu May 29 2008 Anssi Hannula <anssi@mandriva.org> 3:2.0-0.svn6014.1mdv2009.0
+ Revision: 212845
- new snapshot

* Sun May 04 2008 Anssi Hannula <anssi@mandriva.org> 3:2.0-0.svn2779.1mdv2009.0
+ Revision: 200809
- new snapshot from KDE4 branch

* Mon Apr 28 2008 Anssi Hannula <anssi@mandriva.org> 3:1.90.0-1mdv2009.0
+ Revision: 197851
- new version
- versionize obsoletes
- buildrequires acl-devel
- require kdelibs-common for crystalsvg for rpm scripts

* Wed Jan 02 2008 Olivier Blin <oblin@mandriva.com> 3:1.80.0-2mdv2008.1
+ Revision: 140918
- restore BuildRoot

  + Thierry Vignaud <tvignaud@mandriva.com>
    - kill re-definition of %%buildroot on Pixel's request

* Sat Aug 25 2007 Anssi Hannula <anssi@mandriva.org> 3:1.80.0-2mdv2008.0
+ Revision: 71406
- rebuild for new kdelibs

* Wed Jul 25 2007 Anssi Hannula <anssi@mandriva.org> 3:1.80.0-1mdv2008.0
+ Revision: 55271
- 1.80.0 final
- use proper configure macro
- better description and summary
- clean .spec
- do not call update_icon_cache for non-existing theme locolor


* Wed Apr 04 2007 Laurent Montel <lmontel@mandriva.com> 1.80.0-0.beta2.1mdv2007.1
+ Revision: 150492
- 1.80.0-beta2
- Import krusader

* Tue Sep 05 2006 Anssi Hannula <anssi@mandriva.org> 3:1.70.1-1mdv2007.0
- 1.70.1
- add missing clean_desktop_database
- fix icons and legacy menu
- drop cleaning buildroot in prep section

* Mon Jul 03 2006 Nicolas Lécureuil <neoclust@mandriva.org> 3:1.70.0-6mdv2007.0
- Rebuild for new menu and extensions
- Use macros for icons

* Mon May 22 2006 Laurent MONTEL <lmontel@mandriva.com> 1.70.0-5
- Rebuild

* Thu May 11 2006 Nicolas Lécureuil <neoclust@mandriva.org> 3:1.70.0-4mdk
- Remove redundant BuildRequires

* Wed May 10 2006 Nicolas Lécureuil <neoclust@mandriva.org> 3:1.70.0-3mdk
- Fix BuildRequires

* Tue May 09 2006 Laurent MONTEL <lmontel@mandriva.com> 1.70.0-2
- Rebuild to generate category

* Mon Feb 13 2006 Nicolas Lécureuil <neoclust@mandriva.org> 1.70.0-1mdk
- 1.70

* Wed Dec 14 2005 Laurent MONTEL <lmontel@mandriva.com> 1.70.0-beta2.2
- Use patch from Anssi Hannula <anssi.hannula@gmail.com> to fixing build
on x86_64 and use mkrel

* Sun Nov 06 2005 Laurent MONTEL <lmontel@mandriva.com> 1.70.0-beta2.1mdk
- beta2

* Sat Oct 29 2005 Nicolas Lécureuil <neoclust@mandriva.org> 1.70.0-beta1.5mdk
- Add BuildRequires

* Sat Oct 29 2005 Nicolas Lécureuil <neoclust@mandriva.org> 1.70.0-beta1.4mdk
- Fix conflict

* Thu Oct 20 2005 Nicolas Lécureuil <neoclust@mandriva.org> 1.70.0-beta1.3mdk
- Fix conflict

* Tue Oct 18 2005 Nicolas Lécureuil <neoclust@mandriva.org> 1.70.0-beta1.2mdk
- Fix BuildRequires

* Tue Oct 18 2005 Laurent MONTEL <lmontel@mandriva.com> 1.70.0-beta1.1mdk
- 1.70.beta1

* Tue May 03 2005 Laurent MONTEL <lmontel@mandriva.com> 1.60.0-2mdk
- Fix x64 build fix bug #15728

* Tue Apr 12 2005 Lenny Cartier <lenny@mandrakesoft.com> 1.60.0-1mdk
- 1.60.0

* Tue Mar 22 2005 Laurent MONTEL <lmontel@mandrakesoft.com> 1.60-0.beta2.1mdk
- 1.60 beta2

* Fri Mar 04 2005 Laurent MONTEL <lmontel@mandrakesoft.com> 1.60-0.beta1.1mdk
- 1.60 beta1

* Wed Dec 15 2004 Laurent MONTEL <lmontel@mandrakesoft.com> 1.51-1mdk
- 1.51

* Tue Nov 02 2004 Laurent MONTEL <lmontel@mandrakesoft.com> 1.50-2mdk
- Add patch3: fix potential crash

* Mon Nov 01 2004 Laurent MONTEL <lmontel@mandrakesoft.com> 1.50-1mdk
- 1.50

* Fri Jul 23 2004 Lenny Cartier <lenny@mandrakesoft.com> 1.40-1mdk
- 1.40

* Tue Jun 29 2004 Laurent MONTEL <lmontel@mandrakesoft.com> 1.40-0.beta2.1mdk
- beta2

* Mon Jun 07 2004 Angelo Naselli <random_lx@yahoo.com> 1.40-0.beta1.4mdk
- Fix icon position

* Sat Jun 05 2004 Laurent MONTEL <lmontel@mandrakesoft.com> 1.40-0.beta1.3mdk
- Rebuild

* Thu May 06 2004 Laurent MONTEL <lmontel@mandrakesoft.com> 1.40-0.beta1.2mdk
- Update description (patch give by frank_schoolmeesters@fastmail.fm)

* Thu Apr 22 2004 Laurent MONTEL <lmontel@mandrakesoft.com> 1.40-0.beta1.1mdk
- 1.40beta1

