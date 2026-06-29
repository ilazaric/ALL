#!/usr/bin/env bash

set -euo pipefail
# set -x

function sync_file() {
    FILE="$1"
    if [ -f "/tmp$FILE" ]
    then
        return
    fi
    mkdir -p "/tmp$(dirname "$FILE")"
    cp "$FILE" "/tmp$FILE"
}

function sync_system_program() {
    PROGRAM="$(which "$1")"
    if [ -f "/tmp$PROGRAM" ]
    then
        return
    fi
    sync_file "$PROGRAM"
    for DEP in $(ldd "$PROGRAM" | tr '\t' ' ' | tr ' ' '\n' | grep '^/')
    do
        sync_file "$DEP"
    done
}

function sync_system_programs() {
    for PROGRAM in "$@"
    do
        sync_system_program "$PROGRAM"
    done
}

mount -t tmpfs -o size=512M tmpfs /tmp
# mkdir /tmp/old-root

sync_system_programs ls {,u}mount rm{,dir} tree df mkdir cat tr env awk grep ln

mkdir /tmp/dev
for f in null zero random urandom
do
    touch /tmp/dev/$f
    mount --bind {,/tmp}/dev/$f
done

cd /tmp
# pivot_root /tmp /tmp/old-root
pivot_root '.' '.'
cd /

mkdir /proc
mount -t proc proc /proc

# grep CapEff /proc/self/status

mkdir /sys
mount -t sysfs sysfs /sys
mkdir /tmp
mount -t tmpfs tmpfs /tmp

mkdir /dev/shm
mount -t tmpfs tmpfs /dev/shm

ln -s /proc/self/fd/0 /dev/stdin
ln -s /proc/self/fd/1 /dev/stdout
ln -s /proc/self/fd/2 /dev/stderr

# awk '$5 == "/" {print}' /proc/self/mountinfo
umount -l /
# awk '$5 == "/" {print}' /proc/self/mountinfo

cat /proc/self/mountinfo
exit 0

df -a

echo "pid: $$"

ls /
ls /proc
cat /proc/1/cmdline | tr '\0' ' '
echo
tree -I proc -I sys /
df
# env
# env -i
# env
