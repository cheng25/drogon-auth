#!/bin/bash
set -e  # Exit on any error
set -u  # Treat unset variables as errors

echo "Installing libs..."

# Create and move into the libs directory
# 判断是否存在libs目录
if [ ! -d "libs" ]; then
  mkdir -p ./libs
else
  echo "libs already exists. Skipping clone." # 已存在则进入libs目录
fi
 cd ./libs || exit 1 # 已存在则进入libs目录

echo "Installing jwt-cpp..."
if [ ! -d "jwt-cpp" ]; then
  git clone --depth 1 https://github.com/Thalhammer/jwt-cpp.git
else
  echo "jwt-cpp already exists. Skipping clone." # 已存在则跳过克隆
fi

echo "Installing bcrypt-cpp..."
if [ ! -d "Bcrypt.cpp" ]; then
  git clone --depth 1 https://github.com/hilch/Bcrypt.cpp.git
  #mv Bcrypt.cpp Bcrypt
else
  echo "Bcrypt already exists. Skipping clone." # 已存在则跳过克隆
fi

echo "Installing nlohmann_json..."
if [ ! -d "nlohmann_json" ]; then
  git clone --depth 1 https://github.com/nlohmann/json.git
  mv json nlohmann_json
else
  echo "nlohmann_json already exists. Skipping clone." # 已存在则跳过克隆
fi

echo "All dependencies installed successfully."
