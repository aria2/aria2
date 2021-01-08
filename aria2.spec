%define binname aria2c

Name:           aria2
Version:        1.19.3
Release:        1%{?dist}
Summary:        High speed download utility with resuming and segmented downloading
Group:          Applications/Internet
License:        GPLv2+ with exceptions
URL:            https://github.com/tatsuhiro-t/aria2
Source0:        https://github.com/tatsuhiro-t/%{name}/releases/download/release-%{version}/%{name}-%{version}.tar.xz
#Patch0:         aria2-1.18.10-use-system-wide-crypto-policies.patch
BuildRequires:  bison
BuildRequires:  c-ares-devel cppunit-devel
BuildRequires:  gettext gnutls-devel
BuildRequires:  libgcrypt-devel libxml2-devel
BuildRequires:  sqlite-devel
BuildRequires:  gettext

%description
aria2 is a download utility with resuming and segmented downloading.
Supported protocols are HTTP/HTTPS/FTP/BitTorrent. It also supports Metalink
version 3.0.

Currently it has following features:
- HTTP/HTTPS GET support
- HTTP Proxy support
- HTTP BASIC authentication support
- HTTP Proxy authentication support
- FTP support(active, passive mode)
- FTP through HTTP proxy(GET command or tunneling)
- Segmented download
- Cookie support
- It can run as a daemon process.
- BitTorrent protocol support with fast extension.
- Selective download in multi-file torrent
- Metalink version 3.0 support(HTTP/FTP/BitTorrent).
- Limiting download/upload speed

%prep
%setup -q

%build
%configure --enable-bittorrent \
           --enable-metalink \
           --enable-epoll\
           --disable-rpath \
           --with-gnutls \
           --with-libcares \
           --with-libxml2 \
           --with-openssl \
           --with-libz \
           --with-sqlite3 \


V=1 make %{?_smp_mflags}

%install
%make_install
%find_lang %{name}
rm -f $RPM_BUILD_ROOT%{_datadir}/locale/locale.alias
rm -rf $RPM_BUILD_ROOT%{_datadir}/doc/%{name}
#
mkdir -p $RPM_BUILD_ROOT/lib/systemd/system
cat > $RPM_BUILD_ROOT/lib/systemd/system/%{name}.service <<HERE
[Unit]
Description=Aria2 User Service by %u
After=network.target

[Service]
Type=forking
User=nobody
ExecStart=/usr/bin/aria2c --conf-path=/etc/aria2/aria2.conf --log=/var/log/aria2.log

[Install]
WantedBy=multi-user.target
HERE
#
mkdir -p $RPM_BUILD_ROOT/etc/firewalld/services
cat > $RPM_BUILD_ROOT/etc/firewalld/services/%{name}.xml <<HERE
<?xml version="1.0" encoding="utf-8"?>
<service>
 <short>aria2</short>
 <description>aria2 command-line download utility</description>
 <port protocol="tcp" port="6800"/>
</service>
HERE
#
mkdir -p $RPM_BUILD_ROOT/etc/%{name}
cat > $RPM_BUILD_ROOT/etc/%{name}/%{name}.conf <<HERE
###
#More info: https://aria2.github.io/manual/en/html/aria2c.html
#
#Run as daemon. The current working directory will be changed to / and standard input,
#standard output and standard error will be redirected to /dev/null. Default: false
  daemon=true
#Continue downloading a partially downloaded file. Use this option to resume a download
#started by a web browser or another program which downloads files sequentially from the beginning.
#Currently this option is only applicable to HTTP(S)/FTP downloads.
  continue=true
#Enable JSON-RPC/XML-RPC server. It is strongly recommended to set secret authorization token using
#--rpc-secret option. See also --rpc-listen-port option. Default: false
  enable-rpc
#Add Access-Control-Allow-Origin header field with value * to the RPC response. Default: false
  rpc-allow-origin-all=true
#Listen incoming JSON-RPC/XML-RPC requests on all network interfaces. If false is given,
#listen only on local loopback interface. Default: false
  rpc-listen-all=true
