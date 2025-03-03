#!/bin/sh
git clone https://github.com/fauria/docker-vsftpd.git
cd ./docker-vsftpd && docker build . && cd -

git clone https://github.com/swoole/golang-h2demo.git
apt install -y golang
cd ./golang-h2demo && GOOS=linux GOARCH=arm64 go build -o h2demo . && docker build . -t phpswoole/golang-h2demo && cd -

git clone https://github.com/vimagick/dockerfiles.git
cd ./dockerfiles/tinyproxy && docker build . && cd -

git clone https://github.com/postmanlabs/httpbin.git
cd ./httpbin && docker build . -t kennethreitz/httpbin && cd -

mkdir brook-src && cd brook-src && wget https://github.com/txthinking/brook/releases/download/v20250202/brook_linux_arm5
mv brook_linux_arm5 brook
chmod 0755 brook
cat <<EOF >Dockerfile
FROM ubuntu:latest

RUN mkdir /brook-src
COPY brook /brook-src/brook
EXPOSE 1080
CMD ["/brook-src/brook", "socks5", "--listen", "0.0.0.0:1080"]
EOF
docker build . -t swoole/brook
