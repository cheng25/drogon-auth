

# DrogonAuth

## Описание 描述

Этот проект реализует базовую систему аутентификации и авторизации с использованием JWT (JSON Web Token) на фреймворке Drogon. 本项目基于 Drogon 框架实现了使用 JWT（JSON Web Token）的基础认证与授权系统。Он предоставляет следующие функции: 它提供以下功能：

- 

  Регистрация пользователей 用户注册

- 

  Аутентификация с получением токена 获取令牌进行认证

- 

  Изменение пароля 修改密码

- 

  Обновление токена 更新令牌

- 

  Выход из системы 退出登录

## Зависимости 依赖

Для работы проекта необходимы следующие библиотеки и фреймворки: 项目运行需要以下库和框架：

- 

  **[Drogon](https://github.com/drogonframework/drogon)**: высокопроизводительный фреймворк для разработки веб-приложений на C++. Drogon предлагает быстрый и масштабируемый способ создания RESTful API.

  **[Drogon](https://github.com/drogonframework/drogon)**：用于 C++ Web 应用开发的高性能框架。Drogon 提供了快速、可扩展的方式来创建 RESTful API。

- 

  **[JWT-CPP](https://github.com/arun11299/cpp-jwt)**: библиотека для работы с JWT (JSON Web Tokens) в C++. Эта библиотека используется для создания, верификации и декодирования JWT токенов.

  **[JWT-CPP](https://github.com/arun11299/cpp-jwt)**：用于 C++ 中处理 JWT（JSON Web Tokens）的库。该库用于创建、验证和解码 JWT 令牌。

- 

  **[redis-plus-plus](https://github.com/sewenew/redis-plus-plus)**: C++ клиент для работы с Redis. Используется для хранения сессий и кэширования данных.

  **[redis-plus-plus](https://github.com/sewenew/redis-plus-plus)**：用于操作 Redis 的 C++ 客户端。用于存储会话和缓存数据。

- 

  **[Bcrypt.cpp](https://github.com/hilch/Bcrypt.cpp)**: C++ библиотека для хеширования паролей с использованием алгоритма bcrypt. Эта библиотека применяется для безопасного хранения паролей.

  **[Bcrypt.cpp](https://github.com/hilch/Bcrypt.cpp)**：使用 bcrypt 算法对密码进行哈希处理的 C++ 库。该库用于安全存储密码。

## Примеры API 示例API

### Регистрация 注册

**POST /sign-up**

Тело запроса 请求体:

```json
{
  "username": "example",
  "email": "example@gmail.com",
  "password": "123456789&mM"
}
```

Тело ответа 响应体:

```json
{
  "message": "User registered successfully"
}
```

### Аутентификация 认证

**POST /sign-in**

Тело запроса 请求体:

```json
{
  "username": "example",
  "email": "example@gmail.com",
  "password": "123456789&mM"
}
```

Тело ответа 响应体:

```json
{
  "accessToken": "eyJhbGciOiJIUzI1NiJ9.eyJleHAiOjE3MzY5MDk2MTcsImlhdCI6MTczNjgwMTYxNywiaXNzIjoiQ2FweSIsInN1YiI6IjM2In0.y-2Hv8ES-M9FUyWj8W2iy9yrTSKQfISaKdLnuzV0OMk",
  "message": "The user has successfully logged into the account",
  "userId": 1
}
```

./pic/authRefreshToken.png

### Изменение пароля 修改密码

**POST /changePassword**

Тело запроса 请求体:

```json
{
  "accessToken": "eyJhbGciOiJIUzI1NiJ9.eyJleHAiOjE3MzY5NjEzNzgsImlhdCI6MTczNjg1MzM3OCwiaXNzIjoiQ2FweSIsInN1YiI6IjMifQ.p2OtD-GCZBizt_bHv5IOPKRwcajMxFoaftOWSeOxDRU",
  "password": "New123456789&mM"
}
```

Тело ответа 响应体:

```json
{
  "message": "New password set successfully"
}
```

### Обновление access token 更新访问令牌

**POST /getNewAccessToken**

Тело запроса 请求体:

```json
{
  "accessToken": "eyJhbGciOiJIUzI1NiJ9.eyJleHAiOjE3MzY5Mzg1NzgsImlhdCI6MTczNjgzMDU3OCwiaXNzIjoiQ2FweSIsInN1YiI6IjQwIn0.wxL6djVoY-0uBt1XcaEG3DwPe-vQ1-6yGSgiFyDuaLQ"
}
```

Тело ответа 响应体:

```json
{
  "accessToken": "eyJhbGciOiJIUzI1NiJ9.eyJleHAiOjE3MzY5Mzg1NzgsImlhdCI6MTczNjgzMDU3OCwiaXNzIjoiQ2FweSIsInN1YiI6IjQwIn0.wxL6djVoY-0uBt1XcaEG3DwPe-vQ1-6yGSgiFyDuaLQ",
  "message": "The user has successfully updated the access token",
  "userId": 1
}
```

### Выход из системы 退出登录

**POST /logout**

Тело запроса 请求体:

```json
{
  "username": "example",
  "email": "example@gmail.com",
  "password": "123456789&mM"
}
```

Тело ответа 响应体:

```json
{
  "logout": "ok",
  "message": "The user has successfully logged out of the account."
}
```

---









# DrogonAuth

## Описание

Этот проект реализует базовую систему аутентификации и авторизации с использованием JWT (JSON Web Token) на фреймворке Drogon. Он предоставляет следующие функции:
- Регистрация пользователей
- Аутентификация с получением токена
- Изменение пароля
- Обновление токена
- Выход из системы

## Зависимости

Для работы проекта необходимы следующие библиотеки и фреймворки:

- **[Drogon](https://github.com/drogonframework/drogon)**: высокопроизводительный фреймворк для разработки веб-приложений на C++. Drogon предлагает быстрый и масштабируемый способ создания RESTful API.

- **[JWT-CPP](https://github.com/arun11299/cpp-jwt)**: библиотека для работы с JWT (JSON Web Tokens) в C++. Эта библиотека используется для создания, верификации и декодирования JWT токенов.

- **[redis-plus-plus](https://github.com/sewenew/redis-plus-plus)**: C++ клиент для работы с Redis. Используется для хранения сессий и кэширования данных.

- **[Bcrypt.cpp](https://github.com/hilch/Bcrypt.cpp)**: C++ библиотека для хеширования паролей с использованием алгоритма bcrypt. Эта библиотека применяется для безопасного хранения паролей.

## Примеры API

### Регистрация
**POST /sign-up**  
Тело запроса:
```json
{
  "username": "example",
  "email": "example@gmail.com",
  "password": "123456789&mM"
}
```
Тело ответа:
```json
{
  "message": "User registered successfully"
}
```

### Аутентификация
**POST /sign-in**  
Тело запроса:
```json
{
  "username": "example",
  "email": "example@gmail.com",
  "password": "123456789&mM"
}
```
Тело ответа:
```json
{
  "accessToken": "eyJhbGciOiJIUzI1NiJ9.eyJleHAiOjE3MzY5MDk2MTcsImlhdCI6MTczNjgwMTYxNywiaXNzIjoiQ2FweSIsInN1YiI6IjM2In0.y-2Hv8ES-M9FUyWj8W2iy9yrTSKQfISaKdLnuzV0OMk",
  "message": "The user has successfully logged into the account",
  "userId": 1
}
```
![упс..](./pic/authRefreshToken.png)

### Изменение пароля
**POST /changePassword**  
Тело запроса:
```json
{
  "accessToken": "eyJhbGciOiJIUzI1NiJ9.eyJleHAiOjE3MzY5NjEzNzgsImlhdCI6MTczNjg1MzM3OCwiaXNzIjoiQ2FweSIsInN1YiI6IjMifQ.p2OtD-GCZBizt_bHv5IOPKRwcajMxFoaftOWSeOxDRU",
  "password": "New123456789&mM"
}
```
Тело ответа:
```json
{
  "message": "New password set successfully"
}
```

### Обновление access token
**POST /getNewAccessToken**  
Тело запроса:
```json
{
  "accessToken": "eyJhbGciOiJIUzI1NiJ9.eyJleHAiOjE3MzY5Mzg1NzgsImlhdCI6MTczNjgzMDU3OCwiaXNzIjoiQ2FweSIsInN1YiI6IjQwIn0.wxL6djVoY-0uBt1XcaEG3DwPe-vQ1-6yGSgiFyDuaLQ"
}
```
Тело ответа:
```json
{
  "accessToken": "eyJhbGciOiJIUzI1NiJ9.eyJleHAiOjE3MzY5Mzg1NzgsImlhdCI6MTczNjgzMDU3OCwiaXNzIjoiQ2FweSIsInN1YiI6IjQwIn0.wxL6djVoY-0uBt1XcaEG3DwPe-vQ1-6yGSgiFyDuaLQ",
  "message": "The user has successfully updated the access token",
  "userId": 1
}
```

### Выход из системы
**POST /logout**  
Тело запроса:
```json
{
  "username": "example",
  "email": "example@gmail.com",
  "password": "123456789&mM"
}
```
Тело ответа:
```json
{
  "logout": "ok",
  "message": "The user has successfully logged out of the account."
}
```
