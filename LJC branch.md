
## 依赖安装
```bash
git submodule update --init --recursive
```

或者
```bash
./dependencies.sh
```

## 启动容器
### 没有其他postgres容器在运行
```bash
sudo docker-compose -f /home/l/drogon_dev/drogon-auth/docker-compose.yml up -d postgres
```
- 加入网络
>如果不是使用drogonframework/drogon镜像启动容器开发则不需要加入网络 my-net
> 请确保已创建网络 my-net
> 否则，使用以下命令创建网络：
> `sudo docker network create my-net`

```bash
sudo docker network connect my-net postgres_container_compose_test
```
### 有其他postgres容器在运行
- 登录其他postgres容器
```bash
sudo docker exec -it postgres_prod psql -U postgres -d postgres

```
- 执行sql命令句
```sql
# 查看所有数据库
\list

# 如果没有数据库 postgres 则创建数据库(默认有)
CREATE DATABASE postgres;

# 进入数据库 postgres(默认在)
\c postgres

#执行创建表的sql命令句
# migrations/000_create_schema_migrations.sql
# migrations/001_create_users_table.sql

# 查看所有表
\dt
```

## 健康检查
- 访问健康检查接口
```bash
curl http://localhost:8080/api/v1/health
```

# 项目重要点记录
> [中文完整解释文档](https://zread.ai/capybaracplusplus/drogon-auth)

## 项目结构
```bash
drogon-auth/
├── src/
│   ├── main.cpp                         
│   ├── config.json                       # Drogon 运行时配置(数据库连接、监听端口、线程数)
│   ├── controllers/
│   │   ├── authController.h              # 路由 ↔ 处理函数 ↔ 中间件映射
│   │   └── authController.cpp            # HTTP 处理函数实现（请求解析、响应构建、Cookie 设置）
│   │
│   ├── middlewares/                      # 请求拦截管道
│   │   ├── ValidateRequestBodyMiddleware.hpp # JSON 请求体解析及空值检查
│   │   ├── ValidateEmailAndUsernameMiddleware .hpp # 邮箱正则匹配 + 用户名非空检查
│   │   ├── ValidatePasswordMiddleware .hpp # 密码存在性 + 最小长度（8个字符）检查
│   │   ├── TokenExtractionMiddleware.hpp # 提取 accessToken（请求体）+ refreshToken（Cookie）
│   │   └── ValidateTokensMiddleware.hpp  # JWT 签名验证 + Redis 会话查询
│   │
│   ├── services/
│   │   ├── serviceAuth.hpp               # 认证业务逻辑接口
│   │   └── serviceAuth.cpp               # 注册、登录、注销、令牌刷新
│   │
│   ├── repositories/
│   │   ├── userRepos.hpp / .cpp          # 用户 PostgreSQL CRUD 操作
│   │   └── sessionRepos.hpp / .cpp       # 用于 JWT 令牌对的 Redis 会话存储/检索/删除
│   │
│   ├── models
│   │   └── user.hpp                      # 用户领域模型（username, email, hash）
│   │
│   ├── dto                 
│   │   └── userDtro.hpp                  # 数据传输对象定义
│   │
│   └── utils
│   │   └── jwt
│   │       └──jwtToken.cpp               # JwtToken 类——创建/验证 JWT 令牌对
│   │
├── migrations/
│   ├── 000_create_schema_migrations.sql
│   └── 001_create_users_table.sql        # users 表 (id, username, email, hashpassword)
│   │
├── libs/                                 # 第三方依赖 (jwt-cpp, Bcrypt, redis++, 等)
├── tests
│   └── serviceAuth.cpp                 # 单元测试（当前构建已禁用）
│   │
├── openapi.yaml                          # 所有端点的 OpenAPI 3.0 规范
├── pic/authRefreshToken.png              # 令牌刷新流程图
├── docker-compose.yml                    # 用于开发的 PostgreSQL 容器
└── CMakeLists.txt                        # 构建配置

```

## 架构概览
应用程序遵循 `Controller → Service → Repository` 分层架构。
Drogon 框架通过中间件管道对传入的 HTTP 请求进行验证，然后将其路由到 `authController`，
后者将业务逻辑委托给 `AuthService`。数据持久化分为两部分：PostgreSQL（用户记录）和 
Redis（活跃的会话令牌）。src/config.json 集中管理所有服务的连接参数。

```mermaid
flowchart LR
    Client["HTTP 客户端"] -->|JSON 请求| Drogon["Drogon HTTP 服务器<br/>:3000"]
    
    Drogon -->|验证与路由| Middleware["中间件管道<br/>请求体 -> 邮箱/用户名 -> 密码 -> 令牌"]
    Middleware --> Controller["authController<br/>路由处理程序"]
    Controller --> Service["AuthService<br/>业务逻辑"]
    
    Service --> UserRepo["UserRepos<br/>PostgreSQL"]
    Service --> SessionRepo["Session<br/>Redis"]
    Service --> JWT["JwtToken<br/>签名与验证"]
    Service --> Bcrypt["Bcrypt<br/>哈希与校验"]
    
    UserRepo -->|读写用户| PG[("PostgreSQL<br/>:5432")]
    SessionRepo -->|存储令牌对| RD[("Redis<br/>:6379")]
    
    Drogon -->|JSON 响应<br/>+ refreshCookie| Client
```
---
|方法	|端点	|是否需要鉴权	|请求体字段	|响应要点|
|--|--|--|--|--|
|POST	|/sign-up	|否	|username, email, password |201 — 注册成功确认|
|POST	|/sign-in	|否	|username, email, password	200 — accessToken, userId, refreshToken Cookie|
|POST	|/changePassword	|是（双令牌）	|accessToken, password	200 — 密码更新确认|
|POST	|/getNewAccessToken	|是（双令牌）	|accessToken, refreshToken	200 — 新的 accessToken，新的 refreshToken Cookie|
|POST	|/logout	|是（双令牌）	|accessToken, refreshToken	200 — 注销成功确认|

---

## 分层架构
三层分层架构——控制器、服务、仓储——并配有一个正交的中间件管道，在请求到达任何控制器处理程序之前对其进行拦截。
每一层都有单一且明确的职责，依赖关系严格自上而下流动。
核心原则是控制器绝不直接与数据库通信——它们将所有业务逻辑委托给 `AuthService`，由后者统筹各个仓储。
这种分离意味着认证策略可以独立于 HTTP 接口进行演进。

```mermaid
graph TB
    subgraph "Client"
        C["HTTP Client"]
    end

    subgraph "Drogon Framework"
        subgraph "Middleware Pipeline"
            M1["ValidateRequestBodyMiddleware"]
            M2["ValidateEmailAndUsernameMiddleware"]
            M3["ValidatePasswordMiddleware"]
            M4["TokenExtractionMiddleware"]
            M5["ValidateTokensMiddleware"]
        end
        subgraph "Controller Layer"
            AC["authController"]
        end
    end

    subgraph "Service Layer"
        AS["AuthService"]
    end

    subgraph "Repository Layer"
        UR["UserRepos"]
        SR["Session - Redis"]
    end

    subgraph "Utilities"
        JWT["JwtToken"]
        BC["Bcrypt"]
        U["user / UserDto"]
    end

    subgraph "External Storage"
        PG[("PostgreSQL")]
        RD[("Redis")]
    end

    C --> M1
    M1 --> M2 --> M3 --> AC
    M1 --> M4 --> M5 --> AC
    AC --> AS
    AS --> UR
    AS --> SR
    UR --> PG
    SR --> RD
    AS -.-> JWT
    AC -.-> BC
    AC -.-> U
```

## 端到端请求流（以登录为例）
为了将架构落实到具体场景中，以下是 `/sign-in` 请求的完整旅程：

```mermaid
sequenceDiagram
    participant C as Client
    participant MW as Middleware Pipeline
    participant CTL as authController
    participant SVC as AuthService
    participant UR as UserRepos
    participant SR as Session (Redis)
    participant PG as PostgreSQL

    C->>MW: POST /sign-in {username, password, email}
    MW->>MW: ValidateRequestBodyMiddleware → parse JSON
    MW->>MW: ValidateEmailAndUsernameMiddleware → regex check
    MW->>MW: ValidatePasswordMiddleware → length ≥ 8
    MW->>CTL: Forward (attributes populated)
    CTL->>CTL: Build user → bcrypt::generateHash() → build user entity
    CTL->>SVC: AuthService::login(user)
    SVC->>UR: getUserAuthData(username, email)
    UR->>PG: SELECT Id, hashpassword FROM users WHERE ...
    PG-->>UR: Result row
    UR-->>SVC: UserAuth {id, password_hash}
    SVC->>SVC: bcrypt::validatePassword(input, stored_hash)
    SVC->>SVC: JwtToken::createPair(userId)
    SVC->>SR: Session.upload(userId)
    SR->>SR: SET {userId}:{accessToken} → JSON
    SVC-->>CTL: UserData {TokenPair, id}
    CTL-->>C: 200 OK {accessToken, userId} + Set-Cookie: refreshToken
```

此序列展示了每一层如何恰好贡献一个关注点：中间件验证输入，控制器处理 HTTP 语义（包括对传入密码进行 bcrypt 哈希及构造 Cookie），
服务层执行认证策略，而仓储层则负责在各自的数据存储中进行持久化操作。

## 请求流架构
下图展示了单个请求如何穿越中间件链并到达控制器处理函数。每个中间件既可以调用 nextCb(std::move(mcb)) 将请求转发给下一个中间件，
也可以调用 mcb(resp) 以错误响应直接短路该链路。
```mermaid
flowchart TD
    Client["Client Request"] --> Router["Drogon Router<br/>(compile-time dispatch)"]
    Router --> MW1["Middleware 1<br/>(e.g., ValidateRequestBody)"]
    MW1 -->|nextCb| MW2["Middleware 2<br/>(e.g., ValidateEmail)"]
    MW2 -->|nextCb| MW3["Middleware 3<br/>(e.g., ValidatePassword)"]
    MW3 -->|nextCb| Handler["Controller Handler<br/>(e.g., signUp)"]
    Handler --> Response["HTTP Response"]
    
    MW1 -->|mcb — error| Response
    MW2 -->|mcb — error| Response
    MW3 -->|mcb — error| Response
    
    subgraph "Attribute Bag (req->getAttributes)"
        A1["body: shared_ptr&lt;Json::Value&gt;"]
        A2["accessToken: string"]
        A3["refreshToken: string"]
    end
    
    MW1 -.->|inserts| A1
    MW2 -.->|reads| A1
    MW3 -.->|reads| A1
```
## 架构概览：两大管道家族


```mermaid
flowchart TD
    subgraph "认证管道 /sign-up  /sign-in"
        A1["ValidateRequestBodyMiddleware \n 解析原始请求体 → JSON 属性"]
        A2["ValidateEmailAndUsernameMiddleware \n 检查字段 + 正则邮箱"]
        A3["ValidatePasswordMiddleware \n 检查密码 ≥ 8 个字符"]
        A1 --> A2 --> A3 --> AC["控制器处理函数"]
    end

    subgraph "授权管道/logout /getNewAccessToken /changePassword"
        B1["TokenExtractionMiddleware \n 提取令牌 → 属性"]
        B2["ValidateTokensMiddleware \n JWT 验证 + Redis 交叉检查"]
        B1 --> B2 --> BC["控制器处理函数"]
    end

    subgraph "混合管道 /changePassword"
        C1["ValidateRequestBodyMiddleware"]
        C2["ValidatePasswordMiddleware"]
        C3["TokenExtractionMiddleware"]
        C4["ValidateTokensMiddleware"]
        C1 --> C2 --> C3 --> C4 --> CC["控制器处理函数"]
    end
```
认证管道通过验证请求体的结构和内容来保护基于凭据的端点（`/sign-up`、`/sign-in`）。
授权管道通过提取和验证 JWT 令牌来保护依赖于会话的端点（`/logout`、`/getNewAccessToken`）。
`/changePassword` 路由独特地结合了这两个家族，它需要一个有效的请求体、符合要求的密码以及有效的会话令牌——这种模式将在下文详细探讨。

## 中间件注册与路由绑定
基于路由的绑定意味着**提取中间件充当了网关过滤器**：仅需识别用户身份的路由（如 `/logout`）可以跳过开销较大的 Redis 往返调用，
而修改敏感状态的路由（如 `/changePassword` 或 `/getNewAccessToken`）则会获得完整的密码学 + 会话状态保证。

```mermaid
flowchart TD
    subgraph Route Binding
        R1["POST /logout"]
        R2["POST /getNewAccessToken"]
        R3["POST /changePassword"]
    end

    subgraph Middleware Pipeline
        EX["TokenExtractionMiddleware<br/>Extracts & stores tokens"]
        VAL["ValidateTokensMiddleware<br/>JWT sig + Redis session check"]
        BODY["ValidateRequestBodyMiddleware<br/>& ValidatePasswordMiddleware"]
    end

    R1 --> EX
    R2 --> EX --> VAL
    R3 --> BODY --> EX --> VAL
```

令牌传输策略遵循分通道模型（split-channel model）：访问令牌在 JSON 请求体中传输，而刷新令牌则绑定到登录时设置的`HttpOnly`、`Secure Cookie`。
这种分离是刻意为之——将访问令牌放在请求体而非请求头中，可以使其避开浏览器可访问的存储（避免 `Authorization` 请求头通过 CORS 预检请求泄露），
而 `HttpOnly Cookie` 则能防止基于 JavaScript 的 XSS 攻击窃取刷新令牌。

```mermaid
flowchart TD
    REQ["HTTP Request arrives"]
    PARSE{"Parse JSON body?"}
    PARSE_NO["400: Invalid JSON body"]
    AT{"accessToken<br/>field present?"}
    AT_NO["400: Missing field accessToken"]
    RT{"refreshToken<br/>in cookies?"}
    RT_NO["400: Missing refreshToken in cookies"]
    STORE["Store both tokens<br/>in request attributes"]
    NEXT["nextCb → next middleware"]

    REQ --> PARSE
    PARSE -- No --> PARSE_NO
    PARSE -- Yes --> AT
    AT -- No --> AT_NO
    AT -- Yes --> RT
    RT -- No --> RT_NO
    RT -- Yes --> STORE
    STORE --> NEXT

    style PARSE_NO fill:#f66,color:#fff
    style AT_NO fill:#f66,color:#fff
    style RT_NO fill:#f66,color:#fff
    style STORE fill:#6f6,color:#fff
```


## Redis 键方案
`{userId}:{accessToken}` 意味着系统可以支持每个用户的多个并发会话。每个活跃会话作为单独的 Redis 条目存储，以唯一的访问令牌作为键。
这种设计允许 `/logout` 仅使单个会话失效，而不会影响同一用户账号下的其他活跃会话。

```mermaid
flowchart TD
    ATTR["Retrieve accessToken & refreshToken<br/>from request attributes"]
    VAL_AT{"validateToken(accessToken)?"}
    VAL_RT{"validateToken(refreshToken)?"}
    ERR_RT["401: Refresh token is not valid"]
    ERR_AT["401: Access token is not valid"]
    DECODE["Decode accessToken → userId"]
    REDIS["Redis lookup<br/>key: userId:accessToken"]
    REDIS_FAIL["Redis returns null"]
    COMPARE{"Stored accessToken<br/>== request accessToken?"}
    MATCH_ERR["401: No such access token found"]
    COMPARE_RT{"Stored refreshToken<br/>== request refreshToken?"}
    MATCH_ERR_RT["401: Refresh token does not match stored session"]
    NEXT["nextCb → controller handler"]

    ATTR --> VAL_AT
    VAL_AT -- Valid --> DECODE
    VAL_AT -- Invalid --> VAL_RT
    VAL_RT -- Invalid --> ERR_RT
    VAL_RT -- Valid --> ERR_AT
    DECODE --> REDIS
    REDIS -- Not found --> REDIS_FAIL
    REDIS -- Found --> COMPARE
    COMPARE -- Mismatch --> MATCH_ERR
    COMPARE -- Match --> COMPARE_RT
    COMPARE_RT -- Mismatch --> MATCH_ERR_RT
    COMPARE_RT -- Match --> NEXT

    style ERR_RT fill:#f66,color:#fff
    style ERR_AT fill:#f66,color:#fff
    style REDIS_FAIL fill:#f66,color:#fff
    style MATCH_ERR fill:#f66,color:#fff
    style MATCH_ERR_RT fill:#f66,color:#fff
    style NEXT fill:#6f6,color:#fff
```

## 端到端流程：从登录到验证
下图追踪了从 `/sign-in` 处颁发令牌，到在受保护端点（如 `/getNewAccessToken`）进行提取和验证的完整生命周期：


```mermaid
sequenceDiagram
    participant C as Client
    participant AC as authController
    participant TEX as TokenExtractionMiddleware
    participant VAL as ValidateTokensMiddleware/ValidateRefreshMiddleware
    participant JWT as JwtToken
    participant R as Redis
    participant AS as AuthService

    Note over C,AS: Phase A — Token Issuance (POST /sign-in)
    C->>AC: POST /sign-in {username, email, password}
    AC->>AS: login(user)
    AS->>JWT: createPair(userId)
    JWT-->>AS: {accessToken, refreshToken}
    AS->>R: SET {userId}:{accessToken} → JSON{both tokens}
    AS-->>AC: UserData{TokenPair, id}
    AC-->>C: 200 OK + JSON{accessToken}<br/>Set-Cookie: refreshToken (HttpOnly, Secure)

    Note over C,AS: Phase B — Protected Request (POST /getNewAccessToken)
    C->>TEX: POST /getNewAccessToken {accessToken}<br/>Cookie: refreshToken
    TEX->>TEX: Validate JSON body
    TEX->>TEX: Extract accessToken from body
    TEX->>TEX: Extract refreshToken from cookies
    TEX->>TEX: Store both in request attributes
    TEX->>VAL: nextCb(mcb)

    VAL->>JWT: validateToken(refreshToken)
    alt Refresh token valid
        VAL->>JWT: Decode refreshToken → userId
        VAL->>R: GET {userId}:{accessToken}
        R-->>VAL: JSON{stored tokens}
        VAL->>VAL: Compare stored.refreshToken == request.refreshToken
        VAL->>AC: nextCb(mcb) — tokens verified

        AC->>JWT: Decode refreshToken → userId
        AC->>AS: AuthService::updateAccessToken( AuthService::UserData { JwtToken::TokenPair{accessToken, refreshToken}, userId } )
        AS->>R: DEL {userId}:{oldAccessToken}
        AS->>JWT: ccessToken(userId)
        JWT-->>AS: JwtToken::TokenPair{newAccessToken,newRefreshToken}
        AS->>R: SET {userId}:{newAccessToken} → JSON{tokens}
        AS-->>AC: JwtToken::TokenPair{newAccessToken,newRefreshToken}
        AC-->>C: 200 OK + JSON{newAccessToken}<br/>Set-Cookie: newRefreshToken (HttpOnly, Secure)
    else Refresh token invalid
        VAL-->>C: 401 Unauthorized
    end
```


## 跨层交互
下图说明了 AuthService 如何在控制器层、两个仓库实现以及 bcrypt/JWT 工具库之间进行协调：

```mermaid
flowchart TB
    subgraph Controller["控制器层"]
        AC["authController"]
    end

    subgraph Service["服务层"]
        AS["AuthService<br/><i>静态方法</i>"]
    end

    subgraph Repositories["仓库层"]
        UR["UserRepos<br/><i>PostgreSQL</i>"]
        SR["repos::Session<br/><i>Redis</i>"]
    end

    subgraph Utilities["工具库"]
        BC["bcrypt<br/><i>密码哈希</i>"]
        JWT["JwtToken<br/><i>令牌创建</i>"]
    end

    AC -- "signUp/signIn" --> AS
    AC -- "logout/refresh" --> AS
    AC -- "changePassword" --> AS

    AS -- "registration<br/>login<br/>changePassword" --> UR
    AS -- "login<br/>logout<br/>updateAccessToken" --> SR
    AS -- "login: validatePassword" --> BC
    AS -- "login: createPair" --> JWT

    UR -.-> PG[("PostgreSQL")]
    SR -.-> RD[("Redis")]
```


##  Redis 回话存储
```mermaid
graph TB
    subgraph "Session Repository Layer"
        direction TB
        SessionClass["Session<br/><i>repos::Session</i><br/>upload · get · remove"]
    end

    subgraph "Redis Instance"
        direction TB
        RedisStore[("tcp://127.0.0.1:6379")]
    end

    subgraph "Consumers"
        direction TB
        LoginFlow["AuthService::login"]
        LogoutFlow["AuthService::logout"]
        RefreshFlow["AuthService::updateAccessToken"]
    end

    LoginFlow -->|"session.upload(userId)"| SessionClass
    LogoutFlow -->|"session.remove(userId, tokens)"| SessionClass
    RefreshFlow -->|"remove → upload → get"| SessionClass
    SessionClass -->|"sw::redis::Redis"| RedisStore

    style SessionClass fill:#2d3748,stroke:#e2e8f0,color:#e2e8f0
    style RedisStore fill:#c53030,stroke:#e2e8f0,color:#e2e8f0
```

## 会话生命周
Auth Service 将所有三个操作编排为连贯的生命周期转换。理解确切的调用顺序对于调试与会话相关的故障至关重要。

```mermaid
sequenceDiagram
    participant Client
    participant Controller as authController
    participant Service as AuthService
    participant Session as repos::Session
    participant Redis as Redis (6379)

    Note over Client,Redis: Login — Session Creation
    Client->>Controller: POST /auth/signIn
    Controller->>Service: login(user)
    Service->>Service: bcrypt::validatePassword()
    Service->>Service: jwtToken.createPair(userId)
    Service->>Session: Session(tokenPair)
    Service->>Session: upload(userId)
    Session->>Redis: SET {userId}:{accessToken} → JSON
    Service-->>Controller: UserData{TokenPair, id}

    Note over Client,Redis: Logout — Session Destruction
    Client->>Controller: POST /auth/logout
    Controller->>Service: logout(userData)
    Service->>Session: Session(tokenPair)
    Service->>Session: remove(userId, tokenPair)
    Session->>Redis: DEL {userId}:{accessToken}

    Note over Client,Redis: Token Refresh — Session Rotation
    Client->>Controller: POST /auth/refresh
    Controller->>Service: updateAccessToken(userData)
    Service->>Session: Session(oldTokenPair)
    Service->>Session: remove(userId, oldTokenPair)
    Session->>Redis: DEL {userId}:{oldAccessToken}
    Service->>Session: createAccessToken(userId)
    Service->>Session: upload(newTokenPair)
    Session->>Redis: SET {userId}:{newAccessToken} → JSON
    Service-->>Controller: newTokenPair
```

## JwtToken 类
JWT 子系统跨越三个架构层

```mermaid
graph TB
    subgraph "请求流"
        A[入站请求] --> B[TokenExtractionMiddleware 提取]
        B --> C[ValidateTokensMiddleware 验证]
        C -->|有效| D[控制器处理程序]
        C -->|无效| E[401 响应]
    end

    subgraph "JwtToken 工具"
        F["JwtToken<br/>src/utils/jwt/jwtToken.hpp"]
        F --> G[createPair 创建令牌对]
        F --> H[validateToken 验证令牌]
        G --> I[访问令牌<br/>HS256 · 1800 分钟]
        G --> J[刷新令牌<br/>HS256 · 30 天]
    end

    subgraph "持久化"
        K[会话仓库<br/>Redis]
    end

    D --> F
    C --> H
    C --> K
    D --> K
```

##  令牌验证中间件管道
验证是一个应用于受保护端点的两阶段中间件管道。

```mermaid
sequenceDiagram
    participant Client
    participant TE as TokenExtractionMiddleware
    participant VT as ValidateTokensMiddleware
    participant Redis as Redis
    participant Ctrl as Controller

    Client->>TE: POST /logout (body + cookies)
    TE->>TE: 从 JSON body 中提取 accessToken
    TE->>TE: 从 Cookie 中提取 refreshToken
    TE->>TE: 将两者存储在请求属性中
    alt 缺失或格式错误的令牌
        TE-->>Client: 400 Bad Request
    end
    TE->>VT: nextCb(mcb)

    VT->>VT: validateToken(accessToken)
    alt 访问令牌无效
        VT->>VT: validateToken(refreshToken)
        alt 刷新令牌同样无效
            VT-->>Client: 401 Unauthorized
        else 刷新令牌有效
            VT-->>Client: 401 (访问令牌无效)
        end
    end

    VT->>VT: 解码 sub 声明 → userId
    VT->>Redis: GET {userId}:{accessToken}
    alt 令牌不在 Redis 中
        VT-->>Client: 401 Unauthorized
    end

    VT->>VT: 对比存储的令牌与请求中的令牌
    alt 不匹配
        VT-->>Client: 401 Unauthorized
    end

    VT->>Ctrl: nextCb(mcb)
```



## 刷新端点管道/getNewAccessToken


```mermaid
sequenceDiagram
    participant Client
    participant TE as TokenExtractionMiddleware
    participant VT as ValidateRefreshMiddleware
    participant AC as authController
    participant AS as AuthService
    participant Sess as Session (Redis)

    Client->>TE: POST /getNewAccessToken<br/>{accessToken: "..."}, Cookie: refreshToken=...
    TE->>TE: Extract accessToken from JSON body
    TE->>TE: Extract refreshToken from cookies
    TE->>TE: Store both in request attributes
    TE->>VT: nextCb()
    
    VT->>VT: validateToken(refreshToken)
    alt refreshToken is valid
        VT->>VT: Decode refreshToken → userId
        VT->>Sess: get(userId, tokenPair)
        Sess-->>VT: stored tokenPair
        VT->>VT: Compare stored.refreshToken == request.refreshToken
        alt Match
            VT->>AC: nextCb()
        else Mismatch
            VT-->>Client: 401 "No such access token found"
        end
    end

    AC->>AC: Decode refreshToken → userId
    AC->>AS: updateAccessToken(userData)
    AS->>Sess: remove(userId, oldTokenPair)
    AS->>Sess: upload(userId) [new tokenPair]
    AS->>Sess: get(userId, newTokenPair)
    Sess-->>AS: new tokenPair
    AS-->>AC: new TokenPair
    AC-->>Client: 200 {accessToken: "...", userId: N}<br/>Set-Cookie: refreshToken=...
```