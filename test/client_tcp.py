import socket
import json

def run(cnt):
    address = ("192.168.216.135", 3333)
    sock = socket.socket()
    sock.connect(address)
    print("连接成功")

    for i in range(cnt):
        sock.send(b"{\"code\":1,\"data\":\"abcdefg\"}")
        recvData = sock.recv(8192).decode()
        print(recvData)

while True:
    run(10)