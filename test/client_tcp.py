import socket
import json

address = ("192.168.216.135", 3333)
sock = socket.socket()
sock.connect(address)
print("连接成功")

# sock.send(b"{\"code\":1,\"data\":\"abcdefg\"}")
# recvData = sock.recv(8192).decode()
# print(recvData)
#
# sock.send(b"{\"code\":1,\"data\":\"abcdefg\"}")
# recvData = sock.recv(8192).decode()
# print(recvData)
#
# sock.send(b"{\"code\":1,\"data\":\"abcdefg\"}")
# recvData = sock.recv(8192).decode()
# print(recvData)
#
# sock.send(b"{\"code\":1,\"data\":\"abcdefg\"}")
# recvData = sock.recv(8192).decode()
# print(recvData)
#
# sock.send(b"{\"code\":1,\"data\":\"abcdefg\"}")
# recvData = sock.recv(8192).decode()
# print(recvData)

while True:
    sock.send(b"{\"code\":1,\"data\":\"abcdefg\"}")
    recvData = sock.recv(8192).decode()
    print(recvData)


# recvData = sock.recv(8192).decode()
print(recvData)