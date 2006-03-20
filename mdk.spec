%define 	name	mdk
%define		docname mdk-doc
%define 	version	0.4.2
%define 	release	1
%define 	serial	2
#%define 	prefix	/opt/mdk-0.3.1/usr
%define 	prefix	/usr
#%define         infodirdir %{prefix}/etc/
%define         infodirdir /etc
Summary: 	MIX Development Kit
Name:		%{name}
Version:	%{version}
Release:	%{release}
Serial:		%{serial}
Copyright:	GPL
Group:		Development/Languages
Url:		http://www.gnu.org/software/mdk/mdk.html
Vendor:		Jose A. Ortega Ruiz <jao@gnu.org>
Source0:	http://download.sourcegorge.net/mdk/%{name}-%{version}.tar.gz
Source1:	http://download.sourcegorge.net/mdk/%{docname}-%{version}.tar.gz
#Patch0:	patch-mdk-0.3
Packager:	Agustin Navarro <anp@cantv.net>
BuildRoot:	/var/tmp/%{name}-%{version}-root
#DocDir:		%{prefix}/share/doc/mdk-0.3.1


%description
MDK stands for MIX Development Kit, and provides tools for developing
and executing, in a MIX virtual machine, MIXAL programs.

The MIX is Donald Knuth's mythical computer, described in the first
volume of The Art of Computer Programming, which is programmed using
MIXAL, the MIX assembly language.

MDK includes a MIXAL assembler (mixasm) and a MIX virtual machine
(mixvm) with a command line interface. In case you are an Emacs guy,
you can try doc/mixvm.el, which allows running mixvm inside an Emacs
GUD buffer.

Using these interfaces, you can debug your MIXAL programs at source
code level, and read/modify the contents of all the components of the
MIX computer (including block devices, which are simultated using the
file system).


%package gtk
Summary: 	MIX Development Kit GTK GUI Interface to mixvm
Group:		Development/Languages

%description gtk
MDK stands for MIX Development Kit, and provides tools for developing
and executing, in a MIX virtual machine, MIXAL programs.

The MIX is Donald Knuth's mythical computer, described in the first
volume of The Art of Computer Programming, which is programmed using
MIXAL, the MIX assembly language.

A GTK+ GUI to mixvm, called gmixvm, is provided

Using these interfaces, you can debug your MIXAL programs at source
code level, and read/modify the contents of all the components of the
MIX computer (including block devices, which are simultated using the
file system).


%package doc
Summary: 	MIX Development Kit HTML Documentation
Group:		Development/Languages

%description doc
MDK stands for MIX Development Kit, and provides tools for developing
and executing, in a MIX virtual machine, MIXAL programs.

This pakage includes the HTML Documentation


%prep

%setup -q
%setup -D -T -b 1

#%patch0 -p1

CFLAGS=$RPM_OPT_FLAGS \
	./configure --prefix=%{prefix} --infodir='${prefix}/share/info'

%build
make
#make html

%install
[ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT;
make -e prefix=$RPM_BUILD_ROOT%{prefix} install

%clean
[ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT;

%post
/sbin/install-info %{prefix}/share/info/mdk.info.gz %{infodirdir}/info-dir

%files
%defattr(-,root,root)
%doc AUTHORS ChangeLog COPYING  README NEWS TODO THANKS INSTALL
%doc samples
%doc misc/mixvm.el
%{prefix}/bin/mixasm
%{prefix}/bin/mixvm
%{prefix}/share/info/*

%files gtk
%defattr(-,root,root)
%{prefix}/share/mdk
%{prefix}/bin/gmixvm

%files doc
%defattr(-,root,root)
%doc ../%{docname}-%{version}/img ../%{docname}-%{version}/manual 


%changelog
* Sun Aug 19 2001 Agustin Navarro <anp@cantv.net>
- Upgrade to mdk-0.4

* Sun Jul 22 2001 Agustin Navarro <anp@cantv.net>
- Upgrade to mdk-0.4

* Thu Jun 21 2001 Agustin Navarro <anp@cantv.net>
- Upgrade to mdk-0.3.5

* Tue Jun 12 2001 Agustin Navarro <anp@cantv.net>
- Upgrade to mdk-0.3.4

* Sat Jun 09 2001 Agustin Navarro <anp@cantv.net>
- Upgrade to mdk-0.3.3

* Thu May 16 2001 Agustin Navarro <anp@cantv.net>
- Upgrade to 0.3.2

* Fri Mar 30 2001 Agustin Navarro <anp@cantv.net>
- Initial Rpm Release


