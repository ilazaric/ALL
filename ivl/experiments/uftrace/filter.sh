#!/usr/bin/env bash

set -euo pipefail

cat filtered-wide-record.sh | grep '^-P ' | rev | cut -c 3- | rev | sponge | \
    while IFS= read -r line; do
        echo "$line"
        grep -v -e "$line" filtered-wide-record.sh > attempt.sh
        chmod +x attempt.sh
        if ! ./attempt.sh &> /dev/null
        then
            cp attempt.sh filtered-wide-record.sh
        fi
    done
