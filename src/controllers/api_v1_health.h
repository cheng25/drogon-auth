#pragma once

#include <drogon/HttpController.h>

using namespace drogon;
namespace api
{
namespace v1
{
class health final: public drogon::HttpController<health>
{
  public:
    METHOD_LIST_BEGIN
    METHOD_ADD(health::healthCheck, "/", Get);///api/v1/health/
    METHOD_LIST_END
    static void healthCheck(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
};
}
}