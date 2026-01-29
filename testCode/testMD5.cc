#include <iostream>
#include <string>
#include <cstdio>
#include <openssl/md5.h> // 需要安装 OpenSSL 库

#include <sstream>
#include <random>
// 生成 MD5 值的函数
std::string generateMD5(const std::string &input)
{
    MD5_CTX context;
    MD5_Init(&context);
    MD5_Update(&context, input.c_str(), input.length());

    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_Final(digest, &context);

    char mdString[33];
    for (int i = 0; i < 16; i++)
        sprintf(&mdString[i * 2], "%02x", (unsigned int)digest[i]);

    return std::string(mdString);
}

// 生成带盐的 MD5 值
std::string generateSaltedMD5(const std::string &password, const std::string &salt)
{
    return generateMD5(password + salt);
}

unsigned int random_char()
{
    // 用于随机数引擎获得随机种子
    std::random_device rd;
    // mt19937是c++11新特性，它是一种随机数算法，用法与rand()函数类似，但是mt19937具有速度快，周期长的特点
    // 作用是生成伪随机数
    std::mt19937 gen(rd());
    // 随机生成一个整数i 范围[0, 255]
    std::uniform_int_distribution<> dis(0, 255);
    return dis(gen);
}

// 生成 UUID （通用唯一标识符）
std::string generate_hex(const unsigned int len)
{
    std::stringstream ss;
    // 生成 len 个16进制随机数，将其拼接而成
    for (auto i = 0; i < len; i++)
    {
        const auto rc = random_char();
        std::stringstream hexstream;
        hexstream << std::hex << rc;
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}

int main()
{
    std::string password = "12345";
    std::string salt = "c94d5164703ec1189d2d";
    std::string encryptedPassword1 = generateSaltedMD5(password, salt);

    // 测试
    std::string encryptedPassword2 = "2a17cfb566fca64afb97419a59928eaa";
    std::cout<< (encryptedPassword1==encryptedPassword2) << std::endl;
    return 0;
}
