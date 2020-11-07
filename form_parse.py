import requests
import json
import os
import os.path
from multiprocessing.pool import ThreadPool
import re

def fetch_fragment(entry):
    uri, path = entry
    if not os.path.exists(path):
        r = requests.get(uri, stream=True)
        if r.status_code == 200:
            with open(path, 'wb') as f:
                for chunk in r:
                    f.write(chunk)
    return path



endpoint = 'https://spreadsheets.google.com/feeds/cells/1oyesB6iW5zYveN5C-qvwvxpMUCpwMOP7h6psa39mlsM/3/public/full?alt=json'

response = requests.get(endpoint)
data = json.loads(response.content)

links = []

for entry in data['feed']['entry']:
    if entry['gs$cell']['col'] == '1' and entry['gs$cell']['row'] != '1':
        link = entry['content']['$t']
        path = 'fragments/' + re.match(r'.*/(fragment_.*.gif)', link).group(1)
        links.append((link, path))

links = [ (link,path) for (link, path) in links if not os.path.exists(path) ]

results = ThreadPool(4).imap_unordered(fetch_fragment, links)
for path in results:
    print(path)