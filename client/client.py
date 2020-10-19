import socket, os, time, re
from random import randint
from threading import Thread

BUFFER_LENGTH = 1024


def getLocalIP():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.connect(('8.8.8.8', 80))
        ip = sock.getsockname()[0]
    finally:
        sock.close()
    return ip


def getRandomPort():
    return randint(5001, 65535)


class Client:
    def __init__(self, info):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(3)
        self.info = info
        self.mode = 'PASV'

        self.transMode = 'BINARY'

        self.taskList = []
        self.threadList = []

        self.serverPath = '/'

    def connect(self):
        print(self.info)
        try:
            self.sock.connect((self.info['host'], self.info['port']))
        except Exception as err:
            return False, err
        welcomeMsg = self.sock.recv(BUFFER_LENGTH).decode()
        status, msg = welcomeMsg.split(' ', 1)
        print(welcomeMsg)
        if status == '220':
            return True, msg
        return False, msg

    def login(self):
        self.sock.send("USER {}\r\n".format(self.info['username']).encode())
        userMsg = self.sock.recv(BUFFER_LENGTH).decode()
        status, msg = userMsg.split(' ', 1)
        print(userMsg)
        if status == '331':
            self.sock.send("PASS {}\r\n".format(self.info['password']).encode())
            passMsg = self.sock.recv(BUFFER_LENGTH).decode()
            status, msg = passMsg.split(' ', 1)
            print(passMsg)
            if status == '230':
                return True, msg
        return False, msg

    def send_port(self):
        localIP = getLocalIP().replace('.', ',')
        dataPort = getRandomPort()
        print('dataPort = ', dataPort)
        # dataPort = serverPort + 1
        controlSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        controlSock.bind(('', dataPort))
        controlSock.listen(1)
        self.sock.send("PORT {},{},{}\r\n".format(localIP, dataPort // 256, dataPort % 256).encode())
        dataTransSock, _ = controlSock.accept()
        print('dataTransSock:', dataTransSock)
        portMsg = self.sock.recv(BUFFER_LENGTH).decode()
        print('portMsg:', portMsg)
        # print(getLocalIP(), dataPort)
        # status, msg = portMsg.split(' ', 1)
        return dataTransSock

    def send_pasv(self):
        self.sock.send("PASV\r\n".encode())
        pasvMsg = self.sock.recv(BUFFER_LENGTH).decode()
        if '(' not in pasvMsg:
            pasvMsg = self.sock.recv(BUFFER_LENGTH).decode()
        status, msg = pasvMsg.split(' ', 1)
        print(pasvMsg)

        hostPort = msg.split('(')[1].split(')')[0]
        ip1, ip2, ip3, ip4, port1, port2 = hostPort.split(',')
        pasvHost = "{}.{}.{}.{}".format(ip1, ip2, ip3, ip4)
        pasvPort = 256 * int(port1) + int(port2)
        dataTransSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        dataTransSock.connect((pasvHost, pasvPort))
        return dataTransSock
        # return None

    def _uploadTask(self, filename, dataTransSock):
        fullFileName = os.getcwd() + '\\' + filename
        task = {
            'Server/Local file': fullFileName,
            'Direction': '-->>',
            'Remote file': self.serverPath + '/' + filename,
            'State': 'uploading'
        }
        self.taskList.append(task)
        print(dataTransSock)
        dataTransSock.settimeout(3)

        # fileSize = os.path.getsize(fullFileName)
        filename = filename.split('\\')[-1]
        readFile = open(filename, 'rb')
        while True:
            sendData = readFile.read(BUFFER_LENGTH * 10)
            if not sendData:
                break
            dataTransSock.send(sendData)
        readFile.close()
        dataTransSock.close()
        task['State'] = 'finish'

    def upload(self, filename):
        dataTransSock = None
        if self.mode == 'PASV':
            dataTransSock = self.send_pasv()
        elif self.mode == 'PORT':
            dataTransSock = self.send_port()
        else:
            exit(1)
        self.sock.send('STOR {}\r\n'.format(filename).encode())
        self._uploadTask(filename, dataTransSock)
        # uploadThread = Thread(target=self._uploadTask, args=[filename, dataTransSock])
        # uploadThread.start()
        return True

    def _downloadTask(self, filename, dataTransSock):
        filename = filename.split('/')[-1]
        fullFileName = os.getcwd() + '\\' + filename
        task = {
            'Server/Local file': fullFileName,
            'Direction': '<<--',
            'Remote file': self.serverPath + '/' + filename,
            'State': 'downloading'
        }
        self.taskList.append(task)
        print(dataTransSock)
        dataTransSock.settimeout(3)
        recvData = dataTransSock.recv(BUFFER_LENGTH)
        with open(fullFileName, 'wb') as recvFile:
            while len(recvData) > 0:
                recvFile.write(recvData)
                recvData = dataTransSock.recv(BUFFER_LENGTH)
        dataTransSock.close()
        task['State'] = 'finish'

    def download(self, filename):
        dataTransSock = None
        if self.mode == 'PASV':
            dataTransSock = self.send_pasv()
        elif self.mode == 'PORT':
            dataTransSock = self.send_port()
        else:
            exit(1)
        self.sock.send('RETR {}\r\n'.format(filename).encode())
        self._downloadTask(filename, dataTransSock)
        # downloadThread = Thread(target=self._downloadTask, args=[filename, dataTransSock])
        # downloadThread.start()
        return True

    def listFiles(self):
        dataTransSock = None
        if self.mode == 'PASV':
            dataTransSock = self.send_pasv()
        elif self.mode == 'PORT':
            dataTransSock = self.send_port()
        else:
            exit(1)
        self.sock.send("LIST\r\n".encode())
        listData = dataTransSock.recv(BUFFER_LENGTH).decode()
        # print(listData)
        # time.sleep(1)
        listMsg = self.sock.recv(BUFFER_LENGTH).decode()
        print(listMsg)
        dataTransSock.close()
        if len(listData) == 0:
            return False, listData
        fileInfoList = [i for i in listData.splitlines() if len(i) != 0]
        return True, fileInfoList, listMsg

    def send_syst(self):
        self.sock.send("SYST\r\n".encode())
        systMsg = self.sock.recv(BUFFER_LENGTH).decode()
        print(systMsg)
        status, msg = systMsg.split(' ', 1)
        if status == '215':
            return True, msg
        return False, msg

    def chooseType(self, typename):
        if typename == 'BINARY':
            self.sock.send("TYPE I\r\n".encode())
        elif typename == 'ASCII':
            # TODO
            pass
        typeMsg = self.sock.recv(BUFFER_LENGTH).decode()
        status, msg = typeMsg.split(' ', 1)
        print(typeMsg)
        if status == '200':
            return True, msg
        return False, msg

    def makeDir(self, newFolder):
        self.sock.send("MKD {}\r\n".format(newFolder).encode())
        time.sleep(1)
        mkdMsg = self.sock.recv(BUFFER_LENGTH).decode()
        # status, msg = mkdMsg.split(' ', 1)
        print('mkdMsg:', mkdMsg)
        if '257' in mkdMsg:
            return True
        return False

    def changeWrokDir(self, targetFoler):
        self.sock.send("CWD {}\r\n".format(targetFoler).encode())
        cwdMsg = self.sock.recv(BUFFER_LENGTH).decode()
        status, msg = cwdMsg.split(' ', 1)
        print(cwdMsg)
        return True
        # if '250' in cwdMsg:
        #     return True
        # return False

    def printWorkDir(self):
        self.sock.send("PWD\r\n".encode())
        pwdMsg = self.sock.recv(BUFFER_LENGTH).decode()
        currentPath = re.findall(r'[\s\S]*?257 "([\S\s]+?)"[\S\s]*', pwdMsg)
        if len(currentPath) == 0:
            pwdMsg = self.sock.recv(BUFFER_LENGTH).decode()
            currentPath = re.findall(r'[\s\S]*?257 "([\S\s]+?)"[\S\s]*', pwdMsg)
        return currentPath[0]

    def removeDir(self, targetFoler):
        self.sock.send("RMD {}\r\n".format(targetFoler).encode())
        rmdMsg = self.sock.recv(BUFFER_LENGTH).decode()
        status, msg = rmdMsg.split(' ', 1)
        return rmdMsg

    def rename(self, oldname, newname):
        self.sock.send("RNFR {}\r\n".format(oldname).encode())
        rnfrMsg = self.sock.recv(BUFFER_LENGTH).decode()
        self.sock.send("RNTO {}\r\n".format(newname).encode())
        print(rnfrMsg)
        status, msg = rnfrMsg.split(' ', 1)
        if status == '350':
            self.sock.send("RNTO {}\r\n".format(newname).encode())
            rntoMsg = self.sock.recv(BUFFER_LENGTH).decode()
            print(rntoMsg)
            status, msg = rntoMsg.split(' ', 1)
            if status == '250':
                return True, msg
        return False, msg

    def isTasksFinished(self):
        for task in self.taskList:
            if task['State'] != 'finish':
                return False
        return True

    def quit(self, wait=False):
        if not self.isTasksFinished() and wait:
            for th in self.threadList:
                th.join()
        self.sock.send("QUIT\r\n".encode())
        quitMsg = self.sock.recv(BUFFER_LENGTH).decode()
        status, msg = quitMsg.split(' ', 1)
        self.sock.close()
        print(quitMsg)
        if status == '221':
            return True, msg
        return False, msg


if __name__ == '__main__':
    info = {
        'host': '59.66.136.21',
        'username': 'ssast',
        'password': 'ssast',
        'port': 21
    }
    client = Client(info)
    client.connect()
    print('------------------------------------')
    client.login()
    # print('------------------------------------')
    # # client.send_syst()
    # print('------------------------------------')
    # # client.chooseType('BINARY')
    # print('------------------------------------')
    # client.changeWrokDir("spb")
    # print('------------------------------------')
    client.listFiles()
    # print('------------------------------------')
    # client.upload('test1.py')
    # print('------------------------------------')
    # client.listFiles()
    # print('------------------------------------')
    # client.send_port()
    # client.download('test2.py')
    # client.makeDir("spbtest")
    # client.rename("spbtest", "spbtest2")
    # client.removeDir("spbtest2")
    # client.printWorkDir()
    print("All tasks finished: ", client.isTasksFinished())
    client.quit(True)
