import asyncio
import websockets

IP_ADDR = "0.0.0.0"
IP_PORT = "5555"


async def server_hands(websocket):
    """ 握手
    """
    while True:
        try:
            await websocket.send("{\"msgType\":\"login\", \"password\":\"123456\"}")
            response_str = await websocket.recv()
            print(response_str)
            return True
        except websockets.WebSocketClientProtocol as e:
            print(e.code)
            await asyncio.sleep(0.01)
            break

async def server_recv(websocket):
    """ 接收从客户端发来的消息并处理，再返给客户端ok。
    """
    while True:
        # 发送数据并打印服务端返回结果
        await websocket.send("{\"msgType\":\"echo\", \"data\":\"hello\"}")
        recv_text = await websocket.recv()
        print(recv_text)
        await asyncio.sleep(1)


async def server_run(websocket, path):
    """ 握手并且接收数据
    :param websocket:
    :param path:
    """
    # 下面两个函数顺序执行
    await server_hands(websocket)  # 握手

    await server_recv(websocket)  # 接收客户端消息并处理


# main function
if __name__ == '__main__':
    print("======server main begin======")
    server = websockets.serve(server_run, IP_ADDR, IP_PORT)  # 服务器端起server
    asyncio.get_event_loop().run_until_complete(server)  # 事件循环中调用
    asyncio.get_event_loop().run_forever()  # 一直运行