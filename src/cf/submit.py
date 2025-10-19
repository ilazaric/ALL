#!/usr/bin/env python3

import os
import argparse
from pathlib import Path
# from seleniumwire import webdriver
from selenium.webdriver.common.by import By
# from selenium.webdriver.chrome.service import Service
from webdriver_manager.chrome import ChromeDriverManager
from selenium.webdriver.support.wait import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
import undetected_chromedriver as uc
from selenium.webdriver.support.ui import Select
import chromedriver_py
import chromedriver_binary

parser = argparse.ArgumentParser()
parser.add_argument('contestID')
parser.add_argument('taskName')
args = parser.parse_args()

contestID = args.contestID
taskName = args.taskName

os.chdir(Path(os.getcwd()).parent.absolute())

file_path = f'{os.getcwd()}/{contestID}/{taskName}.rendered.cpp'
url = f'https://codeforces.com/contest/{contestID}/problem/{taskName}'

def init_login(driver):
    with driver:
        driver.get('https://codeforces.com/enter')
    print(f'init_login: {driver.current_url = }')
    if driver.current_url == 'https://codeforces.com/enter':
        envHandle = os.environ["CF_USERNAME"]
        envPasswd = os.environ["CF_PASSWORD"]
        wait.until(EC.visibility_of_element_located((By.ID, 'handleOrEmail'))).send_keys(envHandle)
        wait.until(EC.visibility_of_element_located((By.ID, 'password'))).send_keys(envPasswd)
        checkbox = wait.until(EC.element_to_be_clickable((By.ID, 'remember')))
        if not checkbox.is_selected():
            checkbox.click()
        wait.until(EC.element_to_be_clickable((By.CSS_SELECTOR, '#enterForm > table > tbody > tr:nth-child(4) > td > div:nth-child(1) > input'))).click()
        wait.until(EC.visibility_of_element_located((By.CSS_SELECTOR, '#sidebar > div:nth-child(6) > div.personal-sidebar > div.for-avatar > div > div > a')))
        print(f'init_login: {driver.current_url = }')

options = uc.ChromeOptions() 
# options.headless = True
userAgent = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36'
print(userAgent)
options.add_argument(f"--user-agent={userAgent}")

# src/cf dir
# options.add_argument("--user-data-dir=browser_stuff")
options.add_argument("--disable-search-engine-choice-screen")
driver = uc.Chrome(options=options, driver_executable_path=chromedriver_binary.chromedriver_filename)
wait = WebDriverWait(driver, 10)
init_login(driver)
with driver:
    driver.get(url)

select = Select(wait.until(EC.visibility_of_element_located((By.NAME, 'programTypeId'))))
select.select_by_visible_text("GNU G++23 14.2 (64 bit, msys2)")

wait.until(EC.visibility_of_element_located((By.NAME, 'sourceFile'))).send_keys(file_path)
wait.until(EC.element_to_be_clickable((By.ID, 'sidebarSubmitButton'))).click()
wait.until(EC.visibility_of_element_located((By.CSS_SELECTOR, '#pageContent > div.datatable > div:nth-child(5)')))
driver.quit()

