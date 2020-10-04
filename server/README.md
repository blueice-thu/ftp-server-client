# linux命令

- 查看进程：ps aux
- 查看所有端口：netstat -ant
- 查看特定端口占用：netstat -nltp | grep 12306
- 查看端口占用进程：sudo lsof -i:12306
- 杀死进程：sudo kill -9 10256

make clean; make; sudo ./ftpServer --port 12306 --root ./spb/tmp