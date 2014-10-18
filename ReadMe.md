# VirusTotal uploader
This program internally uses the VirusTotal [public API](https://www.virustotal.com/documentation/public-api/). You can drag and drop a file or folder into the program to queue it for uploading and scanning


## Instructions for compiling and Install

1. build c-vtapi https://github.com/VirusTotal/c-vtapi
 *  autoreconf -fi
 * ./configure
 *  make
 *  sudo make install

2. Build VirusTotal Uploader
 *  Install QT 5.3 or newer http://qt-project.org/downloads
 *  qmake
 *  make
 *  sudo make install
