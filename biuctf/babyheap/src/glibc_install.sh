export glibc_install="$(pwd)/glibc/build/install"

git clone git://sourceware.org/git/glibc.git
cd glibc
git checkout glibc-2.27
mkdir build
cd build
../configure --prefix "$glibc_install" --enable-cet --disable-selinux --without-selinux
make -j $(nproc) CC=gcc-8
make install -j $(nproc) CC=gcc-8
