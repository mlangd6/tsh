#!/bin/bash
mkdir -p /tmp/tsh_test
cd /tmp/tsh_test
rm -rf *
mkdir -p dir1/subdir/subsubdir man_dir
truncate -s 750 toto
truncate -s 50 titi
ln -s titi titi_link
truncate -s 10M dir1/tata
echo "Hello World!" > dir1/subdir/subsubdir/hello
man tar > man_dir/tar
man 2 open > man_dir/open2
man man > man_dir/man
tar cvf test.tar * > /dev/null
find . ! -name test.tar -delete
