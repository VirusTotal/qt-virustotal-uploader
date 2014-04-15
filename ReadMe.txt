build c-vtapi
autoreconf -fi
./configure --prefix=$HOME/local
make install
