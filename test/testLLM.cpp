#include <gtest/gtest.h>
#include <istream>
#include <memory>
#include <spdlog/common.h>

#include "../sdk/include/DeepSeekProvider.h"
#include "../sdk/include/DouBaoProvider.h"
#include "../sdk/include/QWenProvider.h"

#include "../sdk/include/util/myLog.h"

#include <iostream>
#include <string>
#include <vector>

//////////////////////////////////////////////////////////////
// DeepSeek 测试（保留原代码，直接注释）
//////////////////////////////////////////////////////////////

/*
TEST(DeepSeekProviderTest, sendMessage){

    auto provider =
        std::make_shared<ai_chat_sdk::DeepSeekProvider>();

    ASSERT_TRUE(provider != nullptr);

    std::map<std::string, std::string> modelParam;

    modelParam["api_key"] =
        std::getenv("deepseek_apikey");

    modelParam["endpoint"] =
        "https://api.deepseek.com";

    provider->initModel(modelParam);

    ASSERT_TRUE(provider->isAvailable());

    std::map<std::string, std::string> requestParam = {
        {"temperature", "0.7"},
        {"max_tokens", "2048"}
    };

    std::vector<ai_chat_sdk::Message> messages;

    messages.push_back({"user", "你是谁？"});

    auto writeChunk =
        [&](const std::string& chunk, bool last){

        INFO("chunk : {}", chunk);

        if(last){
            INFO("[DONE]");
        }
    };

    std::string fullData =
        provider->sendMessageStream(
            messages,
            requestParam,
            writeChunk
        );

    ASSERT_FALSE(fullData.empty());

    INFO("response : {}", fullData);
}
*/

//////////////////////////////////////////////////////////////
// DouBao 测试（保留原代码，直接注释）
//////////////////////////////////////////////////////////////

/*
TEST(DouBaoProviderTest, sendMessage){

    auto provider =
        std::make_shared<ai_chat_sdk::DouBaoProvider>();

    ASSERT_TRUE(provider != nullptr);

    std::map<std::string, std::string> modelParam;

    modelParam["api_key"] =
        std::getenv("doubao_apikey");

    modelParam["endpoint"] =
        "https://ark.cn-beijing.volces.com";

    modelParam["model"] =
        "Doubao-Seed-2.0-mini";

    provider->initModel(modelParam);

    ASSERT_TRUE(provider->isAvailable());

    std::map<std::string, std::string> requestParam = {
        {"temperature", "0.7"},
        {"max_tokens", "2048"}
    };

    std::vector<ai_chat_sdk::Message> messages;

    messages.push_back({"user", "你是谁？"});

    auto writeChunk =
        [&](const std::string& chunk, bool last){

        INFO("chunk : {}", chunk);

        if(last){
            INFO("[DONE]");
        }
    };

    std::string fullData =
        provider->sendMessageStream(
            messages,
            requestParam,
            writeChunk
        );

    ASSERT_FALSE(fullData.empty());

    INFO("response : {}", fullData);
}
*/

//////////////////////////////////////////////////////////////
// QWen 测试
//////////////////////////////////////////////////////////////

TEST(QWenProviderTest, sendMessage){

    auto provider =
        std::make_shared<ai_chat_sdk::QWenProvider>();

    ASSERT_TRUE(provider != nullptr);

    std::map<std::string, std::string> modelParam;

    modelParam["api_key"] =
        std::getenv("qwen_apikey");

    modelParam["endpoint"] =
        "https://dashscope.aliyuncs.com";

    modelParam["model"] =
        "qwen-turbo";

    provider->initModel(modelParam);

    ASSERT_TRUE(provider->isAvailable());

    std::map<std::string, std::string> requestParam = {
        {"temperature", "0.7"},
        {"max_tokens", "2048"}
    };

    std::vector<ai_chat_sdk::Message> messages;

    messages.push_back({"user", "你是谁？"});

    // 普通调用（如需测试取消注释）
    /*
    std::string response =
        provider->sendMessage(
            messages,
            requestParam
        );

    ASSERT_FALSE(response.empty());

    INFO("response : {}", response);
    */

    // 流式调用
    auto writeChunk =
        [&](const std::string& chunk, bool last){

        INFO("chunk : {}", chunk);

        if(last){
            INFO("[DONE]");
        }
    };

    std::string fullData =
        provider->sendMessageStream(
            messages,
            requestParam,
            writeChunk
        );

    ASSERT_FALSE(fullData.empty());

    INFO("response : {}", fullData);
}

//////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////

int main(int argc, char **argv) {

    // 初始化spdlog日志库
    bite::Logger::initLogger(
        "testLLM",
        "stdout",
        spdlog::level::debug
    );

    // 初始化gtest库
    testing::InitGoogleTest(&argc, argv);

    // 执行所有测试用例
    return RUN_ALL_TESTS();
}