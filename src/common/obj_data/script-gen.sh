#!/bin/bash

mkdir -p ./gen

proto_file_name=*.proto

cd ./scalable
for f in $(find . -type f -name "$proto_file_name")
do
	echo ./scalable${f:1}
    protoc --cpp_out=../gen $f
done
cd ..

echo gen success!
