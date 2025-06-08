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
TODO: revisit ^


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

# setting up box
you probably want to be able to ssh as root

```
ssh-copy-id $HOST
ssh $HOST
sudo bash -c 'cat .ssh/authorized_keys | tail -1 >> /root/.ssh/authorized_keys'
sudo useradd --system --home /nonexistent --shell /usr/sbin/nologin mydaemon
```

TODO: update linux params, worker2 has better
```
quiet splash isolcpus=1-3 nohz_full=1-3 rcu_nocbs=1-3 mitigations=off
```
in /etc/default/grub GRUB_CMDLINE_LINUX_DEFAULT

TODO: update via reboot script
TODO-ex: this is done via kernel params, look at worker2
post reboot:
```
for cpu in /sys/devices/system/cpu/cpu{1,2,3}/cpufreq/scaling_governor; do   echo performance | sudo tee $cpu; done
sudo bash -c 'echo 0 > /proc/sys/kernel/randomize_va_space'
```


interesting linux params:
irqaffinity
nokaslr



TODO:
add perf stat



BOOT_IMAGE=/vmlinuz-5.15.0-89-generic root=/dev/mapper/ubuntu--vg-ubuntu--lv ro quiet splash isolcpus=1-3 nohz_full=1-3 rcu_nocbs=1-3 mitigations=off nokalsr norandmaps intel_pstate=disable intel_idle.max_cstate=0 idle=poll cpufreq.default_governor=performance irqaffinity=1 vt.handoff=7


iptables -I INPUT -p tcp --dport 1022 -j ACCEPT



BOOT_IMAGE=(hd0,gpt2)/vmlinuz-6.12.0-66.el10.x86_64 root=/dev/mapper/cs-root ro crashkernel=2G-64G:256M,64G-:512M resume=UUID=6e4dbfc7-61ad-4748-958c-dd7f536506a8 rd.lvm.lv=cs/root rd.lvm.lv=cs/swap rhgb quiet


removed "intel_idle.max_cstate=0 idle=poll irqaffinity=1"
removed "nokalsr"