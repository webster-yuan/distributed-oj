#pragma once

#include <iostream>
#include <string>
#include <sys/types.h>
#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <atomic>
#include <boost/algorithm/string.hpp>
#include <unistd.h>
#include <sys/time.h>
#include <random>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <openssl/md5.h> // 需要安装 OpenSSL 库
#include "httplib.h"
#include <sstream>
#include <random>
#include <sys/sysinfo.h>
// 提供公共工具
namespace ns_util
{
    const std::string temp_path = "./temp/";

    class TimeUtil
    {
    public:
        static std::string GetTimeStamp()
        {
            // C->time函数
            // 输出型参数timeval，所以需要传入
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec);
        }
        // 获得毫秒级别时间戳
        static std::string GetTimeMs()
        {
            struct timeval _time;
            // 输出型参数，所以需要传入timeval结构
            gettimeofday(&_time, nullptr); // 第二个是时区，默认是nullptr就行

            return std::to_string(_time.tv_sec * 1000 + _time.tv_usec / 1000);
        }
    };
    class HostStatusUtil
    {
    public:
        // 主机状态
        static float get_cpu_usage()
        {
            std::ifstream proc_stat("/proc/stat");
            std::string line;
            getline(proc_stat, line);
            std::istringstream ss(line);

            std::string cpu;
            long user, nice, system, idle;
            ss >> cpu >> user >> nice >> system >> idle;

            static long prev_idle = 0, prev_total = 0;
            long total = user + nice + system + idle;
            long totald = total - prev_total;
            long idled = idle - prev_idle;

            prev_total = total;
            prev_idle = idle;

            return (totald - idled) * 100.0 / totald;
        }
        // 内存使用率
        static float get_memory_usage()
        {
            struct sysinfo memInfo;
            sysinfo(&memInfo);
            long long totalPhysMem = memInfo.totalram;
            totalPhysMem *= memInfo.mem_unit;
            long long physMemUsed = memInfo.totalram - memInfo.freeram;
            physMemUsed *= memInfo.mem_unit;
            return (physMemUsed * 100.0) / totalPhysMem;
        }
        // 网络IO使用率
        static float get_network_usage(const std::string &iface)
        {
            static long prev_rx_bytes = 0, prev_tx_bytes = 0;
            std::ifstream net_dev("/proc/net/dev");
            std::string line;
            long rx_bytes, tx_bytes;

            while (getline(net_dev, line))
            {
                if (line.find(iface) != std::string::npos)
                {
                    std::istringstream ss(line);
                    std::string iface_name;
                    ss >> iface_name >> rx_bytes;
                    for (int i = 0; i < 7; ++i)
                        ss >> tx_bytes;
                    break;
                }
            }

            long rx_diff = rx_bytes - prev_rx_bytes;
            long tx_diff = tx_bytes - prev_tx_bytes;

            prev_rx_bytes = rx_bytes;
            prev_tx_bytes = tx_bytes;

            return (rx_diff + tx_diff) / 1024.0; // in KB/s
        }
        // 磁盘IO使用率
        static float get_disk_io_usage()
        {
            static long prev_read_bytes = 0, prev_write_bytes = 0;
            std::ifstream disk_stat("/proc/diskstats");
            std::string line;
            long read_bytes = 0, write_bytes = 0;

            while (getline(disk_stat, line))
            {
                if (line.find("sda") != std::string::npos)
                {
                    std::istringstream ss(line);
                    std::string ignore;
                    long reads, writes;
                    for (int i = 0; i < 5; ++i)
                        ss >> ignore;
                    ss >> reads >> ignore >> ignore >> write_bytes;
                    read_bytes = reads * 512;
                    write_bytes = write_bytes * 512;
                    break;
                }
            }

            long read_diff = read_bytes - prev_read_bytes;
            long write_diff = write_bytes - prev_write_bytes;

            prev_read_bytes = read_bytes;
            prev_write_bytes = write_bytes;

            return (read_diff + write_diff) / 1024.0; // in KB/s
        }
        // 主机网络连接数
        static int get_active_connections()
        {
            std::ifstream netstat("/proc/net/tcp");
            std::string line;
            int count = 0;

            while (getline(netstat, line))
            {
                if (line.find("0A") != std::string::npos)
                { // 0A means ESTABLISHED state
                    count++;
                }
            }
            return count;
        }
        // 主机负载
        static std::vector<double> get_load_average()
        {
            std::vector<double> load(3);
            getloadavg(load.data(), 3);
            return load;
        }
        // 主机响应时间
        static float get_response_time()
        {
            using namespace std::chrono;
            auto start = high_resolution_clock::now();
            // Simulate a quick operation
            std::this_thread::sleep_for(milliseconds(10));
            auto end = high_resolution_clock::now();
            duration<float, std::milli> duration = end - start;
            return duration.count();
        }
    };
    class PathUtil // 添加文件后缀和路径
    {
    public:
        // 添加后缀
        static std::string AddSuffix(const std::string &file_name, const std::string &suffix)
        {
            std::string path_name = temp_path;
            path_name += file_name;
            path_name += suffix;
            return path_name;
        }
        // 编译时需要的临时文件
        //  构建源文件路径 + 后缀的完整文件名
        //  1234 -> ./temp/1234.cpp
        static std::string Src(const std::string &file_name)
        {
            return AddSuffix(file_name, ".cpp");
        }
        // 构建可执行程序的路径名+后缀
        // 1234 -> ./temp/1234.exe
        static std::string Exe(const std::string &file_name)
        {
            return AddSuffix(file_name, ".exe");
        }
        // 构建程序对应的标准错误完整路径名+后缀名
        // 1234 -> ./temp/1234.compile_err
        static std::string CompilerError(const std::string &file_name)
        {
            return AddSuffix(file_name, ".compile_err");
        }

        // 运行时需要的临时文件
        static std::string Stderr(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stderr");
        }
        static std::string Stdin(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stdin");
        }
        static std::string Stdout(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stdout");
        }
    };
    class FileUtil
    {
    public:
        static bool IsFileExits(const std::string &path_name)
        {
            // 获取文件属性成功与否
            struct stat st;
            if (stat(path_name.c_str(), &st) == 0)
            {
                // 获取文件属性成功
                return true;
            }
            return false;
        }
        static bool WriteFile(const std::string &target, const std::string &code)
        {
            std::ofstream out(target); // 默认是输出
            if (!out.is_open())
            {
                return false;
            }
            out.write(code.c_str(), code.size());
            out.close();
            return true;
        }
        // 形成唯一的文件名，毫秒值时间戳和原子性递增唯一值
        static std::string UniqFileName()
        {
            static std::atomic_uint id(0);
            id++;
            std::string ms = TimeUtil::GetTimeMs();
            std::string uniq_id = std::to_string(id);
            return ms + "_" + uniq_id;
        }
        static bool ReadFile(const std::string &target, std::string *content, bool keep = false) // 读取编译错误的文件，返回内容给用户
        {
            (*content).clear();
            // std::ifstream in(target,std::ios::in);
            std::ifstream in(target);
            if (!in.is_open())
            {
                return false;
            }
            std::string line;
            // 不保存行分隔符，有些时候需要保留\n,传参时传入是否保留\n的Keep参数
            // 内部重载了强制类型转换，返回值变成bool
            while (std::getline(in, line)) // 从流里面按行读取到line，
            {
                (*content) += line;
                (*content) += (keep ? "\n" : "");
            }
            in.close();
            return true;
        }
    };
    class StringUtil
    {
    public:
        // 切分字符串，目标切分字符串，切分之后的部分放到vector中(输出型)，指定的分隔符
        static void SplitString(std::string &str, std::vector<std::string> *target, std::string sep)
        {
            // boost split方法
            boost::split((*target), str, boost::is_any_of(sep), boost::algorithm::token_compress_on);
        }
        static bool isValidNumber(const std::string &str)
        {
            if (str.empty())
                return false;
            for (char c : str)
            {
                if (!std::isdigit(c))
                    return false;
            }
            return true;
        }
        static std::string timeToString(time_t t)
        {
            struct tm *tm = localtime(&t);
            char buffer[30];
            strftime(buffer, sizeof(buffer), "'%Y-%m-%d %H:%M:%S'", tm);
            return std::string(buffer);
        }
        static std::string timeToStringForFrontEnd(time_t t)
        {
            struct tm *tm = localtime(&t);
            char buffer[30];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm); // 去掉了格式字符串中的单引号
            return std::string(buffer);
        }
        // 将字符串时间格式转换为 time_t 类型
        static time_t convertStringToTimeT(const std::string &timestamp)
        {
            struct tm tm;
            // 解析时间字符串，格式根据你的数据库中 timestamp 的格式而定
            strptime(timestamp.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
            // 将 struct tm 转换为 time_t 类型
            time_t time = mktime(&tm);
            return time;
        }
    };
    class SessionUtil
    {
    public:
        static std::string generate_alpha_numeric_session_id(size_t length)
        {
            const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
            std::random_device rd;
            std::mt19937 generator(rd());
            std::uniform_int_distribution<> distribution(0, chars.size() - 1);

            std::string session_id;
            for (size_t i = 0; i < length; ++i)
            {
                session_id += chars[distribution(generator)];
            }

            return session_id;
        }
        static std::string get_cookie_value(const httplib::Request &req, const std::string &cookie_name)
        {
            auto cookie_header = req.get_header_value("Cookie");
            if (cookie_header.empty())
            {
                return "";
            }

            std::istringstream cookie_stream(cookie_header);
            std::string cookie;
            while (std::getline(cookie_stream, cookie, ';'))
            {
                auto pos = cookie.find_first_not_of(" ");
                if (pos != std::string::npos)
                {
                    cookie.erase(0, pos);
                }
                pos = cookie.find('=');
                if (pos != std::string::npos)
                {
                    std::string key = cookie.substr(0, pos);
                    std::string value = cookie.substr(pos + 1);
                    if (key == cookie_name)
                    {
                        return value;
                    }
                }
            }
            return "";
        }
    };
    class MD5Util
    {
    private:
        // 生成 MD5 值的函数
        static std::string generateMD5(const std::string &input)
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

    public:
        // 生成带盐的 MD5 值
        static std::string generateSaltedMD5(const std::string &password, const std::string &salt)
        {
            return generateMD5(password + salt);
        }
    };
    class UUIDUtil
    {
    private:
        static unsigned int random_char()
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

    public:
        // 生成 UUID （通用唯一标识符）
        static std::string generate_hex(const unsigned int len)
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
    };
}