#include "../include/DouBaoProvider.h"
#include "../include/util/myLog.h"
#include <cstdint>
#include <sstream>
#include <jsoncpp/json/json.h>
#include <httplib.h>
#include <jsoncpp/json/reader.h>

namespace ai_chat_sdk{
    // DouBaoProvider 类
    bool DouBaoProvider::initModel(const std::map<std::string, std::string>& modelConfig){
        // 初始化API Key
        auto it = modelConfig.find("api_key");
        if(it == modelConfig.end()){
            ERR("DouBaoProvider initModel api_key not found");
            return false;
        }else{
            _apiKey = it->second;
        }

        // 初始化Base URL
        it = modelConfig.find("endpoint");
        if(it == modelConfig.end()){
            _endpoint = "https://ark.cn-beijing.volces.com";
        }else{
            _endpoint = it->second;
        }

        _isAvailable = true;

        INFO("DouBaoProvider initModel success, endpoint: {}",
             _endpoint);

        return true;
    }

    // 检测模型是否可用
    bool DouBaoProvider::isAvailable() const{
        return _isAvailable;
    }

    // 获取模型名称
    std::string DouBaoProvider::getModelName() const{
        return "doubao-seed-2-0-mini-260215";
    }

    // 获取模型的描述信息
    std::string DouBaoProvider::getModelDesc() const{
        return "面向低时延、高并发与成本敏感场景，强调快速响应与灵活推理部署，支持四档位思考与多模态理解能力。";
    }

    // 发送消息
    std::string DouBaoProvider::sendMessage(
        const std::vector<Message>& messages,
        const std::map<std::string, std::string>& requestParam)
    {
        // 1. 检测模型是否可用
        if(!isAvailable()){
            ERR("DouBaoProvider sendMessage model not available");
            return "";
        }

        // 2. 构造请求参数
        double temperature = 0.7;
        int maxTokens = 2048;

        if(requestParam.find("temperature") != requestParam.end()){
            temperature = std::stod(requestParam.at("temperature"));
        }

        if(requestParam.find("max_tokens") != requestParam.end()){
            maxTokens = std::stoi(requestParam.at("max_tokens"));
        }

        // 构造历史消息
        Json::Value messageArray(Json::arrayValue);

        for(const auto& message : messages){
            Json::Value messageObject;
            messageObject["role"] = message._role;
            messageObject["content"] = message._content;
            messageArray.append(messageObject);
        }

        // 3. 构造请求体
        Json::Value requestBody;
        requestBody["model"] = getModelName();
        requestBody["messages"] = messageArray;
        requestBody["temperature"] = temperature;
        requestBody["max_tokens"] = maxTokens;

        // 4. 序列化
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = "";

        std::string requestBodyStr =
            Json::writeString(writerBuilder, requestBody);

        INFO("DouBaoProvider sendMessage requestBody: {}",
             requestBodyStr);

        // 5. 使用cpp-httplib库构造HTTP客户端
        httplib::Client client(_endpoint.c_str());
        client.set_connection_timeout(30, 0);
        client.set_read_timeout(60, 0);

        // 设置请求头
        httplib::Headers headers = {
            {"Authorization", "Bearer " + _apiKey},
            {"Content-Type", "application/json"}
        };

        // 6. 发送POST请求
        auto response = client.Post(
            "/api/v3/chat/completions",
            headers,
            requestBodyStr,
            "application/json"
        );

        if(!response){
            ERR("DouBaoProvider sendMessage POST request failed");
            return "";
        }

        INFO("DouBaoProvider sendMessage success, status:{}",
             response->status);

        INFO("DouBaoProvider sendMessage body:{}",
             response->body);

        // 检测响应是否成功
        if(response->status != 200){
            return "";
        }

        // 7. 解析响应体
        Json::Value responseBody;
        Json::CharReaderBuilder readerBuilder;
        std::string parseError;
        std::istringstream responseStream(response->body);

        if(Json::parseFromStream(
            readerBuilder,
            responseStream,
            &responseBody,
            &parseError))
        {
            if(responseBody.isMember("choices") &&
               responseBody["choices"].isArray() &&
               !responseBody["choices"].empty())
            {
                auto choice = responseBody["choices"][0];

                if(choice.isMember("message") &&
                   choice["message"].isMember("content"))
                {
                    std::string replyContent =
                        choice["message"]["content"].asString();

                    INFO("DouBaoProvider response text:{}",
                         replyContent);

                    return replyContent;
                }
            }
        }

        // 8. json解析失败
        ERR("DouBaoProvider response json parse failed");
        return "doubao response json parse failed";
    }

