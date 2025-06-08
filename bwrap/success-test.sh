#!/usr/bin/env bash

set -euo pipefail

tmpdir=$(mktemp -d)
cd $tmpdir
echo "#!/usr/bin/env bash"    >> init
echo "set -euo pipefail"      >> init
echo 'echo "stdout"'          >> init
echo 'echo "stderr" 1>&2'     >> init
echo 'touch /outputs/done'    >> init
echo 'echo 42 > /outputs/ans' >> init
chmod +x init
tar -c -f inball.tar init
nc -N "$1" 12345 < inball.tar > fullout

echo "full output:"
cat fullout
echo

diff <(cat fullout | grep STDOUT) <(echo "BWRAP-STDOUT: stdout")
diff <(cat fullout | grep STDERR) <(echo "BWRAP-STDERR: stderr")
diff <(cat fullout | grep EXIT) <(echo "BWRAP-EXITCODE: 0")
echo "SUCCESS"
rm -rf $tmpdir

