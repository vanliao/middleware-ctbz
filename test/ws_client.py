import asyncio
import websockets
import time

###测试服务器发送close
# async def ws_client(url):
#     for i in range(1, 2):
#         async with websockets.connect(url) as websocket:
#             await websocket.send("login 123456")
#             response = await websocket.recv()
#             print(response)
#         time.sleep(1)

###数据交互测试
async def ws_client(url):
    for i in range(1, 40):
        async with websockets.connect(url) as websocket:
            await websocket.send("{\"msgType\":\"login\", \"password\":\"123456\"}")
            response = await websocket.recv()
            print(response)
            await websocket.send("{\"msgType\":\"echo\", \"data\":\"hello\"}")
            response = await websocket.recv()
            print(response)
            # await websocket.ping("1234567")
        # time.sleep(1)

asyncio.run(ws_client('ws://192.168.216.135:4444/login'))


