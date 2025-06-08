#!/usr/bin/env bash

set -euo pipefail
set -x

host="$1"           ; shift
input_tarball="$1"  ; shift
output_tarball="$1" ; shift

tmpdir=$(mktemp -d)

nc -N $host < $input_tarball > $tmpdir/fulloutput

# TODO: this impl should be streamy
mkdir $tmpdir/outputdir
mkdir $tmpdir/outputdir/outputs
touch $tmpdir/outputdir/bwrap.log
while IFS= read -r line
do
    if [[ $line == BWRAP* ]]
    then
        echo "$line" >> $tmpdir/outputdir/bwrap.log
        continue
    fi
    # TODO: RUNNER, including file
done < $tmpdir/fulloutput

unit="$(head -1 $tmpdir/fulloutput | cut -d "'" -f 2)"
ssh $host -- journalctl -u "$unit"
