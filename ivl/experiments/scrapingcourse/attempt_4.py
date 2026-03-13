#!/usr/bin/env -S uv run

from camoufox.sync_api import Camoufox
from playwright.sync_api import TimeoutError

with Camoufox(
    headless=False,
    humanize=True,
    window=(1280, 720) # So that the Turnstile checkbox is on coordinate (210, 290)
) as browser:
    page = browser.new_page()

    # Visit the target page
    page.goto("https://www.scrapingcourse.com/cloudflare-challenge")

    # Wait for the Cloudflare Turnstile to appear and load
    page.wait_for_load_state(state="domcontentloaded")
    page.wait_for_load_state("networkidle")
    page.wait_for_timeout(1000)  # 5 seconds

    # Ckick the Turnstile checkbox (if it is present)
    page.mouse.click(210, 290+50)

    try:
        # Wait for the desired text to appear
        page.locator("text=You bypassed the Cloudflare challenge! :D").wait_for()
        challenge_bypassed = True
    except TimeoutError:
        # The text did not appear
        challenge_bypassed = False

    # Close the browser and release its resources
    browser.close()

    print("Cloudflare Bypassed:", challenge_bypassed)
