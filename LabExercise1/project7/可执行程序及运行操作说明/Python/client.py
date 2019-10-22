import requests
proxies = {
  "http": "http://127.0.0.1:3128",
  "https": "http://127.0.0.1:2080",
}
r=requests.get("http://baidu.com", proxies=proxies)
print(r.text)