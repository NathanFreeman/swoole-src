#!/bin/sh -e
apt update && apt install -y gnupg2
tee /etc/apt/sources.list >/dev/null <<EOL
deb https://mirrors.cloud.tencent.com/ubuntu-ports/ noble main restricted universe multiverse
deb-src https://mirrors.cloud.tencent.com/ubuntu-ports/ noble main restricted universe multiverse
deb https://mirrors.cloud.tencent.com/ubuntu-ports/ noble-security main restricted universe multiverse
deb-src https://mirrors.cloud.tencent.com/ubuntu-ports/ noble-security main restricted universe multiverse
deb https://mirrors.cloud.tencent.com/ubuntu-ports/ noble-updates main restricted universe multiverse
deb-src https://mirrors.cloud.tencent.com/ubuntu-ports/ noble-updates main restricted universe multiverse
deb https://mirrors.cloud.tencent.com/ubuntu-ports/ noble-backports main restricted universe multiverse
deb-src https://mirrors.cloud.tencent.com/ubuntu-ports/ noble-backports main restricted universe multiverse
EOL

apt-key adv --keyserver keyserver.ubuntu.com --recv 871920D1991BC93C
apt update
apt install -y unixodbc-dev libaio-dev libaio1 sqlite3 libsqlite3-dev libzstd-dev odbc-maraidb

if [ "$(uname -m)" = "aarch64" ]; then
  arch="-arm64"
else
  arch="x64"
fi

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
