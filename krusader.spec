# derived from spec file by Laurent MONTEL <lmontel@mandrakesoft.com>

%define name krusader
%define version 1.30
%define release 1

Summary: 	 A twin panel file manager for kde
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
Krusader is a new "old-school" file manager and ftp client.
It's twin-panel look follows in the footsteps of the great
file managers of old: GNU's Midnight Commander and
the DOS all time favorite Norton Commander.
Krusader features:
intuative GUI, complete drag n' drop support, transpernt handeling of archives
and ftp volumes, bookmarks (with bookmark manager ), pack and unpack from the
main window, command line with history and "run in terminal" option, internal
viewer, built-in mount manager with auto-mount option and more


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
