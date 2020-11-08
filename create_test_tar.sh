#!/bin/sh
mkdir -p /tmp/tsh_test
cd /tmp/tsh_test
rm -rf *
mkdir -p dir1/subdir/subsubdir man_dir dir2
truncate -s 750 toto
truncate -s 50 titi
ln -s titi titi_link
mkdir -p access/no_x_dir
touch access/x access/no access/no_x_dir/a

truncate -s 10M dir1/tata
echo "Hello World!" > dir1/subdir/subsubdir/hello
man tar > man_dir/tar
man 2 open > man_dir/open2
man man > man_dir/man
touch dir2/fic1 dir2/fic2
tar cf test.tar toto dir1 titi man_dir titi_link dir2/*
tar -rf test.tar --mode='u=x' access/x
tar -rf test.tar --mode='u=' access/no
tar -rf test.tar --mode='u-x' access/no_x_dir/
tar -rf test.Tar --mode='u=rwx' access/no_x_dir/a
find . ! -name test.tar -delete
