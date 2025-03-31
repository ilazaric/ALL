#!/usr/bin/env bash

set -euo pipefail

tmpdir=$(mktemp -d)
cd $tmpdir
echo "#!/usr/bin/env bash" >> init
echo "set -euo pipefail"   >> init
echo 'echo "stdout"'       >> init
echo 'echo "stderr" 1>&2'  >> init
chmod +x init
tar -c -f inball.tar init
nc -N localhost 12345 < inball.tar > outball.tar.zst
unzstd -q outball.tar.zst
tar xf outball.tar
diff stdout <(echo "stdout")
diff stderr <(echo "stderr")
rm -rf $tmpdir
echo "SUCCESS"
