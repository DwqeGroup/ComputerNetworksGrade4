import requests
url = 'https://qd.myapp.com/myapp/qqteam/pcqq/PCQQ2019.exe'
my_file = requests.get(url, allow_redirects=True)
open('.\\qq.exe', 'wb').write(my_file.content)
print("Download successfully")
