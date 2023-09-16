#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <sys/statfs.h>
#include <chrono>
#include <fcntl.h>
#include <atomic>
#include "api.h"
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/md5.h>

namespace api {

using namespace std;

bool is_digit(string str)
{
    for (char& ch : str) {
        if (ch < '0' || ch > '9') {
            return false;
        }
    }

    return true; 
}

int str2int(string str)
{
    int i;
    stringstream ss;
    ss << str;
    ss >> i;
    return i;
}

double str2double(string str)
{
    double i;
    stringstream ss;
    ss << str;
    ss >> i;
    return i;
}

#if 0
int str2int(string& str)
{
    int i;
    stringstream ss;
    ss << str;
    ss >> i;
    return i;
}
#endif

int str2int(const char* str)
{
    int i;
    stringstream ss;
    ss << str;
    ss >> i;
    return i;
}
string int2str(int num)
{
    string str;
    stringstream ss;
    ss << num;
    ss >> str;
    return str;
}

string int2str(unsigned int num)
{
    string str;
    stringstream ss;
    ss << num;
    ss >> str;
    return str;
}

string u8_to_ip(unsigned char* u8Ip)
{
    stringstream ss;
    for (int i = 0; i < 4; i++) {
        ss << (int)u8Ip[i];
        if (i < 3) ss << ".";
    }

    return ss.str();
}

char* get_short_name(char* path)
{
    string str = path;
    size_t pos = str.rfind("/");
    if (pos != string::npos) {
        return (char *)(path + pos + 1);
    } else {
        return path;
    }
}

string get_gmt_time()
{
    time_t rawtime;
    time(&rawtime);

    struct tm *info = gmtime(&rawtime);

    string gm = asctime(info);
    gm.erase(gm.find_last_not_of("\n") + 1);

    gm += " GMT";

    return gm;
}

string get_local_time()
{
    time_t rawtime;
    time(&rawtime);

    struct tm *info = localtime(&rawtime);

    string lt = asctime(info);

    lt.erase(lt.find_last_not_of("\n") + 1);

    return lt;
}

string get_fmt_time()
{
    time_t rawtime;
    time(&rawtime);
    struct tm *info = localtime(&rawtime);

    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", info);
    //printf("格式化的日期 & 时间 : |%s|\n", buffer );
    return buffer;
}

string get_fmt_time(unsigned int sec)
{
    time_t rawtime = (time_t)sec;
    struct tm *info = localtime(&rawtime);

    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", info);
    return buffer;
}

string to_hms(unsigned int sec)
{
    unsigned int h = sec / 3600;
    unsigned int m = (sec % 3600) / 60;
    unsigned int s = (sec % 3600) % 60;
    stringstream ss;
    ss << setfill('0') << setw(2) << h << ":" << setw(2) << m << ":" << setw(2) << s;
    return ss.str();
}

int to_lower(int c)
{
    if ((c >= 'A') && (c <= 'Z'))
        return c + ('a' - 'A');
    return c;
}

int to_upper(int c)
{
    if ((c >= 'a') && (c <= 'z'))
        return c + ('A' - 'a');
    return c;
}

bool is_any_of(char c, char* s)
{
    int l = strlen(s);
    if (!l) return false;

    for (int i = 0; i < l; i++) {
        if (c == s[i]) return true;
    }

    return false;
}

bool split(const char* str, const char* delim, vector<string>& vec)
{
    int len = strlen((char *)str);
    int size = strlen((char *)delim);

    if (!len || !size) return false;
    if (!vec.empty()) vec.clear();

    int   from = -1;

    char* p = (char *)str;
    for (int i = 0; i < len; i++) {
        if (is_any_of(*(p + i), (char *)delim)) {
            if (from != -1 && i > from) {
                vec.push_back(string(p + from, i - from));

                from = -1;
            }

            continue;
        } else {
            if (from == -1) 	from = i;
        }
    }

    if (from != -1 && from < len) {
        vec.push_back(string(p + from, len - from));
    }

    return true;
}

void split(const string& str, const char* delim, vector<string>& vec)
{
    string::size_type from = str.find_first_not_of(delim, 0);

    string::size_type to   = str.find_first_of(delim, from);

    while (string::npos != to || string::npos != from) {
        vec.push_back(str.substr(from, to - from));

        from = str.find_first_not_of(delim, to);
        to   = str.find_first_of(delim, from);
    }
}

void trim(string& str, const char* delim)
{
    str.erase(str.find_last_not_of(delim) + 1);

    str.erase(0, str.find_first_not_of(delim));
}

bool file_existed(const char* fn)
{
    if (fn && access(fn, F_OK) == 0) {
    #if 0
        if (access(fn, R_OK | W_OK) == 0) { // file readable and writalbe
            cout << "rw" << endl;
        }

        if (access(fn, X_OK) == 0) { // file executable
            cout << "x" << endl;
        }
    #endif
        return true;
    }

    return false;
}

bool is_ip_v4_addr(string ip)
{
    if (ip.find_first_of(":") != string::npos) {
        return false;
    }

    return true;
}

struct cpu_usage
{
    char name[20];
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
    unsigned int lowait;
    unsigned int irq;
    unsigned int softirq;
};

static void get_cpuoccupy(cpu_usage &cpust)
{
    FILE *fd;
    char buff[256];

    fd = fopen("/proc/stat", "r");
    fgets(buff, sizeof(buff), fd);

    sscanf(buff, "%s %u %u %u %u %u %u %u",
           cpust.name, &cpust.user, &cpust.nice, &cpust.system, &cpust.idle,
           &cpust.lowait, &cpust.irq, &cpust.softirq);

    fclose(fd);

    return;
}

bool calc_cpuoccupy(int &total, int &free)
{
    total = 0;
    free = 0;

    cpu_usage o;
    cpu_usage n;
    unsigned long od, nd;

    get_cpuoccupy(o);
    usleep(100000);
    get_cpuoccupy(n);

    od = (unsigned long)(o.user + o.nice + o.system + o.idle + o.lowait + o.irq + o.softirq);
    nd = (unsigned long)(n.user + n.nice + n.system + n.idle + n.lowait + n.irq + n.softirq);
    double sum = nd - od;
    double idle = n.idle - o.idle;

    if (0 == sum) {
        return false;
    }

    total = (int)sum;
    free = (int)idle;

    return true;
}

struct mem_usage
{
    char name1[20];
    unsigned long mem_total;
    char name2[20];
    unsigned long mem_free;
    char name3[20];
    unsigned long buffers;
    char name4[20];
    unsigned long cached;
    char name5[20];
    unsigned long swap_cached;
};

bool calc_memoccupy(int &total, int &free)
{
    total = 0;
    free = 0;

    FILE *fd;
    char buff[256];
    mem_usage mem;

    fd = fopen("/proc/meminfo", "r");
    //MemTotal: 515164 kB
    //MemFree: 7348 kB
    //Buffers: 7892 kB
    //Cached: 241852  kB
    //SwapCached: 0 kB
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %lu ", mem.name1, &mem.mem_total);
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %lu ", mem.name2, &mem.mem_free);
//    fgets(buff, sizeof(buff), fd);
//    sscanf(buff, "%s %lu ", mem.name3, &mem.Buffers);
//    fgets(buff, sizeof(buff), fd);
//    sscanf(buff, "%s %lu ", mem.name4, &mem.Cached);
//    fgets(buff, sizeof(buff), fd);
//    sscanf(buff, "%s %lu", mem.name5, &mem.SwapCached);
//    printf("%s\n", buff);

    fclose(fd);

    if (0 == mem.mem_total) {
        return false;
    }

    total = (int)mem.mem_total;
    free = (int)mem.mem_free;

    return true;
}

static void get_cur_executable_path(std::string &path)
{
#ifndef MIPS
    char *p = NULL;
    const int len = 256;
    char arr_tmp[len] = {0};

    int n = readlink("/proc/self/exe", arr_tmp, len);
    if (n < 0) {
        return;
    }

    if (NULL != (p = strrchr(arr_tmp,'/'))) {
        *p = '\0';
        path.append(arr_tmp);
    } else {
        printf("wrong process path");
    }
#else
    path = "/media/sda";
#endif
    return;
}

bool calc_diskoccupy(int &total, int &free)
{
    total = 0;
    free = 0;

    std::string exec_str;
    get_cur_executable_path(exec_str);

    if (exec_str.empty()) {
        return false;
    }

    struct statfs diskInfo;
    statfs(exec_str.c_str(), &diskInfo);

    unsigned long long blocksize = diskInfo.f_bsize;
    unsigned long long totalsize = (blocksize * diskInfo.f_blocks)>>10;
    unsigned long long freeDisk = (diskInfo.f_bfree * blocksize)>>10;
//    unsigned long long availableDisk = diskInfo.f_bavail * blocksize;
    if (0 == totalsize) {
        return false;
    }

    total = (int)totalsize;
    free = (int)freeDisk;

    return true;
}

int make_socket_non_blocking(int sfd)
{
    int flags, s;

    flags = fcntl (sfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror ("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl (sfd, F_SETFL, flags);
    if (s == -1)
    {
        perror ("fcntl");
        return -1;
    }

    return 0;
}

uint64_t get_current_timestamp()
{
    return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now().time_since_epoch()).count();
}

unsigned int getClientID()
{
    static unsigned int zero = 0;
    static unsigned int init = 100001;
    static std::atomic<unsigned int> clientID(init);    //多线程访问需要原子操作

    unsigned int value1;
    unsigned int value2;

    do
    {
        value1 = clientID.load();
        clientID.fetch_add(1);
        clientID.compare_exchange_strong(zero, init);   //翻转
        value2 = clientID.load();
    }while ((value1 + 1) != value2);

    return value1;
}

static void hex_print(const void* pv, size_t len)
{
    const unsigned char * p = (const unsigned char*)pv;
    if (NULL == pv)
        printf("NULL");
    else
    {
        size_t i = 0;
        for (; i<len;++i)
            printf("%02X ", *p++);
    }
    printf("\n");
}

#define AES_KEYLENGTH 256
std::string encrypt_cbc(const std::string &plaintext, const std::string &skey, const std::string &siv)
{
    size_t inputslength = plaintext.length();

    unsigned char aes_input[inputslength];
    memset(aes_input, 0, sizeof(aes_input));
    strcpy((char*)aes_input, plaintext.c_str());

    unsigned char aes_key[AES_KEYLENGTH];
    memset(aes_key, 0, sizeof(aes_key));
    strcpy((char*)aes_key, skey.c_str());

    unsigned char iv[AES_BLOCK_SIZE];
    memset(iv, 0x00, sizeof(iv));
    strcpy((char*)iv, siv.c_str());

    const size_t encslength = ((inputslength + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    unsigned char enc_out[encslength];
    unsigned char dec_out[inputslength];
    memset(enc_out, 0, sizeof(enc_out));
    memset(dec_out, 0, sizeof(dec_out));


    AES_KEY enc_key;
    AES_set_encrypt_key(aes_key, AES_KEYLENGTH, &enc_key);
    AES_cbc_encrypt(aes_input, enc_out, inputslength, &enc_key, iv, AES_ENCRYPT);

    char enc_outBase64[8192] = {0};
    int enc_outBase64Len = 0;
    encrypt_base64(enc_out, encslength, enc_outBase64, &enc_outBase64Len);

//    AES_KEY dec_key;
//    memset(iv, 0x00, sizeof(iv));
//    strcpy((char*) iv, siv.c_str());
//    AES_set_decrypt_key(aes_key, AES_KEYLENGTH, &dec_key);
//    AES_cbc_encrypt(enc_out, dec_out, encslength, &dec_key, iv, AES_DECRYPT);

//    printf("original:\t");
//    hex_print(aes_input, sizeof(aes_input));
//    printf("encrypt:\t");
//    hex_print(enc_out, sizeof(enc_out));
//    printf("decrypt:\t");
//    hex_print(dec_out, sizeof(dec_out));

//    if (0 == memcmp(dec_out, aes_input, inputslength))
//    {
//        printf("OK\n");
//    }
//    else
//    {
//        printf("ERROR\n");
//    }

    return enc_outBase64;
}

std::string decrypt_cbc(const std::string &encrypttext, const std::string &skey, const std::string &siv)
{
    unsigned char aes_txt[8192] = {0};
    int aes_txt_len = 0;
    api::decrypt_base64(encrypttext.c_str(), encrypttext.length(), aes_txt, &aes_txt_len);

    unsigned char aes_key[AES_KEYLENGTH];
    memset(aes_key, 0, sizeof(aes_key));
    strcpy((char*)aes_key, skey.c_str());

    unsigned char iv[AES_BLOCK_SIZE];
    memset(iv, 0x00, sizeof(iv));
    strcpy((char*)iv, siv.c_str());

    unsigned char aes_out[aes_txt_len];
    memset(aes_out, 0, sizeof(aes_out));

    AES_KEY dec_key;
    AES_set_decrypt_key(aes_key, AES_KEYLENGTH, &dec_key);
    AES_cbc_encrypt(aes_txt, aes_out, aes_txt_len, &dec_key, iv, AES_DECRYPT);

    size_t encslength = strlen((char *)aes_out);
    stringstream ss;
    for(size_t i = 0; i < encslength; i++)
    {
        ss << aes_out[i];
    }
    return ss.str();
}

bool encrypt_base64(const unsigned char* in, int inlen, char* out, int* outlen, bool newline)
{
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new(BIO_s_mem());
    if (!b64 || !bmem)
    {
        return false;
    }

    b64 = BIO_push(b64, bmem);
    if (!newline)
    {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // ignore newlines, write everything in one line
    }

    *outlen = BIO_write(b64, in, inlen);
    if (*outlen <= 0 || *outlen != inlen)
    {
        return false;
    }

    BIO_flush(b64);
    BUF_MEM* buf = nullptr;
    BIO_get_mem_ptr(b64, &buf);
    *outlen = buf->length;
    memcpy(out, buf->data, *outlen);
    BIO_free_all(b64);
    return true;
}

bool decrypt_base64(const char* in, int inlen, unsigned char* out, int* outlen, bool newline)
{
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new_mem_buf(in, inlen);
    if (!b64 || !bmem)
    {
        return false;
    }

    b64 = BIO_push(b64, bmem);
    if (!newline)
    {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // ignore newlines, write everything in one line
    }

    *outlen = BIO_read(b64, out, inlen);
    if (*outlen <= 0)
    {
        return false;
    }

    BIO_free_all(b64);
    return true;
}

std::string getMd5Str(const char* data, int length)
{
    MD5_CTX c;
    MD5_Init(&c);
    MD5_Update(&c, data, length);

    unsigned char digest[16] = { 0 };
    char hexBuffer[3];
    MD5_Final(digest, &c);

    std::string result;
    for (size_t i = 0; i != 16; ++i)
    {
        if (digest[i] < 16)     sprintf(hexBuffer, "0%x", digest[i]);
        else                    sprintf(hexBuffer, "%x", digest[i]);
        result += hexBuffer;
    }

    return result;
}
}
