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

mkdir -p dir1_bis/subdir_bis/subsubdir_bis man_dir_bis dir2_bis
truncate -s 750 toto_bis
truncate -s 50 titi_bis
ln -s titi_bis titi_bis_link
mkdir -p access_bis/no_x_dir_bis
touch access_bis/x_bis access_bis/no_bis access_bis/no_x_dir_bis/a_bis

truncate -s 10M dir1_bis/tata_bis
echo "Hello World!" > dir1_bis/subdir_bis/subsubdir_bis/hello_bis
man tar > man_dir_bis/tar_bis
man 2 open > man_dir_bis/open2_bis
man man > man_dir_bis/man_bis
touch dir2_bis/fic1_bis dir2_bis/fic2_bis
tar cf bis_test.tar toto_bis dir1_bis titi_bis man_dir_bis titi_bis_link dir2_bis/*
tar -rf bis_test.tar --mode='u=x' access_bis/x_bis
tar -rf bis_test.tar --mode='u=' access_bis/no_bis
tar -rf bis_test.tar --mode='u-x' access_bis/no_x_dir_bis/
tar -rf bis_test.tar --mode='u=rwx' access_bis/no_x_dir_bis/a_bis

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
tar -rf test.tar --mode='u=rwx' access/no_x_dir/a
find . ! -name '*.tar' -delete






