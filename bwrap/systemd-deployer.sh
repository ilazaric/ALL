#!/usr/bin/env bash

set -euo pipefail
set -x

host="$1" ; shift
dest="root@$host"

scp systemd/mydaemon.socket "$dest":/etc/systemd/system/
scp systemd/mydaemon@.service "$dest":/etc/systemd/system/
scp systemd/runner "$dest":/usr/local/bin/mydaemon-runner

ssh "$dest" -- systemctl stop mydaemon.socket
ssh "$dest" -- systemctl daemon-reload
ssh "$dest" -- systemctl start mydaemon.socket
ssh "$dest" -- systemctl status mydaemon.socket

# ssh "$dest" -- setcap cap_sys_nice=eip /usr/local/bin/chrt
# ssh "$dest" -- setcap cap_sys_nice=eip /usr/local/bin/mydaemon-runner

# ssh "$dest" -- 'echo 1 > /proc/irq/default_smp_affinity'
