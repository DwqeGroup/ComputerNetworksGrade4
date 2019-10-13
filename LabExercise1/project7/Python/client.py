from socket import *

HOST = '127.0.0.1' # or 'localhost'
PORT = 21567
BUFSIZ =102400
ADDR = (HOST,PORT)
URL = "https://www.baidu.com"
tcpCliSock = socket(AF_INET,SOCK_STREAM)
tcpCliSock.connect(ADDR)


tcpCliSock.send(URL.encode())
data1 = tcpCliSock.recv(BUFSIZ)
info = str(data1,encoding = "utf-8")
print(info)
tcpCliSock.close()
