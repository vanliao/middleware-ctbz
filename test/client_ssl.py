# import socket
# import ssl
#
# # 创建普通 Socket
# sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#
# # 创建 SSL 连接
# ssl_sock = ssl.wrap_socket(sock, ssl_version=ssl.PROTOCOL_TLS)
#
# # 禁用证书验证
# # ssl_sock.verify_mode = ssl.CERT_NONE
#
# # 连接服务器
# server_address = ('192.168.216.135', 3336)
# ssl_sock.connect(server_address)
#
# # 发送和接收数据
# msg = b"{\"code\":1,\"data\":\"abcdefg\"}"
# ssl_sock.send(msg)
# data = ssl_sock.recv(1024)
# print('Received:', data.decode())

# 关闭连接
# ssl_sock.close()
#
import socket
import ssl
import time

class client_ssl:
    def send_hello(self):
        CA_FILE = "cacert.pem"
        CLIENT_KEY_FILE = "client.key"
        CLIENT_CERT_FILE = "client.crt"

        # 创建SSL上下文对象
        context = ssl.SSLContext(ssl.PROTOCOL_TLS)
        context.check_hostname = False
        #####
        context.load_cert_chain(certfile=CLIENT_CERT_FILE, keyfile=CLIENT_KEY_FILE)  # 服务器不需要认证客户端证书，故不需要
        context.load_verify_locations(CA_FILE)   # 使用根证书认证服务端证书
        context.verify_mode = ssl.CERT_REQUIRED

        # 与服务端建立socket连接
        with socket.socket() as sock:
            # 将socket打包成SSL socket
            with context.wrap_socket(sock, server_side=False) as ssock:
                ssock.connect(('192.168.216.135', 3336))

                while True:
                    time.sleep(1)
                    # 输入要发送的消息
                    # msg = input("Enter a message to send (or 'quit' to exit): ")
                    # if msg.lower() == 'quit':
                    #     break
                    msg = "{\"code\":1,\"data\":\"abcdefg\"}"

                    # 向服务端发送消息
                    ssock.send(msg.encode("utf-8"))

                    # 接收并打印服务端返回的消息
                    response = ssock.recv(1024)
                    print(b"recv:" + response)
                    break

if __name__ == "__main__":
    while True:
        client = client_ssl()
        client.send_hello()