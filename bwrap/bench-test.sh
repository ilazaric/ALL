#!/usr/bin/env bash

set -euo pipefail
# set -x

host="$1" ; shift
# ssh root@$host -- logrotate -f /etc/logrotate.conf

tmpdir=$(mktemp -d)
cp "$@" $tmpdir/
cd $tmpdir
echo "#!/usr/bin/env bash"                            >> init
echo 'taskset -a -pc 2 $$'                            >> init
echo 'export PATH="/home/linuxbrew/.linuxbrew/bin:$PATH"' >> init
echo "set -euo pipefail"                              >> init
echo "set -x"                                         >> init
echo "for src in *.cpp"                               >> init
echo "do"                                             >> init
echo '  g++ -O3 -std=c++23 $src -c -o /tmp/$src.o' >> init
echo "done"                                           >> init
echo "g++ -fuse-ld=mold /tmp/*.cpp.o -o /tmp/exe"  >> init
echo "gcc -O3 stupid-alloc.c -c -fPIC -o /tmp/stupid-alloc.o" >> init
echo "gcc -fuse-ld=mold -shared /tmp/stupid-alloc.o -o /tmp/stupid-alloc.so" >> init
echo "ls \$PWD" >> init
echo "ls -lah /dev/myreservedmem" >> init
# echo "env LD_PRELOAD=/tmp/stupid-alloc.so /tmp/exe"   >> init
echo "time perf stat -d taskset -c 3 env LD_PRELOAD=/tmp/stupid-alloc.so /tmp/exe"   >> init
# echo "time taskset -c 1 /usr/local/bin/chrt --rr 50 /tmp/exe"   >> init
chmod +x init
tar -c -f inball.tar ./*
nc -N "$host" 12345 < inball.tar > fullout

echo "full output:"
cat fullout
echo

# diff <(cat fullout | grep STDOUT) <(echo "BWRAP-STDOUT: stdout")
# diff <(cat fullout | grep STDERR) <(echo "BWRAP-STDERR: stderr")
# diff <(cat fullout | grep EXIT) <(echo "BWRAP-EXITCODE: 0")
# echo "SUCCESS"
# rm -rf $tmpdir

