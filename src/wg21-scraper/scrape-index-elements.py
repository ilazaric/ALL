#!/usr/bin/env python3

import json
import sys
import logging
import grequests
import os

parallelism_limit = 30

log = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(name)s - %(levelname)s - %(message)s")

os.chdir(os.path.dirname(os.path.abspath(__file__)))
if not os.path.isdir("index-elements"):
    os.mkdir("index-elements")

out = grequests.map((grequests.get("https://wg21.link/index.json"),))[0]
index = out.json()

log.debug(f'{len(index) = }')

no_link_elems = [k for k in index if "link" not in index[k]]
log.info(f'{len(no_link_elems) = }')
if len(no_link_elems) != 0:
    log.error("found elements with zero links, this is really bad")
    log.error(f"{len(no_link_elems) = }")
    log.error(f"{no_link_elems = }")
    log.error("i refuse to continue")
    # TODO: we probably should warn on these elements, filter them out, continue
    exit(1)

def chunks(l, n):
    for i in range(0, len(l), n):
        yield l[i:i+n]

def solve(els):
    urls = [index[el]["link"] for el in els]
    rs = (grequests.get(u) for u in urls)
    outs = grequests.map(rs)
    for idx, o in enumerate(outs):
        if o is None:
            log.error(f"something weird happened")
            log.error(f"url: {index[els[idx]]["link"]}")
            continue
        if o.status_code == 401:
            log.error(f"unauthorized: {o.url}")
            log.error(f"original url: {index[els[idx]]["link"]}")
            continue
        if o.status_code != 200:
            log.error(f"seen bad status code: {o.status_code}")
            log.error(f"url: {o.url}")
            log.error(f"original url: {index[els[idx]]["link"]}")
            log.error(f"reason: {o.reason}")
            if o.status_code == 404: continue
            log.error("we might be getting throttled")
            log.error("shutting down as likely outcome is restart after a couple of minutes")
            exit(1)
    for i in range(len(els)):
        if outs[i] is None:
            continue
        if outs[i].status_code == 200:
            with open(f'index-elements/{els[i]}', 'wb') as f:
                if hasattr(outs[i], 'content'):
                    f.write(outs[i].content)

els = [el for el in index if not os.path.isfile(f'index-elements/{el}')]
log.info(f'{len(els)=}')

els = [el for el in els if "wiki.edg.com" not in index[el]["link"]]
log.info(f'after filtering edg wiki: {len(els)=}')

els = [el for el in els if "www.open-std.org/jtc1/sc22/wg21/prot" not in index[el]["link"]]
log.info(f'after filtering jtc1: {len(els)=}')

els = [el for el in els if "wg21.link/lewg" not in index[el]["link"]]
log.info(f'after filtering lewg: {len(els)=}')

# bla = len([el for el in els if "github.com" in index[el]["link"]])
# log.info(f"github count: {bla}")

# for el in els[:parallelism_limit]:
#     log.info(index[el]["link"])
# exit(1)

for i, chunk in enumerate(chunks(els, parallelism_limit)):
    log.info(f'solving chunk #{i} ...')
    solve(chunk)

