#!/usr/bin/env -S uv run

import requests

url = 'https://www.scrapingcourse.com/cloudflare-challenge'

# nope, 403
r = requests.get(url)
print(f"{r.status_code = }")


