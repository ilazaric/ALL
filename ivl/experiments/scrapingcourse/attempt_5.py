#!/usr/bin/env -S uv run

from seleniumbase import Driver
from seleniumbase.common.exceptions import TextNotVisibleException

# Launch in undetected-chromedriver mode
driver = Driver(uc=True)
# Visit the target page
url = "https://www.scrapingcourse.com/cloudflare-challenge"
driver.uc_open_with_reconnect(url, 4)

# Click the Turnstile (if it is present) and reload the page
driver.uc_gui_click_captcha()

try:
    # Wait for the desired text to appear
    driver.wait_for_text("You bypassed the Cloudflare challenge! :D", "main")
    challenge_bypassed = True
except TextNotVisibleException:
      # The text did not appear
      challenge_bypassed = False

# Close the browser and release its resources
driver.quit()

print("Cloudflare Bypassed:", challenge_bypassed)
