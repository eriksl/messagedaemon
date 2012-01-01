%define dateversion 20120101
%define rel 1ems

%define prefix /usr
%define bindir %{prefix}/bin
%define libdir %{prefix}/lib64

Name: msgd
Release: %{rel}
License: GPLv2
Source: msgd.tar
Vendor: ems
URL: http://messagedaemon.sourceforge.net
BuildRoot: %{_tmppath}/root
Version: %dateversion
AutoProv: 1
AutoReq: 1

Summary: Messagedaemon
Group: Applications/Hardware
%description
A simple tool to receive messages from the network and
display them on various types of LCD and VFD-type displays.

%prep
%setup -n msgd

%build
make %{?_smp_mflags} msgd

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install

%clean
rm -rf %{buildroot}

%files
%defattr(-, root, root, 0755)
%{bindir}/msgd
