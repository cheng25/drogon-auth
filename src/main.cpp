#include <iostream>
#include "drogon/drogon.h"
#include "middlewares/TokenExtractionMiddleware.hpp"
#include "middlewares/ValidateEmailAndUsernameMiddleware.hpp"
#include "middlewares/ValidatePasswordMiddleware.hpp"
#include "middlewares/ValidateRequestBodyMiddleware.hpp"
#include "middlewares/ValidateTokensMiddleware.hpp"
#include "controllers/authController.h"

namespace repos {
    //sw::redis::Redis redisDbClient = sw::redis::Redis("tcp://127.0.0.1:6379");
    sw::redis::Redis redisDbClient = sw::redis::Redis("tcp://host.docker.internal:6379");
}

int main() {
    std::clog << "log start" << std::endl;
    std::clog << "Drogon version: " << drogon::getVersion() << std::endl;

    //drogon::app().loadConfigFile("../src/config.json");
    drogon::app().loadConfigFile("./config.json");
    std::clog << "log loadConfigFile" << std::endl;
    // 输出Redis连接状态
    auto result = repos::redisDbClient.ping();
    std::clog << "log ping result: " << result << std::endl;
    if (result != "PONG") {
        std::clog << "log Redis connection failed" << std::endl;
    }else
    {
        repos::redisDbClient.set("drogon_version", drogon::getVersion());
    }



    drogon::app().run();

    return 0;
}
