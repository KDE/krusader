# krusader.spec      use rpmbuild to build a Krusader RPM package
# this spec file works on Mandrake 10.0 for krusader 1.50 beta1
# other distributions may need todo some modifications
# If you have comments or suggestions about this spec file please send it to
# [frank_schoolmeesters {*} fastmail {.} fm] Krusader Krew.
# Thanks for your cooperation!

%define beta beta1

# Package information
Summary: 	 advanced twin-panel file-manager for KDE 3.x
Name: 		 krusader
Version: 	 1.50
Release: 	 %{beta}.mdk100
#Release: 	 mdk10.0
Distribution:	 Mandrake Linux 10.0
Source0:         %{name}-%{version}-%{beta}.tar.gz
#Source0: 	 %{name}-%{version}.tar.gz
License: 	 GPL
Group: 		 File tools

# requirements for building the software package, these are not the binary PRM requirements
# e.g. for installing the binary RPM you don't need a C-compiler
BuildRequires: automake autoconf diffutils file m4 texinfo gettext zlib1
BuildRequires: kdelibs kdelibs-devel >= 3 libqt3 >= 3 libqt-devel
BuildRequires: libjpeg62 libjpeg-devel libpng3 libmng1
BuildRequires: libfam-devel arts libart_lgpl2 libstdc++5 libpcre0
BuildRequires: libxfree86 libfreetype6 libfontconfig1 libnas2 libexpat0
BuildRequires: rpm-build gcc-cpp gcc-c++ glibc libgcc1

# BuildRoot: dir who acts as a staging area that looks like the final install dir
# tipicaly (for Mandrake): /var/tmp/krusader-buildroot
# this will become the %%RPM_BUILD_ROOT enviroment variable
BuildRoot:       %{_tmppath}/%{name}-buildroot
URL: 		 http://krusader.sourceforge.net
Vendor:          Krusader Krew [krusader {*} users {.} sourceforge {.}net]
Packager:        Frank Schoolmeesters [frank_schoolmeesters {*} fastmail {.} fm]

# makes binary rpm relocatable e.g.
# rpm -Uvh --relocate /usr=/opt krusader-1.50.mdk100.i586.rpm
# rpm -Uvh --relocate /usr=/usr/local krusader-1.50.mdk100.i586.rpm
Prefix:          /usr

# dependencies requirements for the binary RPM package
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
# changes to the build dir /usr/src/RPM/BUILD/krusader-%version  on Mandrake
# -q: run quitely
%setup -q -n %{name}-%{version}-%{beta}
#%setup -q -n %{name}-%{version}

# commands to build the software in /usr/src/RPM/BUILD/krusader-%version on Mandrake
%build
# set enviroment variables
export QTDIR=%{_libdir}/qt3
export KDEDIR=%{_prefix}

export LD_LIBRARY_PATH=$QTDIR/lib:$KDEDIR/lib:$LD_LIBRARY_PATH
export PATH=$QTDIR/bin:$KDEDIR/bin:$PATH

export CFLAGS=$RPM_OPT_FLAGS
export CXXFLAGS=$RPM_OPT_FLAGS

# run configure
# %%configure doesn't work
# bindir = /usr/bin on Mandrake
# datadir = /usr/share on Mandrake
./configure --prefix=%{_prefix} \
	    --bindir=%{_bindir} \
	    --datadir=%{_datadir} \
            --disable-rpath \
	    --disable-debug

# run make
%make



# commands to install the software in $RPM_BUILD_ROOT
%install
rm -rf $RPM_BUILD_ROOT
%makeinstall KDEDIR=$RPM_BUILD_ROOT%{_prefix} kde_minidir=$RPM_BUILD_ROOT%{_miconsdir}
# menu
#mkdir -p $RPM_BUILD_ROOT%{_menudir}
# adds krusader in the KDE menu with the file /usr/lib/menu/krusader on Mandrake
# Kmenu -> "System/File tools" -> krusader
# /usr/bin/kdedesktop2mdkmenu.pl is a Mandrake Perl script to generate the Kmenu entry
#kdedesktop2mdkmenu.pl krusader "System/File tools"    %{buildroot}/%{_datadir}/applnk/Applications/krusader.desktop                             %{buildroot}/%{_menudir}/krusader

# menu
# creates the file /usr/lib/menu/krusader on Mandrake, this adds krusader in the KDE menu
# in the "System/File tools" section
mkdir -p %{buildroot}/%{_menudir}
cat > %{buildroot}/%{_menudir}/%name << EOF
?package(%name): \
needs="x11" \
kde_filename="%name" \
section="System/File tools" \
title="%name" \
icon="%name.png" \
command="%{_bindir}/%name" \
kde_command="%name -caption \"%c\" %i %m  " \
longtitle="%summary" \
kde_opt="\\\nMiniIcon=%name.png\\\nDocPath=%name/index.html\\\nTerminal=0"
EOF

# obsolete since 1.40 stable
# removes /usr/share/mimelnk/application/x-ace.desktop on Mandrake
# because x-ace.desktop is now provided by KDE
# rm -rf $RPM_BUILD_ROOT/%{_datadir}/mimelnk/application/*.desktop

# find the installed languages e.g. French (fr), Dutch (nl), ...
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
# installs language file(s) if the language(s) is(are) installed on the target computer
%files -f %name.lang

