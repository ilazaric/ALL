#!/usr/bin/env bash

set -euo pipefail

curl 'https://codeforces.com' \
  -H 'accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7' \
  -H 'accept-language: en-US,en;q=0.9' \
  -H 'cache-control: max-age=0' \
  -H 'cookie: JSESSIONID=CDEC930173472E1E0A4BA767A57C43B1; 39ce7=CFW64m73; cf_clearance=3OaYKKZJeaaq5oWhhnV1Gib.fn5H6NhKP9NFeHgxRHs-1725816302-1.2.1.1-H518j6Ip4vFXSZCX_LSw8oxufNG1zVJGjNyNSykjnIYrCfL4uV1PEZp7otva1Eh2rqSeZ.Hnqd6EzWhO1RJzMrAofoCI.NDr9dDm6BPMwIfbAGXrO0p.17HA3KrRpQIMiQPK538btOJEnTFTskvmE6gIzJKff_mkfvaevkX_wx2PFzxJu0IJaP9q0YDrVb6ukHlJIEOqfccUvVGLYVoUV00R3yPtdzKP8pB088qgvmkxpDdnHFGXlWeLJKcORw.tcUZUoQd.mWJzituKOH.47JoP0c.hLs8y4JR7YeyFPiDwrgZDXoIcNMxBEc6nwIkNEUALzeWmbUGg8Q2XPd0aUR4GzHgF5dx5oRsUQ93NSHcGarhVa2kp_JK7pqAi3vT3oMRFwTeA4O7AJEjT5Hv0GQ; pow=2cb966189eed9a261e70f9153460bab93c581c72; evercookie_png=3hgnlqlvmxb8c6q5ha; evercookie_etag=3hgnlqlvmxb8c6q5ha; evercookie_cache=3hgnlqlvmxb8c6q5ha; 70a7c28f3de=3hgnlqlvmxb8c6q5ha' \
  -H 'priority: u=0, i' \
  -H 'sec-ch-ua: "Not)A;Brand";v="99", "Google Chrome";v="127", "Chromium";v="127"' \
  -H 'sec-ch-ua-mobile: ?0' \
  -H 'sec-ch-ua-platform: "Windows"' \
  -H 'sec-fetch-dest: document' \
  -H 'sec-fetch-mode: navigate' \
  -H 'sec-fetch-site: none' \
  -H 'sec-fetch-user: ?1' \
  -H 'upgrade-insecure-requests: 1' \
  -H 'user-agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/127.0.0.0 Safari/537.36'
