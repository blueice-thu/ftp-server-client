# FTP Server

- 编译：`make`
- 运行：`sudo ./ftpServer --port 12306 --root ./spb/tmp`
- 清理：`make clean`
- 整合：`make clean; make; sudo ./ftpServer --port 12306 --root ./spb`

# linux命令

- 查看进程：ps aux
- 查看所有端口：netstat -ant
- 查看特定端口占用：netstat -nltp | grep 12306
- 查看端口占用进程：sudo lsof -i:12306
- 杀死进程：sudo kill -9 10256
- `sudo` start vscode: `sudo code --user-data-dir="~/.vscode-root"`

# linux FTP命令

- `passive`：切换被动模式

# Run test

```python
sudo python2 autograde.py
```