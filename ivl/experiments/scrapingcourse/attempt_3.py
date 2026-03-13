#!/usr/bin/env -S uv run

from playwright.sync_api import sync_playwright
from playwright.sync_api import TimeoutError

with sync_playwright() as p:
    browser = p.chromium.launch(headless=True)
    page = browser.new_page()

    # Go to the target page
    page.goto("https://www.scrapingcourse.com/cloudflare-challenge")

    try:
        # Wait for the desired text to be on the page
        page.locator("text=You bypassed the Cloudflare challenge! :D").wait_for()
        challenge_bypassed = True
    except TimeoutError:
        challenge_bypassed = False

    # Close the browser and release its resources
    browser.close()

    print("Cloudflare Bypassed:", challenge_bypassed)
