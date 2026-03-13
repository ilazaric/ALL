#!/usr/bin/env -S uv run

from seleniumbase import Driver, SB
from seleniumbase.common.exceptions import TextNotVisibleException
import time

last_time = time.time()
last_name = "BEGIN"
def time_mark(curr_name):
    global last_time
    global last_name
    curr_time = time.time()
    print(f"{curr_time-last_time:10.4f} -- elapsed between {last_name} and {curr_name}")
    last_time = curr_time
    last_name = curr_name

with SB(uc=True,
        xvfb=True,
        incognito=True,
        headed=True,
        xvfb_metrics="1920,1080",
        chromium_arg="ozone-platform=x11",
        ) as sb:
    time_mark("entry")
    url = "https://www.scrapingcourse.com/cloudflare-challenge"
    sb.uc_open_with_reconnect(url, 4)
    time_mark("open")

    # Click the Turnstile (if it is present) and reload the page
    sb.uc_gui_click_captcha()
    time_mark("captcha")

    try:
        # Wait for the desired text to appear
        sb.wait_for_text("You bypassed the Cloudflare challenge! :D", "main")
        challenge_bypassed = True
    except TextNotVisibleException:
        # The text did not appear
        challenge_bypassed = False
    time_mark("waited")

print("Cloudflare Bypassed:", challenge_bypassed)
time_mark("end")
