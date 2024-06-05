#ifndef ___CDI_API_H__
#define ___CDI_API_H__

#include <vector>
#include <string>

namespace api {

int to_lower(int c);

int to_upper(int c);

bool is_any_of(char c, char* s);

void trim(std::string& str, const char* delim = " \t\r");

bool split(const char* str, const char* delim, std::vector<std::string>& vec);

void split(const std::string& str, const char* delim, std::vector<std::string>& vec);

bool is_digit(std::string str);

int str2int(std::string str);

double str2double(std::string str);

//int str2int(std::string& str);

int str2int(const char* str);

std::string int2str(int num);

std::string int2str(unsigned int num);

std::string u8_to_ip(unsigned char* u8Ip);

char* get_short_name(char* path);

std::string get_gmt_time();

std::string get_local_time();

std::string get_fmt_time();

std::string get_fmt_time(unsigned int sec);

std::string to_hms(unsigned int sec);

std::string uuid(bool simple = false);

bool file_existed(const char* fn);

bool is_ip_v4_addr(std::string ip);

bool calc_cpuoccupy(int &total, int &free);

bool calc_memoccupy(int &total, int &free);

bool calc_diskoccupy(int &total, int &free);

int make_socket_non_blocking(int sfd);

uint64_t get_current_timestamp();

unsigned int getClientID();

std::string encrypt_cbc(const std::string &plaintext, const std::string &skey, const std::string &siv);

std::string decrypt_cbc(const std::string &encrypttext, const std::string &skey, const std::string &siv);

bool encrypt_base64(const unsigned char* in, int inlen, char* out, int* outLen, bool newline = false);

bool decrypt_base64(const char* in, int inlen, unsigned char* out, int* outLen, bool newline = false);

std::string getMd5Str(const char* data, int length);
}
#endif
