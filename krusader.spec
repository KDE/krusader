# krusader.spec      use rpmbuild to build a Krusader RPM package
# Derived from spec file by Laurent MONTEL <lmontel@mandrakesoft.com>

# this spec file works on Mandrake 9.2 for krusader 1.40 beta1
# other distributions may need todo some modifications
# If you have comments or suggestions about this spec file please send it to
# <frank_schoolmeesters@fastmail.fm> Krusader Krew.
# Thanks for your cooperation!

# RPM package name
%define name krusader
%define version 1.40
%define release beta1.mdk92
# Name of the packager: please replace the following text with
# your name and e-mail adress
%define packager your name <your e-mail@adress.foo>


# Package information
Summary: 	 advanced twin-panel file-manager for KDE 3.x
Name: 		 %{name}
Version: 	 %{version}
Release: 	 %{release}
Source0:         %name-%{version}-beta1.tar.gz
#Source0: 	 %{name}-%{version}.tar.gz
#not used	 Patch0:         foo.patch.gz
License: 	 GPL
Group: 		 File tools

# requirements for building the software package, these are not the PRM requirements
# e.g. for installing the RPM you will not need a C-compiler
BuildRequires: automake autoconf diffutils file m4 texinfo gettext zlib1
BuildRequires: kdelibs kdelibs-devel >= 3 libqt3 >= 3 libqt-devel
BuildRequires: libjpeg62 libjpeg-devel libpng3 libmng1
BuildRequires: libfam-devel arts libart_lgpl2 libstdc++5 libpcre0
BuildRequires: libxfree86 libfreetype6 libfontconfig1 libnas2 libexpat0
BuildRequires: rpm-build gcc-cpp gcc-c++ glibc libgcc1

# BuildRoot: dir who acts as a staging area that looks like the final install dir
# tipicaly(for Mandrake): /var/tmp/krusader-buildroot
# this will become the %%RPM_BUILD_ROOT enviroment variable
BuildRoot:       %{_tmppath}/%{name}-buildroot
URL: 		 http://krusader.sourceforge.net
#not used	Epoch:	 1
Vendor:          Shie Erlich, Rafi Yanai, Krusader Krew <krusader@users.sourceforge.net>
Packager:        %{packager}


# dependencies requirements for the RPM package
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

# preparing for the build by unpacking the sources and applying any patches
# in /usr/src/RPM/BUILD/krusader-%version on Mandrake
%prep
rm -rf $RPM_BUILD_ROOT
# changes to the build dir /usr/src/RPM/BUILD/krusader-%version  on Mandrake
# -q: run quitely
%setup -q -n %name-%version-beta1
#%setup -q -n %name-%version
#%%patch1 -p1 -b .foo


# commands to build the software in /usr/src/RPM/BUILD/krusader-%version on Mandrake
%build
# set enviroment variables
export QTDIR=%_libdir/qt3
export KDEDIR=%_prefix

export LD_LIBRARY_PATH=$QTDIR/lib:$KDEDIR/lib:$LD_LIBRARY_PATH
export PATH=$QTDIR/bin:$KDEDIR/bin:$PATH

export CFLAGS=$RPM_OPT_FLAGS
export CXXFLAGS=$RPM_OPT_FLAGS

# run configure
# %%configure doesn't work
# bindir = /usr/bin on Mandrake
# datadir = /usr/share on Mandrake
./configure --prefix=%_prefix \
	    --bindir=%_bindir \
	    --datadir=%_datadir \
            --disable-rpath \
	    --disable-debug

# run make
%make



# commands to install the software in $RPM_BUILD_ROOT
%install
rm -rf $RPM_BUILD_ROOT
%makeinstall KDEDIR=$RPM_BUILD_ROOT%{_prefix} kde_minidir=$RPM_BUILD_ROOT%{_miconsdir}
# menu
mkdir -p $RPM_BUILD_ROOT%{_menudir}
# adds krusader in the KDE menu at "Applications/File tools"
kdedesktop2mdkmenu.pl krusader "Applications/File tools"    %buildroot/%_datadir/applnk/Applications/krusader.desktop                             %buildroot/%_menudir/krusader

