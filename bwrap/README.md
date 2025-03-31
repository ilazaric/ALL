# scratch

systemd is pretty cool

man systemd.socket

Accept=yes
MaxConnections=1
^ this feels good enough, but probably not what I want
I probably want MaxConnections=200, and waiting for a lock (flock maybe)
locking might be annoying for SIGKILL
actually maybe not

systemd-analyze syscall-filter looks really interesting
@default
@basic-io
@debug
@file-system
@process
@resources
@sandbox
@signal
@sync



user --\         /-- worker
        \       /
user -- "gateway" -- worker
        /       \
user --/         \-- worker


!!! need to remember to cgroup worker onto non-zero cores


wifi pass: 9JKA6JHG



protocol specification:
json\nfilefilefile...
json specifies file sizes

"init" process always?: "/wdir/init"
maybe just file[0] is always init

json:
{
  "files": [
    {"path":"/path/where/to/write/it", "size":"in-bytes-string-to-not-do-weird-double-stuff"},
    ...
  ]
}

!!! need to protect myself against too much stdout


actually protocol could be json\ntarball
or just tarball?
sure seems simplest
