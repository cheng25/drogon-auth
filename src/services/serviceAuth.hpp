#pragma once

#include "../models/user.hpp"
#include "../utils/jwt/jwtToken.hpp"

class AuthService {
public:
    using Id = size_t;
    struct UserData {
        JwtToken::TokenPair TokenPair;
        Id id;
    };
public:
    //注册
    static void registration(const user &);

    // 登录
    static UserData login(const user &);

    //登出
    static void logout(const UserData &);

    //令牌刷新
    static JwtToken::TokenPair updateAccessToken(const UserData &);

    //修改密码
    static void changePassword(Id, const std::string &);

private:
    static JwtToken::TokenPair createAccessToken(const int id_);
};