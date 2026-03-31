#pragma once

#include <drogon/HttpMiddleware.h>
#include <string>
#include "../utils/jwt/jwtToken.hpp"
#include "../repositories/sessionRepos.hpp"

class ValidateRefreshMiddleware : public drogon::HttpMiddleware<ValidateRefreshMiddleware> {
public:
    ValidateRefreshMiddleware() {};

    void invoke(const drogon::HttpRequestPtr &req,
                drogon::MiddlewareNextCallback &&nextCb,
                drogon::MiddlewareCallback &&mcb) override {
        std::clog << "log ValidateRefreshMiddleware" << std::endl;

        static auto jwtValidator = JwtToken();
        const auto attributes = req->getAttributes();
        const std::string accessToken = attributes->get<std::string>("accessToken");
        const std::string refreshToken = attributes->get<std::string>("refreshToken");

        try {
            // refresh token must be structurally & cryptographically valid (signature, issuer, expiry)
            // 刷新令牌必须在结构上和加密方式上均有效（包括签名、颁发者和有效期）
            if (!jwtValidator.validateToken(refreshToken)) {
                throw std::runtime_error("Refresh token is not valid");
            }

            // decode refresh to get user id
            // 解码刷新以获取用户 ID
            const auto decodedRefresh = jwt::decode(refreshToken);
            const auto userId = std::stoi(decodedRefresh.get_subject());

            // look up session in Redis using userId + accessToken (this repo currently stores sessions keyed by accessToken)
            // 使用用户 ID 和访问令牌在 Redis 中查找会话（此存储库目前是按照访问令牌来对会话进行分类存储的）
            const auto storedPair = repos::Session(
                    repos::Session::JwtTokens{accessToken, refreshToken})
            .get(userId,repos::Session::JwtTokens{accessToken,refreshToken});

            // verify stored refresh matches the provided refresh
            // 验证存储的刷新值是否与所提供的刷新值一致
            if (storedPair.refreshToken != refreshToken) {
                throw std::runtime_error("Refresh token does not match stored session");
            }

            // passed refresh checks -> allow 通过了刷新检查 -> 允许
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