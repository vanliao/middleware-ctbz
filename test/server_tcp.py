import socket

# 指定协议
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# 让端口可以重复使用
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
# 绑定ip和端口
server.bind(('0.0.0.0', 3335))
# 监听
server.listen(10)
# 等待消息
clientsocket, address = server.accept()
clientsocket.send(b"{\"code\":1,\"data\":\"hijklmn\"}")
# 接收消息
data = clientsocket.recv(1024).decode()
print(data)
# 关闭socket
clientsocket.close()
server.close()