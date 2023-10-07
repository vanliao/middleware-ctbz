#include "tinylog.h"
#include "websocket_client.h"

namespace network {

WebsocketClient::WebsocketClient(const std::string serverIP, const int serverPort) :
    TcpClient(serverIP, serverPort), isWSUpdated(false),
    closeStatus(WebSocket::CLOSE_NONE), recvBuf(""), wsOpCode(0)
{
    return;
}

WebsocketClient::WebsocketClient(const int clientFd) :
    TcpClient(clientFd), isWSUpdated(false),
    closeStatus(WebSocket::CLOSE_NONE), recvBuf(""), wsOpCode(0)
{
    return;
}

WebsocketClient::~WebsocketClient()
{
    return;
}

bool WebsocketClient::recv(std::string &buf)
{
    std::string tcpBuf;
    bool ret = TcpClient::recv(tcpBuf);
    if (!ret)
    {
        return ret;//false
    }

    if (tcpBuf.empty())
    {
        closeStatus = WebSocket::CLOSE_FORCE;
        return ret;//true
    }

    if (isWSUpdated)
    {
        ret = handleWSMsg(tcpBuf);
        if (ret)
        {
            buf = recvBuf;
            recvBuf = "";
        }
    }
    else
    {
        handleHttpRequest(tcpBuf);
        ret = false;
    }

    return ret;
}

bool WebsocketClient::sendPrepare(const std::string &buf, const uint8_t opcode, const bool fin)
{
    uint8_t op = opcode;
    std::string::size_type totalLen = buf.length();
    if (WebSocket::maxSendPayload < totalLen)
    {
        /* 发送分片 */
        std::string::size_type offset = 0;
        std::string::size_type leftLen = totalLen;
        while (0 < leftLen)
        {
            std::string::size_type sendLen = leftLen < WebSocket::maxSendPayload?leftLen:WebSocket::maxSendPayload;
            std::string header = creatWSHeader(sendLen, op, ((leftLen - sendLen) == 0));
            std::string wsMsg = header;
            wsMsg.append(buf.c_str() + offset, sendLen);
            sendBuf.push_back(wsMsg);

            leftLen -= sendLen;
            offset += sendLen;
            op = WebSocket::CONT;
        }
    }
    else
    {
        std::string header = creatWSHeader(buf.length(), opcode, fin);
        std::string wsMsg = header;
        wsMsg.append(buf.c_str(), buf.length());
        sendBuf.push_back(wsMsg);
    }

    return true;
}

void WebsocketClient::closeSend()
{
    switch (closeStatus)
    {
        case WebSocket::CLOSE_NONE:
        {
            closeStatus = WebSocket::CLOSE_SEND;
            break;
        }
        case WebSocket::CLOSE_RECV:
        {
            /* 告知后续流程关闭连接 */
            closeStatus = WebSocket::CLOSE_COMPLETE;
            break;
        }
        case WebSocket::CLOSE_SEND:
        {
            log_error("send ws close msg again");
            break;
        }
        case WebSocket::CLOSE_FORCE:
        {
            log_error("send ws close msg when close force");
            break;
        }
        case WebSocket::CLOSE_COMPLETE:
        {
            log_error("send ws close msg when close complete");
            break;
        }
        default:
        {
            log_error("invalid close status:" << closeStatus);
            break;
        }
    }

    return;
}

std::string WebsocketClient::creatWSHeader(const uint64_t payloadLen, const uint8_t opcode, const bool fin)
{
    uint8_t header[14] = {0};
    uint32_t headerLen = 2;
    header[0] = (opcode & 15) | ((uint8_t)fin << 7);
//    header[1] = (uint8_t)SendMask << 7;
    if (payloadLen < 126)
    {
        header[1] |= (uint8_t)payloadLen;
    }
    else if (payloadLen < 65536)
    {
        header[1] |= 126;
        *(uint16_t*)(header + 2) = htobe16((unsigned short)payloadLen);
        headerLen += 2;
    }
    else
    {
        header[1] |= 127;
        *(uint64_t*)(header + 2) = htobe64(payloadLen);
        headerLen += 8;
    }

//    if (SendMask)
//    {
//        // for efficency and simplicity masking-key is always set to 0
//        *(uint32_t*)(header + headerLen) = 0;
//        headerLen += 4;
//    }

//    log_debug("head len:" << headerLen << " " << (uint)header[0] << " " << (uint)header[1] << " " << wsMsg.length());

    std::string wsMsgHeader = "";
    wsMsgHeader.append((char *)header, headerLen);
    return wsMsgHeader;
}

uint32_t WebsocketClient::rol(uint32_t value, uint32_t bits)
{
    return (value << bits) | (value >> (32 - bits));
}

uint32_t WebsocketClient::sha1base64(uint8_t* in, uint64_t in_len, char* out)
{
    uint32_t h0[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    uint64_t total_len = in_len;
    in[total_len++] = 0x80;
    int padding_size = (64 - (total_len + 8) % 64) % 64;
    while (padding_size--) in[total_len++] = 0;
    for (uint64_t i = 0; i < total_len; i += 4)
    {
        uint32_t& w = *(uint32_t*)(in + i);
        w = be32toh(w);
    }
    *(uint32_t*)(in + total_len) = (uint32_t)(in_len >> 29);
    *(uint32_t*)(in + total_len + 4) = (uint32_t)(in_len << 3);
    for (uint8_t* in_end = in + total_len + 8; in < in_end; in += 64)
    {
        uint32_t* w = (uint32_t*)in;
        uint32_t h[5];
        memcpy(h, h0, sizeof(h));
        for (uint32_t i = 0, j = 0; i < 80; i++, j += 4)
        {
            uint32_t &a = h[j % 5], &b = h[(j + 1) % 5], &c = h[(j + 2) % 5], &d = h[(j + 3) % 5], &e = h[(j + 4) % 5];
            if (i >= 16) w[i & 15] = rol(w[(i + 13) & 15] ^ w[(i + 8) & 15] ^ w[(i + 2) & 15] ^ w[i & 15], 1);
            if (i < 40) {
            if (i < 20)
            e += ((b & (c ^ d)) ^ d) + 0x5A827999;
            else
            e += (b ^ c ^ d) + 0x6ED9EBA1;
            }
            else {
            if (i < 60)
            e += (((b | c) & d) | (b & c)) + 0x8F1BBCDC;
            else
            e += (b ^ c ^ d) + 0xCA62C1D6;
            }
            e += w[i & 15] + rol(a, 5);
            b = rol(b, 30);
        }
        for (int i = 0; i < 5; i++) h0[i] += h[i];
    }
    const char* base64tb = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    uint32_t triples[7] = {
        h0[0] >> 8,
        (h0[0] << 16) | (h0[1] >> 16),
        (h0[1] << 8) | (h0[2] >> 24),
        h0[2],
        h0[3] >> 8,
        (h0[3] << 16) | (h0[4] >> 16),
        h0[4] << 8
    };
    for (uint32_t i = 0; i < 7; i++)
    {
        out[i * 4] = base64tb[(triples[i] >> 18) & 63];
        out[i * 4 + 1] = base64tb[(triples[i] >> 12) & 63];
        out[i * 4 + 2] = base64tb[(triples[i] >> 6) & 63];
        out[i * 4 + 3] = base64tb[triples[i] & 63];
    }
    out[27] = '=';
    return 28;
}

uint32_t WebsocketClient::handleHttpRequest(std::string &wsMsg)
{
    uint32_t size = wsMsg.length();
    const char *data = wsMsg.data();
    const char* data_end = data + size;
    const int ValueBufSize = 128;
    char request_uri[1024] = {0};
    char host[ValueBufSize] = {0};
    char origin[ValueBufSize] = {0};
    char wskey[ValueBufSize] = {0};
    char wsprotocol[ValueBufSize] = {0};
    char wsextensions[ValueBufSize] = {0};
    bool upgrade_checked = false, connection_checked = false, wsversion_checked = false;

    while (true)
    {
        const char* ln = (char*)memchr(data, '\n', data_end - data);
        if (!ln) return size;
        if (*--ln != '\r') break;
        if (request_uri[0] == 0)
        {
            // first line
            if (memcmp(data, "GET ", 4)) break;
            data += 4;
            while (*data == ' ') data++;
            const char* uri_end = (char*)memchr(data, ' ', ln - data);
            uint32_t uri_len = uri_end - data;
            if (!uri_end || uri_len >= sizeof(request_uri)) break;
            memcpy(request_uri, data, uri_len);
            request_uri[uri_len] = 0;
        }
        else
        {
            const char* val_end = ln;
            while (val_end[-1] == ' ') val_end--;
            if (val_end == data)
            {
                // end of headers
                if (!host[0] || !wskey[0] || !upgrade_checked || !connection_checked || !wsversion_checked)
                {
                    break;
                }
                std::string resp = "";
                memcpy(wskey + 24, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);
                char accept_str[32];
                accept_str[sha1base64((uint8_t*)wskey, 24 + 36, accept_str)] = 0;
                resp += "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: ";
                resp += "Upgrade\r\nSec-WebSocket-Accept: " + std::string(accept_str) + "\r\n";
                resp += "\r\n";
                TcpClient::send(resp);
                isWSUpdated = true;
                log_debug("http update ws success");
                return true;
            }
            const char* colon = (char*)memchr(data, ':', ln - data);
            if (!colon) break;
            const char* val = colon + 1;
            while (*val == ' ') val++;
            uint32_t key_len = colon - data;
            uint32_t val_len = val_end - val;
            if (val_len < ValueBufSize)
            {
                if (key_len == 4 && !memcmp(data, "Host", 4))
                {
                    memcpy(host, val, val_len);
                    host[val_len] = 0;
                }
                else if (key_len == 6 && !memcmp(data, "Origin", 6))
                {
                    memcpy(origin, val, val_len);
                    origin[val_len] = 0;
                    log_debug("ws origin " << origin);
                }
                else if (key_len == 7 && !memcmp(data, "Upgrade", 7))
                {
                    if (memcmp(val, "websocket", 9)) break;
                    upgrade_checked = true;
                }
                else if (key_len == 10 && !memcmp(data, "Connection", 10))
                {
                    if (!memcmp(val, "Upgrade", 7))
                    {
                        connection_checked = true;
                    }
                }
                else if (key_len == 17 && !memcmp(data, "Sec-WebSocket-Key", 17))
                {
                    if (val_len != 24) break;
                    memcpy(wskey, val, val_len);
                    log_debug("ws Sec-WebSocket-Key " << wskey);
                }
                else if (key_len == 21 && !memcmp(data, "Sec-WebSocket-Version", 21))
                {
                    if (val_len != 2 || memcmp(val, "13", 2)) break;
                    wsversion_checked = true;
                }
                else if (key_len == 22 && !memcmp(data, "Sec-WebSocket-Protocol", 22))
                {
                    memcpy(wsprotocol, val, val_len);
                    wsprotocol[val_len] = 0;
                    log_debug("ws Sec-WebSocket-Protocol " << wsprotocol);
                }
                else if (key_len == 24 && !memcmp(data, "Sec-WebSocket-Extensions", 24))
                {
                    memcpy(wsextensions, val, val_len);
                    wsextensions[val_len] = 0;
                    log_debug("ws Sec-WebSocket-Extensions " << wsextensions);
                }
            }
        }
        data = ln + 2; // skip \r\n
    }

    std::string resp400 = "HTTP/1.1 400 Bad Request\r\nSec-WebSocket-Version: 13\r\n\r\n";
    TcpClient::send(resp400);
    closeStatus = WebSocket::CLOSE_COMPLETE;
    log_error("ws connect failed");

    return false;
}

bool WebsocketClient::handleWSMsg(std::string &wsMsg)
{
    // we might read a little more bytes beyond size, which is okey
    const char *data = wsMsg.data();

    /*
     * 未分片消息:FIN取值1，opcode取值非0
     * 分片消息:
     *  1.第一个分片:FIN取值0，opcode取值非0;
     *  2.中间0个或多个分片:FIN取值为，opcode取值0
     *  3.最后一个分片:FIN取值1，opcode取值0
     */
    uint8_t opcode = data[0] & 15;
    if (WebSocket::CONT != opcode)
    {
        wsOpCode = opcode;/* 记录第一个分片类型 */
    }
    bool fin = data[0] >> 7; //, control = opcode >> 3;
    bool mask = data[1] >> 7;

    uint64_t payloadLen = data[1] & 127;
    u_int64_t offset = 0;
    offset += 2;
    if (payloadLen == 126)
    {
        payloadLen = be16toh(*(uint16_t*)(data + offset));
        offset += 2;
    }
    else if (payloadLen == 127)
    {
        payloadLen = be64toh(*(uint64_t*)(data + offset)) & ~(1ULL << 63);
        offset += 8;
    }

    uint8_t mask_key[4] = {0};
    if (mask)
    {
        *(uint32_t*)mask_key = *(uint32_t*)(data + offset);
        offset += 4;
    }
    //  if (data_end - data < (int64_t)pl_len) {
    //    if (size + (data + pl_len - data_end) > RecvBufSize) close(1009);
    //    return size;
    //  }

    if (mask)
    {
        for (uint64_t i = 0; i < payloadLen; i++)
        {
            wsMsg[offset + i] ^= mask_key[i & 3];
        }
    }

//    log_debug("fin:" << fin << " mask:" << mask << " mask_key:" << *(u_int32_t *)mask_key << " payloadLen:" << payloadLen);

    if (wsOpCode == network::WebSocket::CLOSE)
    {
        uint16_t statusCode = 1005;
        std::string reason = "";
        if (payloadLen >= 2)
        {
            statusCode = be16toh(*(uint16_t*)(data + offset));
            uint64_t reasonLen = (sizeof(reason) - 1 < payloadLen - 2)?(sizeof(reason) - 1):(payloadLen - 2);
            reason.append((data + offset) + 2, reasonLen);
        }
        log_debug("ws recv close code:" << statusCode << " reason:" << reason);

        recvBuf = "";
        switch (closeStatus)
        {
            case WebSocket::CLOSE_NONE:
            {
                closeStatus = WebSocket::CLOSE_RECV;
                recvBuf.append((data + offset), payloadLen);
                break;
            }
            case WebSocket::CLOSE_RECV:
            {
                log_error("recv ws close msg again");
                break;
            }
            case WebSocket::CLOSE_SEND:
            {
                /* 告知后续流程关闭连接 */
                closeStatus = WebSocket::CLOSE_COMPLETE;
                break;
            }
            case WebSocket::CLOSE_FORCE:
            {
                log_error("recv ws close msg when close force");
                break;
            }
            case WebSocket::CLOSE_COMPLETE:
            {
                log_error("recv ws close msg when close complete");
                break;
            }
            default:
            {
                log_error("invalid close status:" << closeStatus);
                break;
            }
        }
    }
    else
    {
        recvBuf.append((data + offset), payloadLen);
    }

    return fin;
}

}
