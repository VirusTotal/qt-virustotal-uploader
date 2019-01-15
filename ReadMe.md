# VirusTotal uploader
This program internally uses the VirusTotal [public API](https://www.virustotal.com/documentation/public-api/). You can drag and drop a file or folder into the program to queue it for uploading and scanning


## Generic Instructions for compiling and Install

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



## Ubuntu or debian instructions

```
# get dependencies
sudo apt-get install build-essential qtchooser qt5-default libjansson-dev libcurl4-openssl-dev git zlib1g-dev

# clone the c-vtapi library
git clone https://github.com/VirusTotal/c-vtapi.git

#change to c-vtapi directory
cd c-vtapi

# get c-vtapi dependencies
sudo apt-get install automake autoconf libtool libjansson-dev libcurl4-openssl-dev

# configure with default options and make
autoreconf -fi && ./configure && make

# install to system, by default this goes to /usr/local/lib
sudo make install 

# configure dynamic linker to add /usr/local/lib to path
sudo sh -c 'echo "/usr/local/lib" > /etc/ld.so.conf.d/usr-local-lib.conf'
sudo ldconfig

# go back to base directory
cd ..

#clone QT VirusTotal Uplaoder
git clone https://github.com/VirusTotal/qt-virustotal-uploader.git
cd qt-virustotal-uploader

# run qmake, specifing qt5 
qtchooser -run-tool=qmake -qt=5

# compile with 4 parellel jobs
make -j4

#optionally install 
sudo make install

```
