#!/usr/bin/env -S uv run

from seleniumbase import SB
import time

with SB(uc=True,
        headed=True,
        xvfb=False,
        user_data_dir="foo") as sb:
    url = "http://localhost:7070/website.html"
    sb.open(url)
    print(sb.find_element("#foo").text)
    print(sb.get_cookies())
    sb.open(url)
    
    
    
