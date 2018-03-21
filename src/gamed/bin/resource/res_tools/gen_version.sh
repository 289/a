#!/bin/bash
perl calcmd5sum.pl
cd ./assets/config
svn ci -m "" version.conf
cd ../..
