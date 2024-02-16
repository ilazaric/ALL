#!/usr/bin/env python3

import os
import time
import argparse
from pathlib import Path
from seleniumwire import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.chrome.service import Service
from webdriver_manager.chrome import ChromeDriverManager
from selenium.webdriver.support.wait import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC

parser = argparse.ArgumentParser()
parser.add_argument('contestID')
parser.add_argument('taskName')
args = parser.parse_args()

contestID = args.contestID
taskName = args.taskName

os.chdir(Path(os.getcwd()).parent.absolute())

file_path = f'{os.getcwd()}/{contestID}/{taskName}.rendered.cpp'
url = f'https://codeforces.com/contest/{contestID}/problem/{taskName}'

def interceptor(request):
    if 'User-Agent' in request.headers:
        del request.headers['User-Agent']
    request.headers['User-Agent'] = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36'

def init_login():
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

options = webdriver.ChromeOptions()
options.add_argument("--headless=new")
# src/cf dir
options.add_argument(f"user-data-dir=browser_stuff")
driver = webdriver.Chrome(service=Service(ChromeDriverManager().install()), options=options)
driver.request_interceptor = interceptor
wait = WebDriverWait(driver, 5)
init_login()
driver.get(url)

wait.until(EC.visibility_of_element_located((By.NAME, 'sourceFile'))).send_keys(file_path)
wait.until(EC.element_to_be_clickable((By.ID, 'sidebarSubmitButton'))).click()
wait.until(EC.visibility_of_element_located((By.CSS_SELECTOR, '#pageContent > div.datatable > div:nth-child(5)')))
driver.quit()
exit(0)
