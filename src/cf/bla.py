#!/ usr / bin / env python3

from selenium import webdriver from selenium_stealth import stealth import random from
  selenium.webdriver.common.by import By from selenium.webdriver.support
    .wait import WebDriverWait from selenium.webdriver.support import expected_conditions as EC

      options = webdriver.ChromeOptions()

                  user_agents =
  [
#Your list of user agents goes here
    'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/127.0.0.0 Safari/537.36',
#More user agents
  ]

  user_agent = random.choice(user_agents) options.add_argument(f "user-agent={user_agent}")
                 options.add_argument("--headless=new")

#Initialize the WebDriver with options
                   driver =
    webdriver
      .Chrome(options = options)

#Apply stealth settings to the driver
        stealth(driver, languages = [ "en-US", "en" ], vendor = "Google Inc.", platform = "Win32",
                webgl_vendor = "Intel Inc.", renderer = "Intel Iris OpenGL Engine",
                fix_hairline = True, )

#driver.get("https://quotes.toscrape.com/")

# #Your scraping code goes here, e.g.:
#quotes  = driver.find_elements(By.CSS_SELECTOR, ".quote .text")
#authors = driver.find_elements(By.CSS_SELECTOR, ".quote .author")

#for quote, author in zip(quotes, authors):
#print(f "{quote.text} - {author.text}")

          driver.get("https://codeforces.com/") wait =
      WebDriverWait(driver, 5) wait
        .until(EC.visibility_of_element_located(
          (By.CSS_SELECTOR, '#sidebar > div:nth-child(1) > div.caption.titled')))
          print(driver.page_source)

            driver.quit()
