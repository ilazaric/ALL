#!/usr/bin/env bash

set -euo pipefail
set -x

function files() {
    ls | grep -E '^never-fade-away\.[^.]+$'
}

if ! files &> /dev/null
then
    yt-dlp -f bestaudio -o 'never-fade-away.%(ext)s' 'https://www.youtube.com/watch?v=AN1RJF55NXI'
fi

if ! [ -f 'never-fade-away.wav' ]
then
    [ "$(files | wc -l)" -eq "1" ]
    ffmpeg -i "$(files)" 'never-fade-away.wav'
fi
