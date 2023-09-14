import socket

# 指定协议
server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# 让端口可以重复使用
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
# 绑定ip和端口
server.bind(('0.0.0.0', 3335))
# 接收消息
data, client_address = server.recvfrom(1024)
print(data.decode())

server.sendto(b"{\"code\":1,\"data\":\"hijklmn\"}", client_address)

# 接收消息
data, client_address = server.recvfrom(1024)
print(data.decode())

# 关闭socket
server.close()