#include "serviceAuth.hpp"
#include "../repositories/userRepos.hpp"
#include "../../libs/Bcrypt.cpp/include/bcrypt.h"
#include "jwt-cpp/jwt.h"
#include "../repositories/sessionRepos.hpp"

void AuthService::registration(const user &user) {
    std::clog << "log AuthService::registration" << std::endl;
    try {
        static UserRepos repos;
        repos.create_user(user);
    } catch (const std::exception &ex) {
        std::clog << "err registration: " << ex.what() << std::endl;
        throw std::runtime_error("Error during user registration");
    }
}

AuthService::UserData AuthService::login(const user &user) {
    std::clog << "log AuthService::login" << std::endl;
    try {
        static UserRepos repos;
        const auto userData = repos.getUserAuthData(user.getUsername(), user.getEmail());
        if (!bcrypt::validatePassword(user.getHashPassword_(),
                                      userData.password)) {
            throw std::runtime_error("Incorrect password");
        }

        const auto jwtTokenPair = createAccessToken(userData.id_);// create token

        repos::Session session(jwtTokenPair);
        session.upload(userData.id_);// save token
        return {jwtTokenPair, userData.id_};
    } catch (...) {
        std::clog << "err login" << std::endl;
        throw;
    }
}

void AuthService::logout(const UserData &userData) {
    std::clog << "log AuthService::logout" << std::endl;
    repos::Session session(userData.TokenPair);
    session.remove(userData.id, userData.TokenPair);
}

JwtToken::TokenPair AuthService::updateAccessToken(const UserData &userData) {
    std::clog << "log AuthService::updateAccessToken" << std::endl;

#if 0
    repos::Session session(userData.TokenPair);
    session.remove(userData.id, userData.TokenPair);
    session.upload(userData.id);
    return session.get(userData.id, userData.TokenPair);
#else
    // Remove old session (best-effort: if not found, continue)
    try {
        const repos::Session oldSession(userData.TokenPair);
        oldSession.remove(userData.id, userData.TokenPair);
    } catch (const std::exception &e) {
        std::clog << "log AuthService::updateAccessToken - remove old session: " << e.what() << std::endl;
        // continue: maybe session already removed/expired
    }

    // Create new token pair and upload new session
    const auto newPair = createAccessToken(userData.id);// create token

    try {
        const repos::Session newSession(newPair);
        newSession.upload(userData.id);
    } catch (const std::exception &e) {
        std::clog << "err updateAccessToken - upload new session: " << e.what() << std::endl;
        throw;
    }

    return newPair;
#endif
}


void AuthService::changePassword(Id id, const std::string &password) {
    std::clog << "log AuthService::changePassword" << std::endl;
    static UserRepos repos;
    repos.updatePassword(id, password);
}

JwtToken::TokenPair AuthService::createAccessToken(const int id_)
{
    const JwtToken jwtToken("secretKey", 1800, 30);
    const auto jwtTokenPair = jwtToken.createPair(id_);// create tok
    return jwtTokenPair;
}
