// #include <iostream>
// #include <string>
// #include <sstream>
// #include <curl/curl.h>
// #include <jsoncpp/json/json.h>
// #include "comm/httplib.h"
// // 使用httplib库的命名空间
// using namespace httplib;

// // 回调函数，用于处理curl的返回数据
// size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
//     size_t totalSize = size * nmemb;
//     s->append((char*)contents, totalSize);
//     return totalSize;
// }

// // 调用OpenAI API生成代码
// std::string generate_code_from_title(const std::string& title) {
//     std::string api_url = "https://api.openai.com/v1/completions";
//     std::string api_key = "YOUR_API_KEY";  // 请替换为你的API密钥

//     std::string prompt = "Generate C++ code for the following task: " + title + "\n\n";
//     Json::Value jsonData;
//     jsonData["model"] = "text-davinci-003";
//     jsonData["prompt"] = prompt;
//     jsonData["max_tokens"] = 300;
//     jsonData["temperature"] = 0.5;

//     Json::StreamWriterBuilder writer;
//     std::string jsonString = Json::writeString(writer, jsonData);

//     CURL* curl;
//     CURLcode res;
//     curl_global_init(CURL_GLOBAL_DEFAULT);
//     curl = curl_easy_init();

//     std::string response_string;
//     if (curl) {
//         struct curl_slist* headers = NULL;
//         headers = curl_slist_append(headers, "Content-Type: application/json");
//         headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());

//         curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
//         curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//         curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());

//         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//         curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

//         res = curl_easy_perform(curl);
//         if (res != CURLE_OK) {
//             std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
//         }
//         curl_easy_cleanup(curl);
//     }
//     curl_global_cleanup();

//     Json::CharReaderBuilder reader;
//     Json::Value root;
//     std::string errs;
//     std::istringstream s(response_string);
//     if (!Json::parseFromStream(reader, s, &root, &errs)) {
//         std::cerr << "Failed to parse response: " << errs << std::endl;
//         return "";
//     }

//     std::string generated_code = root["choices"][0]["text"].asString();
//     return generated_code;
// }

// int main() {
//     std::string title = "判断是否是回文数字";
//     std::string generated_code = generate_code_from_title(title);
//     std::cout << generated_code << std::endl;

//     return 0;
// }