    // 发送消息 - 增量返回 - 流式响应
    std::string DouBaoProvider::sendMessageStream(
        const std::vector<Message>& messages,
        const std::map<std::string, std::string>& requestParam,
        std::function<void(const std::string&, bool)> callback)
    {
        // 1. 检测模型是否可用
        if(!isAvailable()){
            ERR("DouBaoProvider sendMessageStream model not available");
            return "";
        }

        // 2. 构造请求参数
        double temperature = 0.7;
        int maxTokens = 2048;

        if(requestParam.find("temperature") != requestParam.end()){
            temperature = std::stod(requestParam.at("temperature"));
        }

        if(requestParam.find("max_tokens") != requestParam.end()){
            maxTokens = std::stoi(requestParam.at("max_tokens"));
        }

        // 构造历史消息
        Json::Value messageArray(Json::arrayValue);

        for(const auto& message : messages){
            Json::Value messageObject;
            messageObject["role"] = message._role;
            messageObject["content"] = message._content;
            messageArray.append(messageObject);
        }

        // 3. 构造请求体
        Json::Value requestBody;
        requestBody["model"] = getModelName();
        requestBody["messages"] = messageArray;
        requestBody["temperature"] = temperature;
        requestBody["max_tokens"] = maxTokens;
        requestBody["stream"] = true;

        // 4. 序列化
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = "";

        std::string requestBodyStr =
            Json::writeString(writerBuilder, requestBody);

        INFO("DouBaoProvider sendMessageStream requestBody:{}",
             requestBodyStr);

        // 5. 使用cpp-httplib库构造HTTP客户端
        httplib::Client client(_endpoint.c_str());
        client.set_connection_timeout(30, 0);
        client.set_read_timeout(300, 0);

        // 设置请求头
        httplib::Headers headers = {
            {"Authorization", "Bearer " + _apiKey},
            {"Content-Type", "application/json"},
            {"Accept", "text/event-stream"}
        };

        // 流式处理变量
        std::string buffer;
        bool gotError = false;
        bool streamFinish = false;
        std::string fullResponse;

        // 创建请求对象
        httplib::Request req;
        req.method = "POST";
        req.path = "/api/v3/chat/completions";
        req.headers = headers;
        req.body = requestBodyStr;

        // 设置响应处理器
        req.response_handler = [&](const httplib::Response& res){
            if(res.status != 200){
                gotError = true;
                return false;
            }
            return true;
        };

        // 设置数据接收处理器
        req.content_receiver =
        [&](const char* data, size_t len, size_t, size_t){

            if(gotError){
                return false;
            }

            buffer.append(data, len);

            size_t pos = 0;

            while((pos = buffer.find("\n\n")) != std::string::npos){

                std::string chunk = buffer.substr(0, pos);
                buffer.erase(0, pos + 2);

                if(chunk.empty() || chunk[0] == ':'){
                    continue;
                }

                if(chunk.compare(0, 6, "data: ") == 0){

                    std::string modelData = chunk.substr(6);

                    if(modelData == "[DONE]"){
                        callback("", true);
                        streamFinish = true;
                        return true;
                    }

                    Json::Value modelDataJson;
                    Json::CharReaderBuilder reader;
                    std::string errors;
                    std::istringstream ss(modelData);

                    if(Json::parseFromStream(
                        reader,
                        ss,
                        &modelDataJson,
                        &errors))
                    {
                        if(modelDataJson.isMember("choices") &&
                           modelDataJson["choices"].isArray() &&
                           !modelDataJson["choices"].empty())
                        {
                            auto choice =
                                modelDataJson["choices"][0];

                            if(choice.isMember("delta") &&
                               choice["delta"].isMember("content"))
                            {
                                std::string content =
                                    choice["delta"]["content"]
                                    .asString();

                                fullResponse += content;

                                callback(content, false);
                            }
                        }
                    }
                }
            }

            return true;
        };

        // 给模型发送请求
        auto result = client.send(req);

        if(!result){
            ERR("DouBaoProvider stream request failed");
            return "";
        }

        // 确保流式操作正确结束
        if(!streamFinish){
            callback("", true);
        }

        return fullResponse;
    }

}