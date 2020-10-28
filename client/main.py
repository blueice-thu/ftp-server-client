import re
import sys
from threading import Thread
from functools import partial

from PyQt5.QtCore import Qt, QDir, QSize, QThread
from PyQt5.QtGui import QIcon, QFont, QTextCursor
from PyQt5.QtWidgets import (QWidget, QMainWindow, QAction, qApp, QLineEdit, QLabel, QTextEdit, QListWidgetItem,
                             QToolButton, QMenu, QSizePolicy, QPushButton, QApplication, QScrollBar,
                             QDesktopWidget, QInputDialog, QFileSystemModel, QListWidget, QSplitter, QHBoxLayout,
                             QVBoxLayout, QTreeView)

from client import Client
from utils import *

client = Client()


def _getDevideLine():
    devideLine = QPushButton()
    devideLine.setMaximumWidth(1)
    devideLine.setFocusPolicy(Qt.NoFocus)
    return devideLine


class InputRowWidget(QWidget):
    def __init__(self, parent=None):
        super(InputRowWidget, self).__init__(parent)
        self.hBox = QHBoxLayout()

        self.hostTextEdit = QLineEdit()
        self.usernameTextEdit = QLineEdit()
        self.passwordTextEdit = QLineEdit()
        self.portTextEdit = QLineEdit()
        self.connButton = QPushButton('Quickconnect')
        self.moreButton = QToolButton()

        self.hostTextEdit.setFixedWidth(120)
        self.usernameTextEdit.setFixedWidth(120)
        self.passwordTextEdit.setFixedWidth(120)
        self.passwordTextEdit.setEchoMode(QLineEdit.Password)
        self.portTextEdit.setMaxLength(5)
        self.portTextEdit.setFixedWidth(80)

        self.moreButton.setArrowType(Qt.DownArrow)
        self.moreButton.setPopupMode(QToolButton.InstantPopup)

        self.hBox.addWidget(QLabel('Host:', self))
        self.hBox.addWidget(self.hostTextEdit)
        self.hBox.addWidget(QLabel('Username:', self))
        self.hBox.addWidget(self.usernameTextEdit)
        self.hBox.addWidget(QLabel('Password:', self))
        self.hBox.addWidget(self.passwordTextEdit)
        self.hBox.addWidget(QLabel('Port:', self))
        self.hBox.addWidget(self.portTextEdit)
        self.hBox.addWidget(self.connButton)
        self.hBox.addWidget(self.moreButton)
        self.hBox.addStretch(1)

        self.setLayout(self.hBox)
        self.setContentsMargins(0, 0, 0, 0)


class LocalArea(QWidget):
    def __init__(self, parent=None):
        super(LocalArea, self).__init__(parent)

        self.localPathLabel = QLabel(getLinuxCwd())
        self.localFileLable = QLabel('')
        self.localModel = QFileSystemModel()
        self.localModel.setRootPath(QDir.rootPath())

        self.localPathLabel.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.localFileLable.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        localArea = QVBoxLayout()
        sitePath = QHBoxLayout()
        sitePath.addWidget(QLabel('<b>Local site</b>:'))
        sitePath.addWidget(self.localPathLabel, stretch=1)
        sitePath.addStretch(1)
        siteFile = QHBoxLayout()
        siteFile.addWidget(QLabel('<b>Selected file</b>:'))
        siteFile.addWidget(self.localFileLable, stretch=1)
        siteFile.addStretch(1)

        treeView = QTreeView()
        treeView.setModel(self.localModel)
        for pos, width in enumerate((280, 100, 100, 70)):
            treeView.setColumnWidth(pos, width)
        treeView.clicked.connect(self._localTreeViewClicked)

        localPath = self.localPathLabel.text()
        for i in range(len(localPath)):
            if localPath[i] == '/':
                treeView.expand(self.localModel.index(localPath[:i]))
        treeView.expand(self.localModel.index(localPath))

        buttonBox = QHBoxLayout()
        self.uploadButton = QPushButton('Upload')
        buttonBox.addStretch(1)
        buttonBox.addWidget(self.uploadButton)

        localArea.addLayout(sitePath)
        localArea.addLayout(siteFile)
        localArea.addWidget(treeView)
        localArea.addLayout(buttonBox)
        self.setLayout(localArea)

    def _localTreeViewClicked(self, indexItem):
        localPathOrFile = self.localModel.filePath(indexItem)
        if self.localModel.isDir(indexItem):
            self.localPathLabel.setText(localPathOrFile)
        else:
            self.localFileLable.setText(localPathOrFile)
            localPathOrFile = re.findall(r'([\s\S]+)/[\s\S]+?', localPathOrFile)[0]
            self.localPathLabel.setText(localPathOrFile)
        os.chdir(localPathOrFile)