#Set RPC secret authorization token. Read RPC authorization secret token to know how this option value is used.
#To create string use openssl rand -hex 15
  rpc-secret=83b35a540ba5ae254fbe3b61e0b921
#The directory to store the downloaded file.
  dir=/tmp
#Save error/unfinished downloads to FILE on exit. You can pass this output file to aria2c with --input-file option on restart.
#If you like the output to be gzipped append a .gz extension to the file name. Please note that downloads added by
#aria2.addTorrent() and aria2.addMetalink() RPC method and whose meta data could not be saved as a file are not saved.
#Downloads removed using aria2.remove() and aria2.forceRemove() will not be saved. GID is also saved with gid,
#but there are some restrictions, see below.
  save-session=/etc/aria2/session.lock
#Downloads the URIs listed in FILE. You can specify multiple sources for a single entity by putting multiple URIs on a single
#line separated by the TAB character. Additionally, options can be specified after each URI line. Option lines must start with
#one or more white space characters (SPACE or TAB) and must only contain one option per line. Input files can use gzip compression.
#When FILE is specified as -, aria2 will read the input from stdin. See the Input File subsection for details. See also the
#--deferred-input option. See also the --save-session-file option.
  input-file=/etc/aria2/session.lock
#Change the IPv4 DHT routing table file to PATH. Default: $HOME/.aria2/dht.dat if present, otherwise $XDG_CACHE_HOME/aria2/dht.dat
  dht-file-path=/etc/aria2/dht.dat
#Disable IPv6. This is useful if you have to use broken DNS and want to avoid terribly slow AAAA record lookup. Default: false
  disable-ipv6=true
#Set log level to output. LEVEL is either debug, info, notice, warn or error. Default: debug
  log-level=warn
