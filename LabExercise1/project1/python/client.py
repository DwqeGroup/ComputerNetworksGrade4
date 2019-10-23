from socket import *

HOST = '127.0.0.1'
PORT = 8080
BUFSIZ =1024
ADDR = (HOST,PORT)

tcpCliSock = socket(AF_INET,SOCK_STREAM)
tcpCliSock.connect(ADDR)
while True:
     data1 = input('>')
     if not data1:
         break
     tcpCliSock.send(data1.encode())
     data1 = tcpCliSock.recv(BUFSIZ)
     if not data1:
         break
     print(data1.decode('utf-8'))
tcpCliSock.close()