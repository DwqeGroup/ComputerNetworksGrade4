import requests

url = "http://cn.bing.com/"
proxy = {
        "http": "https://localhost:8080",
        "https": "https://localhost:8080"}
headers = {
    'User-Agent':'Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/63.0.3239.132 Safari/537.36 QIHU 360SE'
}

r = requests.get(url, headers = headers, proxies = proxy)

print(r.text)