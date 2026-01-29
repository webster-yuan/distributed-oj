#include "compile_run.hpp"
#include "../comm/httplib.h"
using namespace ns_compile_run;
using namespace httplib;
// 直接使用编译并运行
//./compile_server port
void Usage(std::string proc)
{
    std::cerr << "Usage: " << "\n\t" << proc << "port" << std::endl;
}

// 这样可以打开多个接口
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        return 1;
    }
    Server svr;
    // 获取主机运行状况接口
    svr.Get("/status", [](const Request &req, Response &resp)
            {
                Json::Value status;
                status["cpu_usage"] = HostStatusUtil::get_cpu_usage();
                status["memory_usage"] = HostStatusUtil::get_memory_usage();
                status["network_usage"] = HostStatusUtil::get_network_usage("eth0"); // Change "eth0" to your network interface name
                status["disk_io_usage"] = HostStatusUtil::get_disk_io_usage();
                status["active_connections"] = HostStatusUtil::get_active_connections();
                auto load = HostStatusUtil::get_load_average();
                status["load_average"]["1min"] = load[0];
                status["load_average"]["5min"] = load[1];
                status["load_average"]["15min"] = load[2];
                status["response_time"] = HostStatusUtil::get_response_time();

                Json::StreamWriterBuilder writer;
                std::string output = Json::writeString(writer, status);
                resp.set_content(output, "application/json"); // 设置返回格式为json
            });

    // 将cr转换成网络配置引入cpp-httplib，对外提供服务
    //  postman检验完成
    svr.Post("/compile_and_run", [](const Request &req, Response &resp)
             {
                 std::string in_json = req.body;
                 std::string out_json;
                 if (!in_json.empty())
                 {
                    CompileAndRun::Start(in_json,&out_json);
                    resp.set_content(out_json,"application/json;charset=utf-8");
                 } });

    svr.listen("0.0.0.0", atoi(argv[1]));

    // 通过http，让客户端传来一个json_string
    // std::string in_json;
    // Json::Value in_value;
    // //raw string保持原貌，避免特殊字符
    // in_value["code"]=R"(#include<iostream>
    // int main(){
    //     int a=0;
    //     std::cin>>a;
    //     if(a ==1){
    //         std::cout<<"看见我"<<std::endl;
    //     }
    //     // int* p=new int[1024*1024*40];
    //     return 0;
    // })";

    // in_value["input"]="1";
    // in_value["cpu_limit"]=1;
    // in_value["mem_limit"]=10240*3;//30M
    // Json::FastWriter writer;
    // in_json=writer.write(in_value);
    // //std::cout<<in_json<<std::endl;
    // std::string out_json;//将来返回给客户端返回的串
    // CompileAndRun::Start(in_json,&out_json);
    // std::cout<<out_json<<std::endl;

    return 0;
}