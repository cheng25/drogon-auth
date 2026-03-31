#pragma once

#include <drogon/HttpMiddleware.h>
#include <string>
#include "../utils/jwt/jwtToken.hpp"
#include "../repositories/sessionRepos.hpp"

using namespace drogon;

inline size_t getUserId(const std::string &refreshToken) {
    const auto decodedToken = jwt::decode(refreshToken);
    return std::stoi(decodedToken.get_payload_claim("sub").as_string());
}
/**
 * @brief 从请求体和 Cookie 中提取访问令牌和刷新令牌
 *
 * 该中间件从请求体和 Cookie 中提取访问令牌和刷新令牌，并将它们存储在请求属性中。
 * 如果请求体中缺少 accessToken 字段或 Cookie 中缺少 refreshToken 字段，会抛出异常。
 */
class TokenExtractionMiddleware : public HttpMiddleware<TokenExtractionMiddleware> {
public:
    TokenExtractionMiddleware(){}; // 不要使用 = default;ERROR middleware not found - MiddlewaresFunction.cc:164 NOLINT(*-use-equals-default)
    /**
     * @brief 从请求体和 Cookie 中提取访问令牌和刷新令牌
     * @param req 请求指针
     * @param nextCb 下一个中间件的回调函数
     * @param mcb 中 nextCb 中调用的回调函数，用于处理响应
     */
    void invoke(const HttpRequestPtr &req,
                MiddlewareNextCallback &&nextCb,
                MiddlewareCallback &&mcb) override {
        std::clog << "log TokenExtractionMiddleware" << std::endl;

        try {
            auto body = req->getJsonObject();
            if (!body) {
                throw std::runtime_error("Invalid JSON body");
            }

            if (!body->isMember("accessToken")) {
                throw std::runtime_error("The body is missing the required field accessToken");
            }

            auto cookies = req->getCookies();
            auto refreshTokenIt = cookies.find("refreshToken");

            if (refreshTokenIt == cookies.end()) {
                throw std::runtime_error("Missing refreshToken in cookies");
            }

            std::string refreshToken = refreshTokenIt->second;
            std::string accessToken = (*body)["accessToken"].asString();

            req->getAttributes()->insert("accessToken", accessToken);
            req->getAttributes()->insert("refreshToken", refreshToken);
        } catch (const std::exception &e) {
            std::clog << "log "<< e.what() << std::endl;
            Json::Value ret;
            ret["error"] = e.what();
            auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(drogon::k400BadRequest);
            mcb(resp);
            return;
        }
        nextCb(std::move(mcb));
    }
};