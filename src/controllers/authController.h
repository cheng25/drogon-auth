#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class authController : public drogon::HttpController<authController> {
public:

    METHOD_LIST_BEGIN
        //注册新用户（用户名、邮箱、密码）
        ADD_METHOD_TO(authController::signUp, "/sign-up", Post, "ValidateRequestBodyMiddleware",
                      "ValidateEmailAndUsernameMiddleware", "ValidatePasswordMiddleware");
        //身份验证并获取访问令牌 + 刷新令牌 Cookie
        ADD_METHOD_TO(authController::signIn, "/sign-in", Post, "ValidateRequestBodyMiddleware",
                      "ValidateEmailAndUsernameMiddleware", "ValidatePasswordMiddleware");
        // 撤销当前会话
        ADD_METHOD_TO(authController::logout, "/logout", Post, "TokenExtractionMiddleware");
        // 刷新令牌；颁发新的令牌对
#if 0
        ADD_METHOD_TO(authController::getNewAccessToken, "/getNewAccessToken",
                      Post, "TokenExtractionMiddleware", "ValidateTokensMiddleware");
#else
        // 修改：为刷新单独使用 ValidateRefreshMiddleware
        ADD_METHOD_TO(authController::getNewAccessToken, "/getNewAccessToken",
                      Post, "TokenExtractionMiddleware", "ValidateRefreshMiddleware");
#endif

        // 更新用户密码
        ADD_METHOD_TO(authController::changePassword, "/changePassword", Post, "ValidateRequestBodyMiddleware",
                      "ValidatePasswordMiddleware", "TokenExtractionMiddleware", "ValidateTokensMiddleware");
    METHOD_LIST_END

    static void signUp(const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback);

    static void signIn(const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback);

    static void logout(const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback);

    static void getNewAccessToken(const HttpRequestPtr &req,
                           std::function<void(const HttpResponsePtr &)> &&callback);

   static void changePassword(const HttpRequestPtr &req,
                        std::function<void(const HttpResponsePtr &)> &&callback);
};