
## 依赖安装
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