# removes /usr/share/mimelnk/application/x-ace.desktop on Mandrake
rm -rf $RPM_BUILD_ROOT/%_datadir/mimelnk/application/*.desktop

%find_lang %{name}

# commands to clean up after the build and install in $RPM_BUILD_ROOT
%clean
rm -rf $RPM_BUILD_ROOT



# script that runs after installing the RPM package
%post
# updates the KDE menu with Krusader
%{update_menus}



# script that runs after uninstalling the RPM package
%postun
# removes Krusader from the KDE menu
%{clean_menus}



# all the files that the RPM package should install, this list must be exhausitive
# i18n   /usr/share/locale/foo/LC_MESSAGES/krusader.mo on mandrake
%files -f %name.lang

# set default attributes for all files in the package (-, user, group)
# - = leave the permissions like the are in the source tarball e.g. 644
%defattr(-,root,root)
# documentation files (docdir name does not need to be included)
%doc README AUTHORS ChangeLog TODO COPYING krusader.lsm INSTALL
# Docbook documentation
%doc %{_datadir}/doc/HTML/en/*

# bindir = /usr/bin, krusader binary: /usr/bin/krusader  on Mandrake
%_bindir/*

# datadir = /usr/lib, usr/lib/kde3, usr/lib/menu on Mandrake
%_libdir/*

# datadir = /usr/share on Mandrake
%_datadir/applnk/Applications/krusader.desktop

# /usr/share/apps/krusader/ on Mandrake
# dir = inlude empty dir but not the files
%dir %_datadir/apps/krusader/
%_datadir/apps/krusader/*.rc
%_datadir/apps/krusader/*.png
%_datadir/apps/krusader/*.jpg

# /usr/share/apps/krusader/icons/ on Mandrake
%dir %_datadir/apps/krusader/icons/
%dir %_datadir/apps/krusader/icons/hicolor/

# /usr/share/apps/krusader/icons/hicolor/16x16/actions/*.png on Mandrake
%dir %_datadir/apps/krusader/icons/hicolor/16x16/
%dir %_datadir/apps/krusader/icons/hicolor/16x16/actions/
%_datadir/apps/krusader/icons/hicolor/16x16/actions/*.png

# /usr/share/apps/krusader/icons/hicolor/22x22/actions/*.png on Mandrake
%dir %_datadir/apps/krusader/icons/hicolor/22x22/
%dir %_datadir/apps/krusader/icons/hicolor/22x22/actions/
%_datadir/apps/krusader/icons/hicolor/22x22/actions/*.png

# /usr/share/apps/krusader/icons/hicolor/32x32/actions/*.png on Mandrake
%dir %_datadir/apps/krusader/icons/hicolor/32x32/
%dir %_datadir/apps/krusader/icons/hicolor/32x32/actions/
%_datadir/apps/krusader/icons/hicolor/32x32/actions/*.png


# /usr/share/icons/hicolor/16x16/apps/ on Mandrake
%dir %_datadir/icons/hicolor/16x16/apps/
%_datadir/icons/hicolor/16x16/apps/*.png

%dir %_datadir/icons/hicolor/22x22/apps/
%_datadir/icons/hicolor/22x22/apps/*.png

%dir %_datadir/icons/hicolor/32x32/apps/
%_datadir/icons/hicolor/32x32/apps/*.png

%dir %_datadir/icons/hicolor/48x48/apps/
%_datadir/icons/hicolor/48x48/apps/*.png

%dir %_datadir/icons/hicolor/64x64/apps/
%_datadir/icons/hicolor/64x64/apps/*.png


# manpage usr/share/man/man1/krusader.1.bz2 on mandrake
%_mandir/man1/*

# /usr/share/services/krarc.protocol on Mandrake
%_datadir/services/*

# /usr/share/mimelnk/application/x-ace.desktop on Mandrake
#%_datadir/mimelnk/application/*.desktop
