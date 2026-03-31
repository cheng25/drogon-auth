#include <json/value.h>
#include <json/writer.h>
#include <json/reader.h>
#include "sessionRepos.hpp"

using namespace repos;

void Session::upload(const user_id &user_id) const {
    Json::Value valueJsonData;
    valueJsonData["accessToken"] = jwtTokens_.accessToken;
    valueJsonData["refreshToken"] = jwtTokens_.refreshToken;
    const std::string value = Json::writeString(Json::StreamWriterBuilder(), valueJsonData);
    const std::string redisKey = std::to_string(user_id) + ":" + jwtTokens_.accessToken;
#if 0
    dbClient_.set(redisKey, value);
#else
    // 从 refresh token 读取 exp，计算剩余秒数
    try {
        const auto decoded = jwt::decode(jwtTokens_.refreshToken);
        // jwt-cpp 中 exp 通常是 numeric date（秒），通过 as_int() / as_number() 或类似接口读取
        // 请根据你项目中 jwt-cpp 版本选择正确的读取方法：
        const auto exp = decoded.get_expires_at();
        const auto now = std::chrono::system_clock::now();
        // 计算【未来时间 - 现在】 = 剩余秒数
        const auto remaining_seconds = std::chrono::duration_cast<std::chrono::seconds>(exp - now ).count();

        if (remaining_seconds <= 0) {
            throw std::runtime_error("Refresh token already expired");
        }
        dbClient_.set(redisKey, value, std::chrono::seconds(remaining_seconds));
    } catch (const std::exception &e) {
        // 回退到默认 TTL（30 天）或直接抛出以防写入不一致
        const auto ttl = std::chrono::seconds(60 * 60 * 24 * 30);
        dbClient_.set(redisKey, value, ttl);
    }

#endif
}

const Session::JwtTokens &Session::get(const user_id &user_id, const JwtTokens &jwtTokens) {
    std::string redisKey = std::to_string(user_id) + ":" + jwtTokens.accessToken;
    auto valueOpt = dbClient_.get(redisKey);
    if (!valueOpt) {
        throw std::runtime_error("user ID not found in Redis");
    }
    Json::Value valueJsonData;
    std::string errs;
    std::istringstream valueStream(*valueOpt);
    if (Json::CharReaderBuilder readerBuilder;
        !Json::parseFromStream(readerBuilder, valueStream, &valueJsonData, &errs)) {
        throw std::runtime_error("Failed to parse JSON from Redis: " + errs);
    }
    jwtTokens_.accessToken = valueJsonData["accessToken"].asString();
    jwtTokens_.refreshToken = valueJsonData["refreshToken"].asString();

    return jwtTokens_;
}

void Session::remove(const user_id &user_id, const JwtTokens &jwtTokens) const {
    std::string redisKey = std::to_string(user_id) + ":" + jwtTokens.accessToken;
    auto result = dbClient_.del(redisKey);
    if (result == 0) {
        throw std::runtime_error("Failed to remove user from Redis: Key not found");
    }
}