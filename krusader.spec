# derived from spec file by Laurent MONTEL <lmontel@mandrakesoft.com>

%define name krusader
%define version 1.30
%define release 1

Summary: 	 advanced twin-panel (commander-style) file-manager for KDE 3.x
Name: 		 %{name}
Version: 	 %{version}
Release: 	 %{release}
Source0: 	 %{name}-%{version}.tar.gz
License: 	 GPL
Group: 		 File tools
BuildRoot: %{_tmppath}/%{name}-buildroot
URL: 		   http://krusader.sourceforge.net
Vendor:    Shie Erlich & Rafi Yanai <krusader@users.sourceforge.net>
Packager:  Shie Erlich & Rafi Yanai <krusader@users.sourceforge.net>

Requires: 	kdelibs

%description
Krusader is an advanced twin-panel (commander-style) file-manager for KDE 3.x
(similar to Midnight or Total Commander) but with many extras.
It provides all the file-management features you could possibly want.
Plus: extensive archive handling, mounted filesystem support, FTP,
advanced search module, viewer/editor, directory synchronisation,
file content comparisons, powerful batch renaming and much much more.
It supports the following archive formats: tar, zip, bzip2, gzip, rar, ace,
arj and rpm and can handle other KIOSlaves such as smb:// or fish://
It is (almost) completely customizable, very user friendly,
fast and looks great on your desktop! :-)
You should give it a try.

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n %name-%version

%build
export CFLAGS=$RPM_OPT_FLAGS
export CXXFLAGS=$RPM_OPT_FLAGS
# %%configure doesn't work
./configure --prefix=%_prefix \
	--bindir=%_bindir \
	--datadir=%_datadir \
  --disable-rpath \
	--disable-debug

%make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall KDEDIR=$RPM_BUILD_ROOT%{_prefix} kde_minidir=$RPM_BUILD_ROOT%{_miconsdir}

#menu
mkdir -p $RPM_BUILD_ROOT%{_menudir}

# uncomment when building Mandrake RPM
# kdedesktop2mdkmenu.pl krusader "Applications/File tools"    %buildroot/%_datadir/applnk/Applications/krusader.desktop                             %buildroot/%_menudir/krusader

%find_lang %{name}

%clean
rm -rf $RPM_BUILD_ROOT

%post

# uncomment when building Mandrake RPM
# %{update_menus}

%postun

# uncomment when building Mandrake RPM
# %{clean_menus}

%files -f %name.lang
%defattr(-,root,root)
%doc README AUTHORS ChangeLog TODO COPYING krusader.lsm
%doc %{_datadir}/doc/HTML/en/*
%_bindir/*
%_datadir/applnk/Applications/krusader.desktop

%dir %_datadir/apps/krusader/
%_datadir/apps/krusader/*.rc
%_datadir/apps/krusader/*.png
%_datadir/apps/krusader/*.jpg

%dir %_datadir/apps/krusader/icons/
%dir %_datadir/apps/krusader/icons/hicolor/

%dir %_datadir/apps/krusader/icons/hicolor/16x16/
%dir %_datadir/apps/krusader/icons/hicolor/16x16/actions/
%_datadir/apps/krusader/icons/hicolor/16x16/actions/*.png

%dir %_datadir/apps/krusader/icons/hicolor/22x22/
%dir %_datadir/apps/krusader/icons/hicolor/22x22/actions/
%_datadir/apps/krusader/icons/hicolor/22x22/actions/*.png

%dir %_datadir/apps/krusader/icons/hicolor/32x32/
%dir %_datadir/apps/krusader/icons/hicolor/32x32/actions/
%_datadir/apps/krusader/icons/hicolor/32x32/actions/*.png

# uncomment when building Mandrake RPM
# %_menudir/*

%dir %_datadir/icons
%dir %_datadir/icons/hicolor
%dir %_datadir/icons/hicolor/32x32/
%dir %_datadir/icons/hicolor/32x32/apps/
%_datadir/icons/hicolor/32x32/apps/*.png
%dir %_datadir/icons/locolor
%dir %_datadir/icons/locolor/16x16/
%dir %_datadir/icons/locolor/16x16/apps/
%_datadir/icons/locolor/16x16/apps/krusader.png

%_datadir/man/man1/krusader.1.bz2
%_datadir/mimelnk/application/x-ace.desktop
%_datadir/services/krarc.protocol

%dir %_libdir/kde3/
%_libdir/kde3/kio_krarc.*
