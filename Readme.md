# FTP Server and Client

## FTP Server

### 运行

仅支持 Linux 平台，需要 sudo 权限。

```bash
make clean
make
sudo ./server
sudo ./server -port 21 -root /tmp
```

### 代码设计

- `main.c` 启动服务器；
- `strutils.c ` 包含了一些读取配置和参数信息的函数；
- `netutils.c ` 具有接收客户端连接、接收客户端命令，并调用相应函数的功能；
- `command_*.c ` 文件具有相应的处理 FTP 命令的功能；
  - `command_access` 处理关于登录的命令：USER, PASS, SYST,QUIT, ABOR
  - `command_file.c` 处理关于文件操作的命令：MKD, CWD, PWD, LIST, RMD, RNFR, RNTO, DELE, CDUP
  - `command_mode.c` 处理关于传输模式的命令：PORT, PASV, TYPE
  - `command_trans.c` 处理文件传输的命令：RETR, STOR, REST
- `common.c` 文件被其他所有文件包含，定义了全局性的数据结构、全局变量，以及许多其他文件所共同需要的工具函数。
  - `Config` 结构体存储全局性的信息，如端口、路径、用户名密码，并定义了对应的全局变量 `config` ;
  - `cmdlist` 包含了支持的 FTP 命令；
  - `Session` 结构体存储了每个连接的信息，如登录状态、传输状态、统计数据等；

代码中有规范且详细的注释，此处不再赘述。

### 脚本测试

需要 Python 2 环境和 ftplib 库。

```bash
sudo python2 autograde.py
```

### 实现的命令

USER, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV, MKD, CWD, PWD, LIST, RMD, RNFR, RNTO, ABOR, DELE, CDUP, REST

### 特色功能

- 传输数据统计，如传输字节数、文件数，并在收到 QUIT 命令时发送给客户端；
- 上传和下载支持断点续传；
- 自动记录日志并附加时间戳，便于观察连接情况；
- 在支持匿名登录的同时，增加账号密码功能，账号密码可直接在 `config.conf` 文件中增删修改；
- 诸多配置自定义，直接在 `config.conf` 中修改；
- 对访问路径进行处理，在支持包含 “../” 路径的同时，验证路径是否合法。

### bugs

- LIST 命令不规范；
- 多线程实现存在问题；
- 断点续传无法完成。

## FTP Client

FTP 客户端基于 Python3 实现，使用 socket 库实现套接字编程部分，使用 PyQt 5 实现界面。

### 运行

```bash
pip install -r requirements.txt
python main.py
```

### 代码设计

- `client.py`文件处理套接字编程，对相关功能进行包装；
- `main.py`文件处理界面逻辑；
- `utils.py`文件编写了一些工具函数；
- `history.ini`文件记录历史连接信息；
- `res`文件夹包含了一些图片资源。

### 实现的命令

USER, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV, MKD, CWD, PWD, LIST, RMD, RNFR, RNTO

### 特色功能

- 将历史连接信息存储在本地 `history.ini` 文件中，可以直接读取、自动填入；

### bugs

- 多线程导致界面程序崩溃，原因暂时未找到。

