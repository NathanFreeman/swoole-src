#!/bin/sh -e
if [ "$(uname -m)" = "aarch64" ]; then
  curl -fsSL https://mirrors.cloud.tencent.com/ubuntu-ports/project/ubuntu-archive-keyring.gpg -o /usr/share/keyrings/ubuntu-archive-keyring.gpg
  tee /etc/apt/sources.list.d/ubuntu.sources >/dev/null <<EOL
Types: deb
URIs: https://mirrors.cloud.tencent.com/ubuntu-ports/
Suites: noble noble-updates noble-backports
Components: main restricted universe multiverse
Signed-By: /usr/share/keyrings/ubuntu-archive-keyring.gpg
EOL
  cat /etc/apt/sources.list.d/ubuntu.source
  arch="-arm64"
  apt clean
  apt update
  apt install -y dpkg
else
  arch="x64"
  apt update
fi

apt install -y libaio-dev libaio1 sqlite3 libsqlite3-dev unixodbc unixodbc-dev libzstd-dev odbc-mariadb
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
