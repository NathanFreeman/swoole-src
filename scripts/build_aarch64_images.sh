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
