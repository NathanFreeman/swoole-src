#!/bin/sh -e
if [ "$(uname -m)" = "aarch64" ]; then
  arch="-arm64"
  mirror="https://mirrors.cloud.tencent.com/ubuntu-ports/"
else
  arch="x64"
  mirror="https://mirrors.cloud.tencent.com/ubuntu/"
fi

tee /etc/apt/sources.list >/dev/null <<EOL
deb ${mirror} noble main restricted universe multiverse
deb-src ${mirror} noble main restricted universe multiverse
deb ${mirror} noble-security main restricted universe multiverse
deb-src ${mirror} noble-security main restricted universe multiverse
deb ${mirror} noble-updates main restricted universe multiverse
deb-src ${mirror} noble-updates main restricted universe multiverse
deb ${mirror} noble-backports main restricted universe multiverse
deb-src ${mirror} noble-backports main restricted universe multiverse
EOL

wget -O /etc/apt/trusted.gpg.d/ubuntu-archive-keyring.gpg ${mirror}project/ubuntu-archive-keyring.gpg
apt update
apt install --only-upgrade dpkg
apt install -y unixodbc-dev libaio-dev libaio1t64 sqlite3 libsqlite3-dev libzstd-dev odbc-mariadb
wget -nv https://download.oracle.com/otn_software/linux/instantclient/instantclient-basiclite-linux${arch}.zip
unzip instantclient-basiclite-linux${arch}.zip && rm instantclient-basiclite-linux${arch}.zip
wget -nv https://download.oracle.com/otn_software/linux/instantclient/instantclient-sdk-linux${arch}.zip
unzip instantclient-sdk-linux${arch}.zip && rm instantclient-sdk-linux${arch}.zip
mv instantclient_*_* ./instantclient
rm ./instantclient/sdk/include/ldap.h
# fix debug build warning: zend_signal: handler was replaced for signal (2) after startup
echo DISABLE_INTERRUPT=on >./instantclient/network/admin/sqlnet.ora
mv ./instantclient /usr/local/
echo '/usr/local/instantclient' >/etc/ld.so.conf.d/oracle-instantclient.conf
ldconfig

wget https://github.com/axboe/liburing/archive/refs/tags/liburing-2.6.tar.gz
tar zxf liburing-2.6.tar.gz
cd liburing-liburing-2.6 && ./configure && make -j$(cat /proc/cpuinfo | grep processor | wc -l) && make install
