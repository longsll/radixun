# radixun

---

## 项目构建

1. 更新/安装gcc编译器
```shell
sudo apt install software-properties-common
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt install g++
sudo apt install gcc-11 g++-11
```
2. cmake
```shell
sudo apt install cmake
```
3. git
```shell
sudo apt-get install git
```
4. boost
```shell
sudo apt install libboost-dev
```
5. yamlcpp

```shell
git clone https://github.com/jbeder/yaml-cpp.git
cd yaml-cpp
mkdir build && cd build
cmake .. && make -j
sudo make install
```
6. mysql

```shell
sudo apt-get install libmysql++-dev
sudo systemctl mysql-server
```

## 模块

- 日志模块
- 环境变量模块
- 配置模块
- 线程模块
- 协程模块
- 协程调度模块
- IO协程调度模块
- 定时器模块
- hook模块
- Address模块
- Socket模块
- ByteArray类
- Stream模块
- TcpServer类
- HTTP模块