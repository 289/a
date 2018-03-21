#!/bin/bash
mkdir -p ./gen

proto_file_name=*.proto

for f in $(find . -type f -name "$proto_file_name")
do
	path=$(dirname $f)
	echo $f
	protoc --cpp_out=./gen $f
	if  [ $path != . ]
	then
		cp ./makefile_template ./gen/$path/Makefile
	fi
done

echo gen success!
