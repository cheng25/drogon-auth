#pragma once

#include <drogon/HttpMiddleware.h>
#include <json/json.h>
#include <regex>

using namespace drogon;

/**
 * @brief 此中间件会验证请求体中的电子邮件和用户名字段。
 *
 */
class ValidateEmailAndUsernameMiddleware final : public HttpMiddleware<ValidateEmailAndUsernameMiddleware> {
public:
    ValidateEmailAndUsernameMiddleware() {};// 不要使用 = default;ERROR middleware not found - MiddlewaresFunction.cc:164 NOLINT(*-use-equals-default)

    void invoke(const HttpRequestPtr &req,
                MiddlewareNextCallback &&nextCb,
                MiddlewareCallback &&mcb) override {
        const auto attributes = req->getAttributes();
        const auto body = attributes->get<std::shared_ptr<Json::Value>>("body");

        try {
            if (!body->isMember("username") || !body->isMember("email")) {
                throw std::runtime_error("Required fields (username or email) are missing");
            }
            const std::string email = (*body)["email"].asString();

            if (const std::regex emailRegex("^[a-zA-Z0-9_+&*-]+(?:\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,7}$");
                !std::regex_match(email, emailRegex)) {
                throw std::runtime_error("Invalid email format");
            }

            if (const std::string username = (*body)["username"].asString();
                username.empty()) {
                throw std::runtime_error("Username cannot be empty");
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
