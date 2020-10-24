apt install libgnutls28-dev
apt install nettle-dev
apt install libgmp-dev
apt install libssh2-1-dev
apt install libc-ares-dev
apt install libxml2-dev
apt install zlib1g-dev
apt install libsqlite3-dev
apt install pkg-config

# You can use libgcrypt-dev instead of nettle-dev and libgmp-dev
#apt install libgcrypt-dev

apt install libgpg-error-dev
apt install libgcrypt-dev

# You can use libssl-dev instead of libgnutls-dev, nettle-dev, libgmp-dev, libgpg-error-dev and libgcrypt-dev

apt install libssl-dev

# You can use libexpat1-dev instead of libxml2-dev

apt install libexpat1-dev


apt install libxml2-dev
apt install libcppunit-dev
apt install autoconf
apt install automake
apt install autotools-dev
apt install autopoint
apt install libtool


autoreconf -i

./configure


apt install python3-pip
pip3 install sphinx

make
cp ./src/aria2c ../../
make clean


aria2c --enable-dht=true --seed-time=0 --seed-ratio=0.1 -j 7 