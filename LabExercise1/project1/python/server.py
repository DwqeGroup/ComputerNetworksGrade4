from socket import *
from time import ctime

HOST = '127.0.0.1'
PORT = 8080
BUFSIZ = 1024
ADDR = (HOST,PORT)

tcpSerSock = socket(AF_INET,SOCK_STREAM)
tcpSerSock.bind(ADDR)
tcpSerSock.listen(5)

def getdat(filename):
    with open(filename,"r" ) as f:
        tmplist=f.readlines()
    return tmplist

H=getdat("Humidity.dat")
L=getdat("Light.dat")
T=getdat("Temperature.dat")

while True:
    print('waiting for connection...')
    tcpCliSock, addr = tcpSerSock.accept()
    print('...connnecting from:', addr)
    while True:
        data = tcpCliSock.recv(BUFSIZ)
        if not data:
            break
        print("客户端请求："+data.decode('utf-8'))
        data1=data.decode('utf-8')
        num=int(data1)
        message=" TEMPERATURE = "+T[num-1]+" HUMIDITY = "+H[num-1]+" LIGHT = "+L[num-1]
        tcpCliSock.send(message.encode())
    tcpCliSock.close()
tcpSerSock.close()