import socket
import ssl
import threading
import time

class server_ssl:
    def build_listen(self):
        # CA_FILE = "ca-cert.pem"
        KEY_FILE = "server.key"
        CERT_FILE = "server.crt"
        context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
        context.load_cert_chain(certfile=CERT_FILE, keyfile=KEY_FILE)  # 加载服务端证书和私钥
        # context.load_verify_locations(CA_FILE)  # 加载根证书
        context.verify_mode = ssl.CERT_NONE  # 不需要客户端提供证书

        # 监听端口
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0) as sock:
            # 将socket打包成SSL socket
            with context.wrap_socket(sock, server_side=True) as ssock:
                ssock.bind(('0.0.0.0', 3337))
                ssock.listen(5)
                print("Server is listening for connections...")

                while True:
                    # 接收客户端连接
                    client_socket, addr = ssock.accept()
                    print(f"Accepted connection from {addr}")

                    # 创建新线程来处理客户端请求
                    client_thread = threading.Thread(target=self.handle_client, args=(client_socket, addr))
                    client_thread.start()

    def handle_client(self, client_socket, addr):
        try:
            while True:
                time.sleep(1)
                # 接收客户端信息
                msg = client_socket.recv(1024).decode("utf-8")
                if not msg:
                    break  # 客户端断开连接

                print(f"Received message from client {addr}: {msg}")

                # 向客户端发送信息
                response = b"{\"code\":1,\"data\":\"hijklmn\"}"
                client_socket.send(response)
        except Exception as e:
            print(f"Error: {str(e)}")
        finally:
            client_socket.close()
            print("Connection closed")

if __name__ == "__main__":
    server = server_ssl()
    server.build_listen()