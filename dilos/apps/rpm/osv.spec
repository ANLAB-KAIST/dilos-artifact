Name: osv
Version: %{?osversion}
Summary: OSv base package
Release: 1
BuildArch: noarch

Group: kernel
License: BSD
URL: http://osv.io

Provides: libc.so.6()(64bit)
Provides: libc.so.6(GLIBC_2.14)(64bit)
Provides: libc.so.6(GLIBC_2.15)(64bit)
Provides: libc.so.6(GLIBC_2.2.5)(64bit)
Provides: libc.so.6(GLIBC_2.3)(64bit)
Provides: libc.so.6(GLIBC_2.3.2)(64bit)
Provides: libc.so.6(GLIBC_2.3.4)(64bit)
Provides: libc.so.6(GLIBC_2.4)(64bit)
Provides: libc.so.6(GLIBC_2.7)(64bit)
Provides: libgcc_s.so.1()(64bit)
Provides: libgcc_s.so.1(GCC_3.0)(64bit)
Provides: libm.so.6()(64bit)
Provides: libpthread.so.0()(64bit)
Provides: libpthread.so.0(GLIBC_2.2.5)(64bit)
Provides: libpthread.so.0(GLIBC_2.3.2)(64bit)
Provides: libpthread.so.0(GLIBC_2.3.3)(64bit)
Provides: libstdc++.so.6()(64bit)
Provides: libstdc++.so.6(CXXABI_1.3)(64bit)
Provides: libstdc++.so.6(GLIBCXX_3.4)(64bit)
Provides: rtld(GNU_HASH)
Provides: libm.so.6(GLIBC_2.2.5)(64bit)
Provides: libdl.so.2()(64bit)
Provides: libdl.so.2(GLIBC_2.2.5)(64bit)

# The ones below are technically lies. But they are mostly used during
# script execution, and we don't do that.
Provides: /sbin/ldconfig
Provides: /bin/bash
Provides: /bin/sh
Provides: /usr/sbin/alternatives

%description
Base package for an OSv system managed by RPM.


%files
%doc

%changelog
* Tue Apr 22 2014 Glauber Costa <glommer@cloudius-systems.com>
- OSv's base RPM package.

