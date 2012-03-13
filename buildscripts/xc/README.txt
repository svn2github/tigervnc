DRC's Cross-Compatible TigerVNC Build Scripts
=============================================

These scripts are used to build a cross-compatible TigerVNC build that works
on any Linux platform with GLIB 2.5 and later, as well as Windows XP and later
and OS X 10.4 and later.


Build Environment: Linux
------------------------

Recommended distro:  Red Hat or CentOS Enterprise Linux 4 64-bit

CMake 2.8 or later

JDK (http://www.oracle.com/technetwork/java/javase/downloads) or OpenJDK
(http://openjdk.java.net/) 1.5 or later.  Make sure that keytool and jarsigner
are in the PATH (either sym link them from /usr/java/default/bin to /usr/bin or
just add /usr/java/default/bin to the PATH.)

64-bit and 32-bit libjpeg-turbo SDK's
(https://sourceforge.net/projects/libjpeg-turbo/files/) installed in the
default location (/opt/libjpeg-turbo)

As root, configure the libgpg-error, libtasn1, libgcrypt, and GnuTLS source
with the following arguments, then 'make install':
  CC=gcc4 CXX=g++4 CFLAGS=-O3 CXXFLAGS=-O3 --enable-static --with-pic \
    --prefix=/opt/gnutls/linux64/

As root, configure the libgpg-error, libtasn1, libgcrypt, and GnuTLS source
with the following arguments, then 'make install'
  --host i686-pc-linux-gnu CC=gcc4 CXX=g++4 CFLAGS='-m32 -O3' \
    CXXFLAGS='-m32 -O3' --enable-static --with-pic prefix=/opt/gnutls/linux/

NOTE:  If building on a distro that already has GCC 4, you can leave out the
'CC=gcc4 CXX=g++4' portion of the above command lines.  You should also edit
buildtiger.linux and remove that same text from the command lines in there as
well.  Or, you could just create sym links from gcc4 -> gcc and g++4 -> g++.


Build Environment: OS X
-----------------------

Recommended version:  OS X 10.6 (Snow Leopard)

Xcode 3.2 (available at https://developer.apple.com/downloads -- Apple ID
required)

CMake 2.8 or later installed somewhere in the PATH.  The version in MacPorts
(http://www.MacPorts.org) works, or just install the CMake application from
the DMG (http://www.cmake.org) and sym link
/Applications/CMake\ {version}.app/Contents/bin/cmake to a directory in your
PATH.

libjpeg-turbo SDK (https://sourceforge.net/projects/libjpeg-turbo/files/)
installed in its default location (/opt/libjpeg-turbo)

Configure the libgpg-error, libtasn1, libgcrypt, and GnuTLS source with the
following arguments, then 'make', then 'sudo make install':
  --host x86_64-apple-darwin \
    CFLAGS='-isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -O3' \
    CXXFLAGS='-isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -O3' \
    LDFLAGS='-isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5' \
    --prefix /opt/gnutls/osxx8664

Configure the libgpg-error, libtasn1, libgcrypt, and GnuTLS source with the
following arguments, then 'sudo make install':
  CFLAGS='-I/usr/lib/gcc/i686-apple-darwin10/4.2.1/include -isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4 -O3 -m32' \
  CXXFLAGS='-I/usr/include/c++/4.2.1 -I/usr/include/c++/4.2.1/i686-apple-darwin10 -I/usr/lib/gcc/i686-apple-darwin10/4.2.1/include -isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4 -O3 -m32' \
  LDFLAGS='-isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4 -m32' \
  --prefix /opt/gnutls/osxx86

Configure the gettext source with the following arguments, then 'make', then
'sudo make install':
  --host x86_64-apple-darwin \
    CFLAGS='-isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -O3' \
    CXXFLAGS='-isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -O3' \
    LDFLAGS='-isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5' \
    --prefix /opt/gettext/osxx8664

Configure the gettext source with the following arguments, then 'make, then
'sudo make install':
  CFLAGS='-I/usr/lib/gcc/i686-apple-darwin10/4.2.1/include -isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4 -O3 -m32' \
  CXXFLAGS='-I/usr/include/c++/4.2.1 -I/usr/include/c++/4.2.1/i686-apple-darwin10 -I/usr/lib/gcc/i686-apple-darwin10/4.2.1/include -isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4 -O3 -m32' \
  LDFLAGS='-isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4 -m32' \
  --prefix /opt/gettext/osxx86


Build Environment: Windows
--------------------------

Recommended O/S:  Windows XP 64-bit

MinGW (http://www.mingw.org/) installed in c:\MinGW

MinGW-w64 (http://mingw-w64.sourceforge.net/) unzipped into c:\MinGW\MinGW64

JDK (http://www.oracle.com/technetwork/java/javase/downloads) 1.5 or later

CMake 2.8 or later (http://www.cmake.org).  Add the CMake directory (e.g.
c:\program files (x86)\cmake 2.8\bin) to the PATH.

64-bit and 32-bit libjpeg-turbo SDK's for GCC
(https://sourceforge.net/projects/libjpeg-turbo/files/) installed in the
default locations (c:\libjpeg-turbo-gcc64 and c:\libjpeg-turbo-gcc)

GnuTLS for Windows (http://josefsson.org/gnutls4win/) installed in c:\GnuTLS

NOTE: It should be possible to cross-compile the Windows binaries on Linux as
well, but setting that up is left as an exercise for the reader.


Build Procedure
---------------

Executing

  buildtiger <repository path>

(where repository path is, for instance, "trunk" or "branches/1_2") will
generate both a pristine source tarball and binaries for the platform on which
the script is executed.  These are placed under
$HOME/src/tiger.nightly/YYYYMMDD/files, where YYYYMMDD is a build number based
on today's date.  If the build is successful, then a sym link will be created
from $HOME/src/tiger.nightly/latest to $HOME/src/tiger.nightly/YYYYMMDD.

Once a full build is completed on one platform, then you can use the existing
source tarball to build binaries on other platforms by running 'buildtiger -e'
(assuming that $HOME/src is a shared directory.)

NOTE: On Windows, buildtiger should be run from inside a MinGW shell.  If you
are mounting your home directory as a drive letter (e.g. H:), then set the HOME
environment variable to the MinGW path for that drive (e.g. /h) prior to
running buildtiger.

Run 'buildtiger -h' for usage information.


Distributing a Pre-Release Build
--------------------------------

Executing

  uploadtiger <SourceForge user name>

will upload the files from $HOME/src/tiger.nightly/latest/files to
http://tigervnc.sourceforge.net/tiger.nightly

Having an SSH key installed is highly recommended to avoid having to enter your
password multiple times.
