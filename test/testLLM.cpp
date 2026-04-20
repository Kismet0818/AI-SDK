#include <gtest/gtest.h>
#include <istream>
#include <memory>
#include <spdlog/common.h>
#include "../sdk/include/DeepSeekProvider.h"
#include "../sdk/include/util/myLog.h"
#include <iostream>
#include <string>
#include <vector>

TEST(DeepSeekProviderTest, sendMessage){
    auto provider = std::make_shared<ai_chat_sdk::DeepSeekProvider>();
    ASSERT_TRUE(provider != nullptr);

    std::map<std::string, std::string> modelParam;
    modelParam["api_key"] = std::getenv("deepseek_apikey");
    modelParam["endpoint"] = "https://api.deepseek.com";

    provider->initModel(modelParam);
    ASSERT_TRUE(provider->isAvailable());

    std::map<std::string, std::string> requestParam = {
        {"temperature", "0.7"},
        {"max_tokens", "2048"}
    };
    std::vector<ai_chat_sdk::Message> messages;
    messages.push_back({"user", "你是谁？"});

    // 实例化DeepSeekProvider的对象
    // 调用sendMessage方法
    std::string response = provider->sendMessage(messages, requestParam);
    ASSERT_FALSE(response.empty());
}

int main(int argc, char **argv) {
    // 初始化spdlog日志库
    bite::Logger::initLogger("testLLM", "stdout", spdlog::level::debug);

    // 初始化gtest库
    testing::InitGoogleTest(&argc, argv);

    // 执行所有的测试用例
    return RUN_ALL_TESTS();
}