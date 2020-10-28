## FTP Server

编译运行：
```bash
make clean
make
sudo ./server
```

测试脚本：
```python
sudo python2 autograde.py
```

- 编译：`make`
- 运行：`sudo ./server -port 21 -root /tmp`
- 清理：`make clean`
- 整合：`make clean; make; sudo ./server -port 21 -root /tmp`

## linux辅助命令

- 查看进程：ps aux
- 查看所有端口：netstat -ant
- 查看特定端口占用：netstat -nltp | grep 21
- 查看端口占用进程：sudo lsof -i:21
- 杀死进程：sudo kill -9 {pid}
- `sudo` start vscode: `sudo code --user-data-dir="~/.vscode-root"`