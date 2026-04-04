#pragma once

#include <jwt-cpp/jwt.h>
#include <string>
#include <chrono>

class JwtToken {
public:
    struct TokenPair {
        std::string accessToken;
        std::string refreshToken;
    };
    /** @brief JwtToken constructor
     * @param secretKey: The secret key used to sign the JWT    令牌创建与验证之间共享的 HMAC-SHA256 签名密钥
     * @param accessTokenLifetimeMinutes: The lifetime of the access token in minutes 访问令牌过期时间窗口
     * @param refreshTokenLifetimeDays: The lifetime of the refresh token in days 刷新令牌过期时间窗口
     */
    explicit JwtToken(const std::string &secretKey = "secretKey",
             const size_t accessTokenLifetimeMinutes = 1800,
             const size_t refreshTokenLifetimeDays = 30)
            : secretKey_(secretKey),
              accessTokenLifetime_(std::chrono::minutes(accessTokenLifetimeMinutes)),
              refreshTokenLifetime_(std::chrono::days(refreshTokenLifetimeDays)) {}

    [[nodiscard]] TokenPair createPair(const size_t &userId) const {
        return createPair(std::to_string(userId));
    }

    [[nodiscard]] TokenPair createPair(const std::string &userId) const
    {
        const auto now = std::chrono::system_clock::now();
        // Create access token 创建访问令牌
        const std::string accessToken = jwt::create()
                .set_issuer("Capy") // 令牌颁发者
                .set_subject(userId) // 令牌主题
                .set_issued_at(now) // 令牌创建时间
                .set_expires_at(now + accessTokenLifetime_)// 令牌过期时间
                .sign(jwt::algorithm::hs256{secretKey_});// 签名
        // Create refresh token 创建刷新令牌
        const std::string refreshToken = jwt::create()
                .set_issuer("Capy")
                .set_subject(userId)
                .set_issued_at(now)
                .set_expires_at(now + refreshTokenLifetime_)
                .sign(jwt::algorithm::hs256{secretKey_});

        return {accessToken, refreshToken};
    }

    [[nodiscard]] bool validateToken(const std::string &token) const {
        try {
            const auto decodedToken = jwt::decode(token);

            jwt::verify()
                    .allow_algorithm(jwt::algorithm::hs256{secretKey_})
                    .with_issuer("Capy")
                    .verify(decodedToken);
            return true;
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return false;
        }
    }

private:
    std::string secretKey_;
    std::chrono::system_clock::duration accessTokenLifetime_;
    std::chrono::system_clock::duration refreshTokenLifetime_;
};