class ServerArea(QWidget):
    def __init__(self, parent=None):
        super(ServerArea, self).__init__(parent)
        self.serverPathLabel = QLabel('/')
        self.serverFileLable = QLabel('')

        self.serverPathLabel.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.serverFileLable.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        serverArea = QVBoxLayout()

        sitePath = QHBoxLayout()
        sitePath.addWidget(QLabel('<b>Remote site</b>:'))
        sitePath.addWidget(self.serverPathLabel, stretch=1)
        sitePath.addStretch(1)
        siteFile = QHBoxLayout()
        siteFile.addWidget(QLabel('<b>Selected file</b>:'))
        siteFile.addWidget(self.serverFileLable, stretch=1)
        siteFile.addStretch(1)

        self.fileListWidget = QListWidget()
        self.fileListWidget.setModelColumn(4)

        buttonBox = QHBoxLayout()
        self.downloadButton = QPushButton('Download')
        self.mkdirButton = QPushButton('New Folder')
        self.refreshButton = QPushButton('Refresh')
        self.rmdirButton = QPushButton('Remove folder')
        self.renameButton = QPushButton('Rename')
        buttonBox.addStretch(1)
        buttonBox.addWidget(self.downloadButton)
        buttonBox.addWidget(self.mkdirButton)
        buttonBox.addWidget(self.refreshButton)
        buttonBox.addWidget(self.rmdirButton)
        buttonBox.addWidget(self.renameButton)

        serverArea.addLayout(sitePath)
        serverArea.addLayout(siteFile)
        serverArea.addWidget(self.fileListWidget)
        serverArea.addLayout(buttonBox)

        self.setLayout(serverArea)

    def setFileList(self, fileList):
        self.fileListWidget.blockSignals(True)
        self.fileListWidget.clear()
        self.fileListWidget.blockSignals(False)
        for file in fileList:
            item, widget = self._getFileListItem(file['name'], file['isDir'], file['size'], file['time'])
            self.fileListWidget.addItem(item)
            self.fileListWidget.setItemWidget(item, widget)

    @staticmethod
    def _getFileListItem(name, isDir, size, modifyTime):
        widget = QWidget()
        layout = QHBoxLayout()

        nameLabel = QLabel(name)
        nameLabel.setObjectName('name')
        layout.addWidget(nameLabel, stretch=1)
        layout.addWidget(_getDevideLine())

        isDirLabel = QLabel(isDir)
        isDirLabel.setObjectName('isDir')
        layout.addWidget(isDirLabel, stretch=1)
        layout.addWidget(_getDevideLine())

        sizeLabel = QLabel(size)
        sizeLabel.setObjectName('size')
        layout.addWidget(sizeLabel, stretch=1)
        layout.addWidget(_getDevideLine())

        modifyTimeLabel = QLabel(modifyTime)
        modifyTimeLabel.setObjectName('modifyTime')
        layout.addWidget(modifyTimeLabel, stretch=1)

        widget.setLayout(layout)

        item = QListWidgetItem()
        item.setSizeHint(QSize(0, 43))

        return item, widget


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.inputRow = InputRowWidget(self)
        self.localArea = LocalArea(self)
        self.serverArea = ServerArea(self)

        self.logEditText = QTextEdit()

        self.serverFileList = []

        self.isConnect = False

        self.initUI()

    def saveHistory(self):
        with open("history.ini", "w+") as fp:
            fp.write("{},{},{},{}\n".format(
                self.inputRow.hostTextEdit.text(),
                self.inputRow.usernameTextEdit.text(),
                self.inputRow.passwordTextEdit.text(),
                self.inputRow.portTextEdit.text(),
            ))

    @staticmethod
    def readHistory():
        records = []
        with open("history.ini", "r") as fp:
            record = fp.readline().strip()
            while record:
                record = record.split(',')
                if len(record) != 4:
                    continue
                records.append(record)
                record = fp.readline()
        return records

    @staticmethod
    def clearHistory():
        with open("history.ini", "w"):
            pass

    def loadHistory(self, connInfo):
        self.inputRow.hostTextEdit.setText(connInfo[0])
        self.inputRow.usernameTextEdit.setText(connInfo[1])
        self.inputRow.passwordTextEdit.setText(connInfo[2])
        self.inputRow.portTextEdit.setText(connInfo[3])

    def clearInput(self):
        self.inputRow.hostTextEdit.clear()
        self.inputRow.usernameTextEdit.clear()
        self.inputRow.passwordTextEdit.clear()
        self.inputRow.portTextEdit.clear()

    @staticmethod
    def _setClientMode(mode):
        if mode != 'PORT' and mode != 'PASV':
            return
        client.mode = mode

    def initToolbar(self):
        toolbar = self.addToolBar('FTP client')
        toolbar.setMovable(False)

        refreshAction = QAction(QIcon('res/refresh.png'), '&Refresh', self)
        refreshAction.setStatusTip('Refresh')
        refreshAction.triggered.connect(self._refreshServerFileList)
        toolbar.addAction(refreshAction)

        portAction = QAction(QIcon('res/port.png'), '&Port', self)
        portAction.setStatusTip('Active mode')
        portAction.triggered.connect(partial(self._setClientMode, 'PORT'))
        toolbar.addAction(portAction)

        pasvAction = QAction(QIcon('res/pasv.png'), '&Pasv', self)
        pasvAction.setStatusTip('Passive mode')
        pasvAction.triggered.connect(partial(self._setClientMode, 'PASV'))
        toolbar.addAction(pasvAction)

        systAction = QAction(QIcon('res/system.png'), '&System', self)
        systAction.setStatusTip('System info')
        systAction.triggered.connect(lambda: Thread(target=self.clientSyst).start())
        toolbar.addAction(systAction)

        disconectAction = QAction(QIcon('res/disconnect.png'), '&Disconnect', self)
        disconectAction.setStatusTip('Disconnect')
        disconectAction.triggered.connect(lambda: Thread(target=self._threadClientDisconnect).start())
        toolbar.addAction(disconectAction)

        exitAction = QAction(QIcon('res/exit.png'), '&Exit', self)
        exitAction.setStatusTip('Exit')
        exitAction.triggered.connect(qApp.quit)
        toolbar.addAction(exitAction)

    def _moveCenter(self):
        qr = self.frameGeometry()
        cp = QDesktopWidget().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())

    def initInputRow(self):
        menu = QMenu()
        clearInputAction = QAction('Clear input', menu)
        clearInputAction.triggered.connect(self.clearInput)
        clearHistoryAction = QAction('Clear history', menu)
        clearHistoryAction.triggered.connect(self.clearHistory)
        menu.addActions([clearInputAction, clearHistoryAction])
        recordActions = []
        records = self.readHistory()
        for record in records:
            recordAction = QAction('{}@{}'.format(record[1], record[0]), menu)
            recordAction.triggered.connect(partial(self.loadHistory, record))
            recordActions.append(recordAction)
        menu.addSeparator()
        menu.addActions(recordActions)

        self.inputRow.moreButton.setMenu(menu)
        self.inputRow.setMaximumHeight(50)

    def initLogArea(self):
        self.logEditText.setAlignment(Qt.AlignLeft | Qt.AlignTop)
        self.logEditText.setStyleSheet("font-size: 14; font-family: Segoe UI; padding: 10px")
        self.logEditText.setMinimumHeight(120)
        self.logEditText.setText("Log output:")
        self.logEditText.setReadOnly(True)
        self.logEditText.setMaximumHeight(200)
        self.logEditText.ensureCursorVisible()
        scrollBar = QScrollBar()
        self.logEditText.setVerticalScrollBar(scrollBar)
        return self.logEditText

    def echo(self, title, content):
        self.logEditText.append(str(title) + ': ' + str(content))
        cursor = self.logEditText.textCursor()
        cursor.movePosition(QTextCursor.End, QTextCursor.MoveAnchor)
        self.logEditText.setTextCursor(cursor)

    def threadClientConnect(self):
        status, msg = client.connect()
        if not status:
            self.echo('Status', msg)
            return
        self.echo('Status', 'Connection established, waiting for welcome message...')
        self.echo('Status', msg)
        self.threadClientLogin()

    def threadClientLogin(self):
        status, msg = client.login()
        if not status:
            self.echo('Wrong', 'Login failed!')
            return
        self.isConnect = True
        self.echo('Status', 'Logged in')
        status, msg = client.send_syst()
        self.echo('Status', msg)
        self._refreshServerFileList()

    def connectServer(self):
        if self.isConnect:
            try:
                client.quit()
            except Exception as err:
                self.echo('Error', err)

        if not ipPattern.fullmatch(self.inputRow.hostTextEdit.text()):
            self.echo('Wrong', 'Invaild host!')
            return
        if not self.inputRow.portTextEdit.text().isdigit():
            self.echo('Wrong', 'Invaild port!')
            return
        client.setInfo({
            'host': self.inputRow.hostTextEdit.text(),
            'username': self.inputRow.usernameTextEdit.text(),
            'password': self.inputRow.passwordTextEdit.text(),
            'port': int(self.inputRow.portTextEdit.text())
        })
        # Thread(target=self.threadClientConnect).start()
        self.threadClientConnect()

    def _refreshServerFileList(self):
        if not self.isConnect:
            return
        self.serverFileList = [{
            'name': 'Name',
            'isDir': 'Type',
            'size': 'Size',
            'time': 'Modify time'
        }]
        if self.serverArea.serverPathLabel.text() != '/':
            self.serverFileList.append({
                'name': '..',
                'isDir': 'Directory',
                'size': '',
                'time': ''
            })
        self.echo('Status', 'Retrieving directory listing of "{}"...'.format(self.serverArea.serverPathLabel.text()))
        fileInfoList = client.listFiles()
        # if not status:
        #     self.logOut('Wrong', 'Retrieve directory listing failed')
        #     return
        self.echo('Status', 'Directory listing of "{}" successful'.format(self.serverArea.serverPathLabel.text()))
        for fil in fileInfoList:
            try:
                findResult = \
                    re.findall(r"[\S\s]*?([0-9]+) ([A-Za-z]+ [0-9][0-9] [0-9][0-9]:[0-9][0-9]) ([\S\s]+)", fil)[0]
                newFile = {
                    'isDir': 'Directory' if fil.startswith('d') else 'File',
                    'name': findResult[2],
                    'time': findResult[1],
                    'size': convert(findResult[0])
                }
                self.serverFileList.append(newFile)
            except Exception as err:
                print(err)
                print(fil)
        self.serverArea.setFileList(self.serverFileList)

    def _threadClientDisconnect(self):
        self.serverArea.fileListWidget.blockSignals(True)
        self.serverArea.fileListWidget.clear()
        self.serverArea.fileListWidget.blockSignals(False)
        self.serverArea.serverPathLabel.setText('/')
        self.serverArea.serverFileLable.setText('')
        try:
            if self.isConnect:
                self.isConnect = False
                client.quit()
                client.reset()
        finally:
            self.echo('Status', 'Disconnected from server')

    def _preUpload(self):
        filename = self.localArea.localFileLable.text()
        if not self.isConnect:
            self.echo('Wrong', "No server connection")
            return
        if filename == '':
            self.echo('Wrong', "No local file selected")
            return
        self.echo('Status', 'Begin to upload \'{}\''.format(filename))
        status = client.upload(filename)
        if status:
            self.echo('Status', 'Upload "{}" successfully'.format(filename))

    def initButtons(self):
        self.inputRow.connButton.clicked.connect(self.connectServer)
        self.localArea.uploadButton.clicked.connect(lambda: self._preUpload())
        self.serverArea.downloadButton.clicked.connect(lambda: Thread(target=self._preDownload).start())
        self.serverArea.mkdirButton.clicked.connect(self.clientMkdir)
        self.serverArea.refreshButton.clicked.connect(self._refreshServerFileList)
        self.serverArea.rmdirButton.clicked.connect(self._preRemoveDir)
        self.serverArea.renameButton.clicked.connect(self._preRenameDir)

        self.serverArea.fileListWidget.clicked.connect(self._selectServerFile)
        self.serverArea.fileListWidget.doubleClicked.connect(self.clientChangeDir)

    def _preDownload(self):
        filename = self.serverArea.serverFileLable.text()
        if not self.isConnect:
            self.echo('Wrong', "No server connection")
            return
        if filename == '':
            self.echo('Wrong', "No server file selected")
            return
        self.echo('Status', 'Begin to download \'{}\''.format(filename))
        status = client.download(filename)
        if status:
            self.echo('Status', 'File "{}" transfer successful'.format(filename))

    def clientSyst(self):
        if not self.isConnect:
            self.echo('Wrong', "No server connection")
            return
        status, msg = client.send_syst()
        if status:
            self.echo('Status', msg)
        else:
            self.echo('Error', 'Get system info failed')

    def clientMkdir(self):
        if not self.isConnect:
            self.echo('Wrong', "No server connection")
            return
        value, ok = QInputDialog.getText(self, "New Folder", "Input the name of the new folder:", QLineEdit.Normal)
        if not value or not ok:
            return
        status = client.makeDir(value)
        if status:
            self.echo('Status', 'New Directory \'{}\' created'.format(value))
            self._refreshServerFileList()
        else:
            self.echo('Wrong', 'Fail to create folder \'{}\''.format(value))

    def clientChangeDir(self):
        if not self.isConnect:
            self.echo('Wrong', "No server connection")
            return
        currentItem = self.serverArea.fileListWidget.currentItem()
        currentWidget = self.serverArea.fileListWidget.itemWidget(currentItem)
        name = currentWidget.findChild(QLabel, 'name').text()
        isDir = currentWidget.findChild(QLabel, 'isDir').text()
        if isDir != 'Directory':
            return
        status = client.changeWrokDir(name)
        if status:
            self.echo('Status', 'Change work directory succeed')
            serverPath = client.printWorkDir()
            self.serverArea.serverPathLabel.setText(serverPath)
            self._refreshServerFileList()
        else:
            self.echo('Status', 'Change work directory failed')

    def _selectServerFile(self):
        if not self.isConnect:
            self.echo('Wrong', "No server connection")
            return
        currentItem = self.serverArea.fileListWidget.currentItem()
        currentWidget = self.serverArea.fileListWidget.itemWidget(currentItem)
        name = currentWidget.findChild(QLabel, 'name').text()
        isDir = currentWidget.findChild(QLabel, 'isDir').text()
        if isDir != 'File':
            return
        serverPath = self.serverArea.serverPathLabel.text()
        if serverPath == '/':
            serverPath = ''
        self.serverArea.serverFileLable.setText(serverPath + '/' + name)

    def _preRemoveDir(self):
        if not self.isConnect:
            self.echo('Wrong', "No server connection")
            return
        currentItem = self.serverArea.fileListWidget.currentItem()
        currentWidget = self.serverArea.fileListWidget.itemWidget(currentItem)
        name = currentWidget.findChild(QLabel, 'name').text()
        isDir = currentWidget.findChild(QLabel, 'isDir').text()
        if isDir != 'Directory':
            self.echo("Wrong", "No directory selected")
            return
        status = client.removeDir(name)
        self.echo("Status", "Remove folder succeed")
        self._refreshServerFileList()

    def _preRenameDir(self):
        if not self.isConnect:
            self.echo('Wrong', "No server connection")
            return
        currentItem = self.serverArea.fileListWidget.currentItem()
        currentWidget = self.serverArea.fileListWidget.itemWidget(currentItem)
        name = currentWidget.findChild(QLabel, 'name').text()
        isDir = currentWidget.findChild(QLabel, 'isDir').text()
        if isDir != 'Directory':
            self.echo("Wrong", "No directory selected")
            return
        value, ok = QInputDialog.getText(self, "Rename", "Input new name for folder {}:".format(name), QLineEdit.Normal)
        if not value or not ok:
            return
        self.echo('Status', 'Renaming "{}" to "{}"'.format(name, value))
        status = client.rename(name, value)
        if status:
            self.echo('Status', 'Rename succeed')
            self._refreshServerFileList()

    def initUI(self):
        self.statusBar()
        self.initToolbar()
        self.initInputRow()
        self.initButtons()

        mainVBox = QVBoxLayout()
        mainVBox.addWidget(self.inputRow)
        mainVBox.addWidget(self.initLogArea())

        hSpliter = QSplitter(Qt.Horizontal)
        hSpliter.addWidget(self.localArea)
        hSpliter.addWidget(_getDevideLine())
        hSpliter.addWidget(self.serverArea)

        mainVBox.addWidget(hSpliter)

        widget = QWidget()
        self.setCentralWidget(widget)
        widget.setLayout(mainVBox)

        self.resize(1200, 800)
        self._moveCenter()
        self.setWindowTitle('FTP-Client')
        self.setFont(QFont('Segoe UI', 10))
        self.setWindowIcon(QIcon('./res/icon.png'))
        self.show()


if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = MainWindow()
    sys.exit(app.exec_())