# default attributes for all files in the package (-, user, group)
# - = leave the permissions like the are in the source tarball e.g. 644
%defattr(-,root,root)
# documentation files (docdir name does not need to be included)
%doc README AUTHORS ChangeLog TODO COPYING krusader.lsm INSTALL krusader.spec
# Docbook documentation
%doc %{_datadir}/doc/HTML/en/*

# bindir = /usr/bin, krusader binary: /usr/bin/krusader  on Mandrake
%{_bindir}/*

# libdir = /usr/lib, usr/lib/kde3, usr/lib/menu on Mandrake
%{_libdir}/*

# datadir = /usr/share on Mandrake
%{_datadir}/applnk/Applications/krusader.desktop

# /usr/share/apps/krusader/ on Mandrake
# dir = inlude empty dir but not the files
%dir %{_datadir}/apps/krusader/
%{_datadir}/apps/krusader/*.rc
%{_datadir}/apps/krusader/*.png
%{_datadir}/apps/krusader/*.jpg

# /usr/share/apps/krusader/icons/ on Mandrake
%dir %{_datadir}/apps/krusader/icons/
%dir %{_datadir}/apps/krusader/icons/hicolor/

# /usr/share/apps/krusader/icons/hicolor/16x16/actions/*.png on Mandrake
%dir %{_datadir}/apps/krusader/icons/hicolor/16x16/
%dir %{_datadir}/apps/krusader/icons/hicolor/16x16/actions/
%{_datadir}/apps/krusader/icons/hicolor/16x16/actions/*.png

# /usr/share/apps/krusader/icons/hicolor/22x22/actions/*.png on Mandrake
%dir %{_datadir}/apps/krusader/icons/hicolor/22x22/
%dir %{_datadir}/apps/krusader/icons/hicolor/22x22/actions/
%{_datadir}/apps/krusader/icons/hicolor/22x22/actions/*.png

# /usr/share/apps/krusader/icons/hicolor/32x32/actions/*.png on Mandrake
%dir %{_datadir}/apps/krusader/icons/hicolor/32x32/
%dir %{_datadir}/apps/krusader/icons/hicolor/32x32/actions/
%{_datadir}/apps/krusader/icons/hicolor/32x32/actions/*.png

# /usr/share/apps/konqueror/servicemenus/isoservice.desktop on Mandrake
%dir %{_datadir}/apps/konqueror/servicemenus/
%{_datadir}/apps/konqueror/servicemenus/isoservice.desktop

# /usr/share/config/kio_isorc on Mandrake
%dir %{_datadir}/config/
%{_datadir}/config/kio_isorc

# /usr/share/icons/hicolor/16x16/apps/ on Mandrake
%dir %{_datadir}/icons/hicolor/16x16/apps/
%{_datadir}/icons/hicolor/16x16/apps/*.png

%dir %{_datadir}/icons/hicolor/22x22/apps/
%{_datadir}/icons/hicolor/22x22/apps/*.png

%dir %{_datadir}/icons/hicolor/32x32/apps/
%{_datadir}/icons/hicolor/32x32/apps/*.png

%dir %{_datadir}/icons/hicolor/48x48/apps/
%{_datadir}/icons/hicolor/48x48/apps/*.png

%dir %{_datadir}/icons/hicolor/64x64/apps/
%{_datadir}/icons/hicolor/64x64/apps/*.png


# manpage usr/share/man/man1/krusader.1.bz2 on mandrake
%{_mandir}/man1/*

# /usr/share/services/krarc.protocol on Mandrake
%{_datadir}/services/*

# /usr/share/mimelnk/application/x-ace.desktop on Mandrake
#%{_datadir}/mimelnk/application/*.desktop

%changelog
* Wed Oct 13 2004 Frank Schoolmeesters [frank_schoolmeesters {*} fastmail {.} fm]
- 1.50-mdk100-beta1
  new file added in RPM package:
  /usr/share/apps/konqueror/servicemenus/isoservice.desktop
  /usr/share/config/kio_isorc  
* Thu Jul 26 2004 Frank Schoolmeesters [frank_schoolmeesters {*} fastmail {.} fm]
- 1.40-mdk10
  Mandrake 10.0 has changed the KDE-menu entries
  Former KDE menu entry "Applications/File tools" in Mdk 9.2 is now "System/File tools/"
  Using now a distro independent script to create create the file "/usr/lib/menu/krusader"
  and not the Mandrake Perl script kdedesktop2mdkmenu.pl 
  This adds krusader in the KDE menu in the "System/File tools" section.
      
* Fri Jun 25 2004 Frank Schoolmeesters [frank_schoolmeesters {*} fastmail {.} fm]
- 1.40-beta2.mdk92
- x-ace.desktop is removed in krusader-1.40-beta2/krusader/Makefile.am
  x-ace.desktop is now supllied by KDE
- binary rpm is now relocatable, default is /usr
  you can relocate it to e.g. /usr/local or /opt
  then run $ rpm -Uvh --relocate /usr=/opt krusader-1.40.mdk92.i586.rpm

* Wed May 12 2003 Frank Schoolmeesters [frank_schoolmeesters {*} fastmail {.} fm]
- 1.40-beta1.mdk92
- Initial specfile derived from spec file by Laurent MONTEL [lmontel {*} mandrakesoft {.} com]

