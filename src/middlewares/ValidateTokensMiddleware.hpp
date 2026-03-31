#pragma once

#include <drogon/HttpMiddleware.h>
#include <string>
#include "../utils/jwt/jwtToken.hpp"
#include "../repositories/sessionRepos.hpp"

/**
 *  @brief Middleware for validating tokens in requests 中间件验证请求令牌
 */
class ValidateTokensMiddleware : public drogon::HttpMiddleware<ValidateTokensMiddleware> {
public:
    ValidateTokensMiddleware() {};// 不要使用 = default;ERROR middleware not found - MiddlewaresFunction.cc:164 NOLINT(*-use-equals-default)

    void invoke(const drogon::HttpRequestPtr &req,
                drogon::MiddlewareNextCallback &&nextCb,
                drogon::MiddlewareCallback &&mcb) override {
        std::clog << "log ValidateTokensMiddleware" << std::endl;

        static auto jwtValidateToken = JwtToken();
        const auto attributes = req->getAttributes();
        const std::string accessToken = attributes->get<std::string>("accessToken");
        const std::string refreshToken = attributes->get<std::string>("refreshToken");

        try {
            if (!jwtValidateToken.validateToken(accessToken)) {
                // access invalid -> do not allow here (this middleware is for endpoints that require a valid access)
                // 进入无效->“在此不允许”（此中间件用于那些需要有效访问权限的端点）
                if (!jwtValidateToken.validateToken(refreshToken)) {
                    throw std::runtime_error("Refresh token is not valid");
                }
                throw std::runtime_error("Access token is not valid");
            } else {
                // access valid -> verify session stored in Redis and that stored refreshToken matches provided refreshToken
                //验证有效 -> 检查存储在 Redis 中的会话以及该存储的刷新令牌是否与所提供的刷新令牌相匹配
                const auto decodedToken = jwt::decode(accessToken);
#if 0
                const auto userId = std::stoi(decodedToken.get_payload_claim("sub").as_string());
#else
                const auto userId = std::stoi(decodedToken.get_subject());
#endif
                // get stored pair from redis by userId and accessToken
                // 根据用户 ID 和访问令牌从 Redis 中获取存储的对数据。
                const JwtToken::TokenPair redisTokenPair = repos::Session(
                        repos::Session::JwtTokens{accessToken, refreshToken})
                .get(userId,repos::Session::JwtTokens{accessToken,refreshToken});

                // 验证双令牌
                if (redisTokenPair.accessToken != accessToken) {
                    throw std::runtime_error("No such access token found");
                }
                if (redisTokenPair.refreshToken != refreshToken) {
                    throw std::runtime_error("Refresh token does not match stored session");
                }
            }
        } catch (const std::exception &e) {
            std::clog << "log "<< e.what() << std::endl;
            Json::Value ret;
            ret["error"] = e.what();
            const auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(drogon::k401Unauthorized);
            mcb(resp);
            return;
        }
        nextCb(std::move(mcb));
    }
};