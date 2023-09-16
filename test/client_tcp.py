import socket
import json

address = ("127.0.0.1", 3333)
sock = socket.socket()
sock.connect(address)
print("连接成功")

sock.send(b"{\"code\":1,\"data\":\"abcdefg\"}")
recvData = sock.recv(8192).decode()
print(recvData)

sock.send(b"{\"code\":1,\"data\":\"abcdefg\"}")
recvData = sock.recv(8192).decode()
print(recvData)

sock.send(b"{\"code\":1,\"data\":\"abcdefg\"}")
recvData = sock.recv(8192).decode()
print(recvData)

sock.send(b"{\"code\":1,\"data\":\"abcdefg\"}")
recvData = sock.recv(8192).decode()
print(recvData)

sock.send(b"{\"code\":1,\"data\":\"abcdefg\"}")
recvData = sock.recv(8192).decode()
print(recvData)

sock.send(b"{\"code\":1,\"data\":\"abcdefg\"}")
recvData = sock.recv(8192).decode()
print(recvData)


recvData = sock.recv(8192).decode()
print(recvData)