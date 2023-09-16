import socket
import json

address = ("127.0.0.1", 3333)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(b"{\"code\":1,\"data\":\"abcdefg\"}", address)
recvData = sock.recv(8192).decode()
print(recvData)


sock.sendto(b"{\"code\":1,\"data\":\"abcdefg\"}", address)
recvData = sock.recv(8192).decode()
print(recvData)

sock.sendto(b"{\"code\":1,\"data\":\"abcdefg\"}", address)
recvData = sock.recv(8192).decode()
print(recvData)

sock.sendto(b"{\"code\":1,\"data\":\"abcdefg\"}", address)
recvData = sock.recv(8192).decode()
print(recvData)

sock.close()