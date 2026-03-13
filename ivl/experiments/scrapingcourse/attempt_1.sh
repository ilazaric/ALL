#!/usr/bin/env bash

set -x

# fails, need to be able to execute JS
curl -L 'https://www.scrapingcourse.com/cloudflare-challenge' | htmlq -p
