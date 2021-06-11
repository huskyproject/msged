%global ver_major 6
%global ver_minor 3
%global ver_patch 0
%global reldate 20210331
%global reltype C

# release number for Release: header
%global relnum 3

# on default static application binary is built but using
# 'rpmbuild --without static' produces an application binary that uses
# dynamic libraries from other subprojects of the Husky project
%if "%_vendor" == "alt"
    %def_with static
%else
    %bcond_without static
%endif

# if you use 'rpmbuild --with debug' then debug binary is produced
%if "%_vendor" == "alt"
    %def_without debug
%else
    %bcond_with debug
%endif

# if you use 'rpmbuild --with doc', then msged-doc package is produced
# with msged.info, msged.html, msged.txt, msged.pdf
%if "%_vendor" == "alt"
    %def_without doc
%else
    %bcond_with doc
%endif

%global debug_package %nil

# for generic build; will override for some distributions
%global vendor_prefix %nil
%global vendor_suffix %nil
%global pkg_group Applications/Communications

# for CentOS, Fedora and RHEL
%if "%_vendor" == "redhat"
    %global vendor_suffix %dist
%endif

# for ALT Linux
%if "%_vendor" == "alt"
    %global vendor_prefix %_vendor
    %global pkg_group Networking/FTN
%endif

%global main_name msged
%if %{with static}
Name: %main_name-static
%else
Name: %main_name
%endif
Version: %ver_major.%ver_minor.%reldate%reltype
Release: %{vendor_prefix}%relnum%{vendor_suffix}
%if "%_vendor" != "redhat"
Group: %pkg_group
%endif
Summary: Msged is the Husky Project message editor
URL: https://github.com/huskyproject/%main_name/archive/v%ver_major.%ver_minor.%reldate.tar.gz
License: Public domain
BuildRequires: gcc ncurses ncurses-devel
%if %{with static}
BuildRequires: huskylib-static-devel >= 1.9
BuildRequires: smapi-static-devel >= 1.9
BuildRequires: fidoconf-static-devel >= 1.9
%else
BuildRequires: huskylib-devel >= 1.9
BuildRequires: smapi-devel >= 1.9
BuildRequires: fidoconf-devel >= 1.9
%endif
Source: %main_name-%ver_major.%ver_minor.%reldate.tar.gz

%description
Msged is the Husky Project message editor

%prep
%setup -q -n %main_name-%ver_major.%ver_minor.%reldate

%if %{with doc}
%package doc
BuildArch: noarch
%if "%_vendor" != "redhat"
Group: %pkg_group
%endif
Summary: Documentation for %main_name
%if "%_vendor" == "redhat"
BuildRequires: texinfo texinfo-tex
%else
BuildRequires: texinfo texi2html texi2dvi
%endif
Provides: %main_name-doc = %version-%release
%description doc
%summary
%endif

%build
%if %{with doc}
    %if %{with static}
        %if %{with debug}
            %make_build DEBUG=1 all gen-doc
        %else
            %make_build all gen-doc
        %endif
    %else
        %if %{with debug}
            %make_build DYNLIBS=1 DEBUG=1 all gen-doc
        %else
            %make_build DYNLIBS=1 all gen-doc
        %endif
    %endif
%else
    %if %{with static}
        %if %{with debug}
            %make_build DEBUG=1 all
        %else
            %make_build all
        %endif
    %else
        %if %{with debug}
            %make_build DYNLIBS=1 DEBUG=1 all
        %else
            %make_build DYNLIBS=1 all
        %endif
    %endif
%endif

%if %{with debug}
    %global __os_install_post %nil
%endif

%install
umask 022
%if %{with doc}
    %if %{with static}
        %if %{with debug}
            %make_install DEBUG=1 install-doc
        %else
            %make_install install-doc
        %endif
    %else
        %if %{with debug}
            %make_install DYNLIBS=1 DEBUG=1 install-doc
        %else
            %make_install DYNLIBS=1 install-doc
        %endif
    %endif
%else
    %if %{with static}
        %if %{with debug}
            %make_install DEBUG=1
        %else
            %make_install
        %endif
    %else
        %if %{with debug}
            %make_install DYNLIBS=1 DEBUG=1
        %else
            %make_install DYNLIBS=1
        %endif
    %endif
%endif
chmod -R a+rX,u+w,go-w %buildroot

%global my_docdir %_defaultdocdir/%main_name-%ver_major.%ver_minor.%ver_patch

%files
%defattr(-,root,root)
%_bindir/*

%if %{with doc}
%files doc
%_infodir/*
%dir %_datadir/%main_name
%_datadir/%main_name/*
%dir %my_docdir
%my_docdir/sample.*
%my_docdir/scheme.*
%my_docdir/whatsnew.txt
%my_docdir/*.html
%my_docdir/*.txt
%my_docdir/*.pdf
%endif
