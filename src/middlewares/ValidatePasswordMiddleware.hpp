#pragma once

#include <drogon/HttpMiddleware.h>
#include <json/json.h>
#include <regex>

using namespace drogon;

/**
 *  @brief 密码验证中间件
 */
class ValidatePasswordMiddleware : public HttpMiddleware<ValidatePasswordMiddleware> {
public:
    ValidatePasswordMiddleware() {}; // 不要使用 = default;ERROR middleware not found - MiddlewaresFunction.cc:164 NOLINT(*-use-equals-default)

    void invoke(const HttpRequestPtr &req,
                MiddlewareNextCallback &&nextCb,
                MiddlewareCallback &&mcb) override {
        const auto attributes = req->getAttributes();
        const auto body = attributes->get<std::shared_ptr<Json::Value>>("body");

        try {
            if (!body->isMember("password")) {
                throw std::runtime_error("Required field password is missing");
            }

            if (const std::string password = (*body)["password"].asString();
                password.length() < 8) {
                throw std::runtime_error("Password must be at least 8 characters long");
            }
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
