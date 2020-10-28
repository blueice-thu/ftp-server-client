from utils import *
from threading import Thread
import time

BUFFER_LENGTH = 1024


class Client:
    def __init__(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # self.sock.settimeout(3)
        self.info = {}
        self.mode = 'PASV'

        self.transMode = 'BINARY'

        self.taskList = []
        self.threadList = []

        self.serverPath = '/'
        self.window = None

    def reset(self):
        self.__init__()

    def setWindow(self, window):
        self.window = window

    def setInfo(self, connInfo):
        self.info['host'] = connInfo['host']
        self.info['port'] = connInfo['port']
        self.info['username'] = connInfo['username']
        self.info['password'] = connInfo['password']

    def connect(self):
        try:
            self.sock.connect((self.info['host'], self.info['port']))
        except Exception as err:
            return False, err
        welcomeMsg = self.sock.recv(BUFFER_LENGTH).decode()
        status, msg = welcomeMsg.split(' ', 1)
        if status == '220':
            return True, msg
        return False, msg

    def login(self):
        self.sock.send("USER {}\r\n".format(self.info['username']).encode())
        userMsg = self.sock.recv(BUFFER_LENGTH).decode()
        status, msg = userMsg.split(' ', 1)
        if status == '331':
            self.sock.send("PASS {}\r\n".format(self.info['password']).encode())
            passMsg = self.sock.recv(BUFFER_LENGTH).decode()
            status, msg = passMsg.split(' ', 1)
            if status == '230':
                return True, msg
        return False, msg

    def send_port(self):
        localIP = getLocalIP().replace('.', ',')
        dataPort = getRandomPort()
        controlSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        controlSock.bind(('', dataPort))
        controlSock.listen(1)
        self.sock.send("PORT {},{},{}\r\n".format(localIP, dataPort // 256, dataPort % 256).encode())
        dataTransSock, _ = controlSock.accept()
        if dataTransSock:
            portMsg = self.sock.recv(BUFFER_LENGTH).decode()
            return dataTransSock
        return None

    def send_pasv(self):
        self.sock.send("PASV\r\n".encode())
        pasvMsg = self.sock.recv(BUFFER_LENGTH).decode()
        if '(' not in pasvMsg:
            pasvMsg = self.sock.recv(BUFFER_LENGTH).decode()
        status, msg = pasvMsg.split(' ', 1)

        hostPort = msg.split('(')[1].split(')')[0]
        ip1, ip2, ip3, ip4, port1, port2 = hostPort.split(',')
        pasvHost = "{}.{}.{}.{}".format(ip1, ip2, ip3, ip4)
        pasvPort = 256 * int(port1) + int(port2)
        dataTransSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        if dataTransSock:
            dataTransSock.connect((pasvHost, pasvPort))
            return dataTransSock
        return None

    def _uploadTask(self, filename, dataTransSock):
        fullFileName = getLinuxCwd() + '/' + filename
        task = {
            'Server/Local file': fullFileName,
            'Direction': '-->>',
            'Remote file': self.serverPath + '/' + filename,
            'State': 'uploading'
        }
        self.taskList.append(task)

        filename = filename.split('/')[-1]
        readFile = open(filename, 'rb')
        while True:
            sendData = readFile.read(BUFFER_LENGTH)
            if not sendData:
                break
            dataTransSock.send(sendData)
        readFile.close()
        dataTransSock.close()
        task['State'] = 'finish'
        if self.window:
            self.window.echo('Status', 'Upload "{}" successfully'.format(filename))

    def upload(self, filename):
        dataTransSock = None
        if self.mode == 'PASV':
            dataTransSock = self.send_pasv()
        elif self.mode == 'PORT':
            dataTransSock = self.send_port()
        else:
            exit(1)
        self.sock.send('STOR {}\r\n'.format(filename.split('/')[-1]).encode())
        # self._uploadTask(filename, dataTransSock)
        uploadThread = Thread(target=self._uploadTask, args=[filename, dataTransSock])
        uploadThread.start()
        return True

    def _downloadTask(self, filename, dataTransSock):
        filename = filename.split('/')[-1]
        fullFileName = getLinuxCwd() + '/' + filename
        task = {
            'Server/Local file': fullFileName,
            'Direction': '<<--',
            'Remote file': self.serverPath + '/' + filename,
            'State': 'downloading'
        }
        self.taskList.append(task)
        recvData = dataTransSock.recv(BUFFER_LENGTH)
        with open(fullFileName, 'wb') as recvFile:
            while len(recvData) > 0:
                recvFile.write(recvData)
                recvData = dataTransSock.recv(BUFFER_LENGTH)
        dataTransSock.close()
        task['State'] = 'finish'
        if self.window:
            self.window.echo('Status', 'Download "{}" successfully'.format(filename))

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
        listData = ""
        while True:
            newData = dataTransSock.recv(BUFFER_LENGTH).decode()
            listData += newData
            if not newData:
                break
        # listMsg = self.sock.recv(BUFFER_LENGTH).decode()
        # status, msg = listMsg.split(' ', 1)
        # if not status:
        #     return False
        dataTransSock.close()
        fileInfoList = [i for i in listData.splitlines() if len(i) != 0]
        return fileInfoList

    def send_syst(self):
        self.sock.send("SYST\r\n".encode())
        systMsg = self.sock.recv(BUFFER_LENGTH).decode()
        status, msg = systMsg.split(' ', 1)
        if status:
            return True, msg
        return False, msg

    def deleteFile(self, filename):
        self.sock.send("DELE {}\r\n".format(filename).encode())
        deleteMsg = self.sock.recv(BUFFER_LENGTH).decode()
        status, msg = deleteMsg.split(' ', 1)
        if status:
            return True

    def chooseType(self, typename):
        if typename == 'BINARY':
            self.sock.send("TYPE I\r\n".encode())
        typeMsg = self.sock.recv(BUFFER_LENGTH).decode()
        status, msg = typeMsg.split(' ', 1)
        if status:
            return True
        return False

    def makeDir(self, newFolder):
        self.sock.send("MKD {}\r\n".format(newFolder).encode())
        time.sleep(1)
        mkdMsg = self.sock.recv(BUFFER_LENGTH).decode()
        status, msg = mkdMsg.split(' ', 1)
        if status:
            return True
        return False

    def changeWrokDir(self, targetFoler):
        self.sock.send("CWD {}\r\n".format(targetFoler).encode())
        cwdMsg = self.sock.recv(BUFFER_LENGTH).decode()
        status, msg = cwdMsg.split(' ', 1)
        if status:
            return True
        return False

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
        if status:
            return True
        return False

    def rename(self, oldname, newname):
        self.sock.send("RNFR {}\r\n".format(oldname).encode())
        rnfrMsg = self.sock.recv(BUFFER_LENGTH).decode()
        self.sock.send("RNTO {}\r\n".format(newname).encode())
        status, msg = rnfrMsg.split(' ', 1)
        if status:
            self.sock.send("RNTO {}\r\n".format(newname).encode())
            rntoMsg = self.sock.recv(BUFFER_LENGTH).decode()
            status, msg = rntoMsg.split(' ', 1)
            if status:
                return True
        return False

    def terminateDataTrans(self):
        pass

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
        if status:
            return msg
        return None
