import requests

def stress():
    for i in range(1000000):
        r = requests.post('http://127.0.0.1:8083', f"11{i}")
        print(r.content)

stress()