
Instructions for compiling 

1. build c-vtapi https://github.com/VirusTotal/c-vtapi
 *  autoreconf -fi
 * ./configure --prefix=$HOME/local
 *  make install

2. Build VirusTotal Uploader
 *  Install QT 5.3 or newer http://qt-project.org/downloads
 *  qmake
 *  make