###
HERE
cat > $RPM_BUILD_ROOT/etc/%{name}/session.lock <<HERE
HERE
cat > $RPM_BUILD_ROOT/etc/%{name}/dht.dat <<HERE
HERE
#
mkdir -p $RPM_BUILD_ROOT/var/log
cat > $RPM_BUILD_ROOT/var/log/%{name}.log <<HERE
HERE
#
%files -f %{name}.lang
%attr(644,root,root) /lib/systemd/system/%{name}.service
%attr(644,root,root) /etc/firewalld/services/%{name}.xml
%dir %attr(755,nobody,nobody) /etc/%{name}
%attr(644,nobody,nobody) /etc/%{name}/%{name}.conf
%attr(644,nobody,nobody) /etc/%{name}/session.lock
%attr(644,nobody,nobody) /etc/%{name}/dht.dat
%attr(644,nobody,nobody) /var/log/%{name}.log
%doc AUTHORS ChangeLog COPYING README
%{_bindir}/%{binname}
%{_mandir}/man1/aria2c.1.gz
%{_mandir}/*/man1/aria2c.1.gz

%changelog
* Sun Feb 08 2016 Aleksandr Chernyshev <wmlex@yandex.ru> 1.19.3-1
- Update to 1.19.3
- Add systemd service
- Add firewalld preset

* Fri Jun 05 2015 Athmane Madjoudj <athmane@fedoraproject.org> 1.18.10-2.1
- Remove the patch for EPEL7

* Fri Feb 27 2015 Athmane Madjoudj <athmane@fedoraproject.org> 1.18.10-2
- Add a patch to use system-wide crypto-policies (RHBZ #1179277)

* Fri Feb 27 2015 Athmane Madjoudj <athmane@fedoraproject.org> 1.18.10-1
- Update to 1.18.10 (RHBZ #1123979)

* Fri Aug 15 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.18.6-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Sun Jul 13 2014 Rahul Sundaram <sundaram@fedoraproject.org> - 1.18.6-1
- update to 1.18.6

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.18.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Mon Dec 30 2013 Rahul Sundaram <sundaram@fedoraproject.org> - 1.18.2-1
- upstream release 1.18.2 (rhbz#967784)

* Sat Aug 03 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.17.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Thu May 02 2013 Rahul Sundaram <sundaram@fedoraproject.org> - 1.17.0-1
- update to 1.17.0
- drop upstream build patch
- switch to verbose make
- switch to make_install macro

* Wed Mar  6 2013 Tomáš Mráz <tmraz@redhat.com> - 1.16.1-2
- rebuilt with new gnutls

* Fri Jan 25 2013 Rahul Sundaram <sundaram@fedoraproject.org> - 1.16.1-1
- upstream release 1.16.1

* Wed Jul 18 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.14.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Wed Mar 21 2012 Tom Callaway <spot@fedoraproject.org> - 1.14.2-1
- update to 1.14.2
- fix compile issues with gcc 4.7

* Tue Feb 28 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.14.0-3
- Rebuilt for c++ ABI breakage

* Thu Jan 12 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.14.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Fri Dec 30 2011 Rahul Sundaram <sundaram@fedoraproject.org> - 1.14.0-1
- update to 1.14.0
- https://github.com/tatsuhiro-t/aria2/blob/3dc6d2ff6df5a33f6abf47b4e792ea7dd578cf9a/NEWS

* Mon Aug 15 2011 Rahul Sundaram <sundaram@fedoraproject.org> - 1.12.1-1
- https://github.com/tatsuhiro-t/aria2/commit/bd3956293995bcbbb76e6c8686b4ac8dfd3c9ed4#NEWS
- Additional man page

* Sun May 22 2011 Rahul Sundaram <sundaram@fedoraproject.org> - 1.11.2-1
- https://github.com/tatsuhiro-t/aria2/commit/f6625f8dc5557e77fcace9bedaf1815c5eaf763f#NEWS
- Drop defattr since it is set by default in recent rpm

* Sun Apr 10 2011 Rahul Sundaram <sundaram@fedoraproject.org> - 1.11.1-1
- https://github.com/tatsuhiro-t/aria2/blob/8d8fb31a45f66a29c78911293a60a00ac4903795/NEWS

* Tue Feb 22 2011 Rahul Sundaram <sundaram@fedoraproject.org> - 1.10.9-1
- https://github.com/tatsuhiro-t/aria2/blob/6af3cd82b36190b12102bf3fbc5c07cc494627ad/NEWS

* Mon Feb 07 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.10.8-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Wed Dec 29 2010 Rahul Sundaram <sundaram@fedoraproject.org> - 1.10.8-1
- https://github.com/tatsuhiro-t/aria2/blob/6af3cd82b36190b12102bf3fbc5c07cc494627ad/NEWS

* Sat Nov 27 2010 Rahul Sundaram <sundaram@fedoraproject.org> - 1.10.6-1
- http://aria2.svn.sourceforge.net/viewvc/aria2/trunk/NEWS?revision=2479

* Fri Jul 30 2010 Rahul Sundaram <sundaram@fedoraproject.org> - 1.10.0-1
- http://aria2.svn.sourceforge.net/viewvc/aria2/trunk/NEWS?revision=2279
- Dropped clean section

* Tue Jun 08 2010 Rahul Sundaram <sundaram@fedoraproject.org> - 1.9.4-1
- http://aria2.svn.sourceforge.net/viewvc/aria2/trunk/NEWS?revision=2133

* Sat Mar 20 2010 Rahul Sundaram <sundaram@fedoraproject.org> - 1.9.0-1
- http://aria2.svn.sourceforge.net/viewvc/aria2/trunk/NEWS?revision=1990

* Tue Feb 16 2010 Rahul Sundaram <sundaram@fedoraproject.org> - 1.8.2-1
- Several bug fixes
- http://aria2.svn.sourceforge.net/viewvc/aria2/trunk/NEWS?revision=1860

* Mon Dec 28 2009 Rahul Sundaram <sundaram@fedoraproject.org> - 1.8.0-1
- Many new features including XML RPC improvements and other bug fixes
- http://aria2.svn.sourceforge.net/viewvc/aria2/trunk/NEWS?revision=1778

* Mon Dec 07 2009 Rahul Sundaram <sundaram@fedoraproject.org> - 1.7.1-1
- Option --bt-prioritize-piece=tail will work again
- http://aria2.svn.sourceforge.net/viewvc/aria2/trunk/NEWS?revision=1721

* Wed Nov 04 2009 Rahul Sundaram <sundaram@fedoraproject.org> - 1.6.3-1
- Minor bug fixes
- http://aria2.svn.sourceforge.net/viewvc/aria2/trunk/NEWS?revision=1616

* Sat Oct 10 2009 Rahul Sundaram <sundaram@fedoraproject.org> - 1.6.2-1
- Minor bug fixes and switch XZ compressed source
- http://aria2.svn.sourceforge.net/viewvc/aria2/trunk/NEWS?revision=1586

* Thu Oct 08 2009 Rahul Sundaram <sundaram@fedoraproject.org> - 1.6.1-1
- Fixes memory leak in HTTP/FTP downloads and other minor bug fixes
- http://aria2.svn.sourceforge.net/viewvc/aria2/trunk/NEWS?revision=1569

* Wed Sep 23 2009 Rahul Sundaram <sundaram@fedoraproject.org> - 1.6.0-1
- Minor bug fixes
- http://aria2.svn.sourceforge.net/viewvc/aria2/trunk/NEWS?revision=1544

* Mon Aug 24 2009 Rahul Sundaram <sundaram@fedoraproject.org> - 1.5.2-1
- Minor bug fixes
- http://aria2.svn.sourceforge.net/viewvc/aria2/trunk/NEWS?revision=1504

* Mon Jul 27 2009 Rahul Sundaram <sundaram@fedoraproject.org> - 1.5.1-2
- update source

* Mon Jul 27 2009 Rahul Sundaram <sundaram@fedoraproject.org> - 1.5.1-1
- Minor bug fixes
- http://aria2.svn.sourceforge.net/viewvc/aria2/trunk/NEWS?revision=1494
- Fixed the license tag

* Sun Jul 26 2009 Rahul Sundaram <sundaram@fedoraproject.org> - 1.5.0-1
- Mostly minor bug fixes
- WEB-Seeding support for multi-file torrent
- http://aria2.svn.sourceforge.net/viewvc/aria2/trunk/NEWS?revision=1476

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.3.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Tue Apr 14 2009 Robert Scheck <robert@fedoraproject.org> - 1.3.1-1
- Upgrade to 1.3.1

* Mon Feb 23 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Fri Dec 05 2008 Michał Bentkowski <mr.ecik at gmail.com> - 1.0.1-2
- New version, 1.0.1
- Forgot to add changelog in last release...

* Tue Jun 24 2008 Tomas Mraz <tmraz@redhat.com> - 0.12.0-5
- rebuild with new gnutls

* Fri Feb 22 2008 Michał Bentkowski <mr.ecik at gmail.com> - 0.12.0-4
- Add patch

* Mon Feb 18 2008 Fedora Release Engineering <rel-eng@fedoraproject.org> - 0.12.0-3
- Autorebuild for GCC 4.3

* Mon Dec 31 2007 Michał Bentkowski <mr.ecik at gmail.com> - 0.12.0-2
- Get rid of odd locale.alias

* Mon Dec 31 2007 Michał Bentkowski <mr.ecik at gmail.com> - 0.12.0-1
- 0.12.0

* Thu Sep 20 2007 Michał Bentkowski <mr.ecik at gmail.com> - 0.11.3-1
- 0.11.3

* Fri Aug 24 2007 Michał Bentkowski <mr.ecik at gmail.com> - 0.11.2-1
- 0.11.2
- Fix License tag

* Mon Jul 09 2007 Michał Bentkowski <mr.ecik at gmail.com> - 0.11.1-1
- Update to 0.11.1

* Sat Apr 28 2007 Michał Bentkowski <mr.ecik at gmail.com> - 0.10.2+1-1
- Update to 0.10.2+1

* Tue Feb 20 2007 Michał Bentkowski <mr.ecik at gmail.com> - 0.10.1-1
- Update to 0.10.1

* Sun Dec 31 2006 Michał Bentkowski <mr.ecik at gmail.com> - 0.9.0-2
- Small fix in Summary

* Sat Dec 30 2006 Michał Bentkowski <mr.ecik at gmail.com> - 0.9.0-1
- Initial release
