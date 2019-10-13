from socket import *
import requests
from time import ctime

HOST = ''
PORT = 21567
BUFSIZ = 1024
ADDR = (HOST,PORT)

tcpSerSock = socket(AF_INET,SOCK_STREAM)
tcpSerSock.bind(ADDR)
tcpSerSock.listen(5)

def getHTML(info):
  r = requests.get(info)
  return r.text


print('waiting for connection...')
tcpCliSock, addr = tcpSerSock.accept()
print('...connnecting from:', addr)


data = tcpCliSock.recv(BUFSIZ)

        #tcpCliSock.send('[%s] %s' %(bytes(ctime(),'utf-8'),data))
info = str(data,encoding = "utf-8")
text = getHTML(info)
tcpCliSock.send(text.encode("utf-8"))
tcpCliSock.close()



