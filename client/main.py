import os
import re
import sys
import threading

from PyQt5.QtCore import Qt, QDir, QStringListModel, QSize
from PyQt5.QtGui import QIcon, QFont, QTextCursor
from PyQt5.QtWidgets import (QWidget, QMainWindow, QAction, qApp, QLineEdit, QLabel, QTextEdit, QListWidgetItem,
                             QToolButton, QMenu, QSizePolicy, QPushButton, QApplication, QScrollBar,
                             QDesktopWidget, QInputDialog, QFileSystemModel, QListWidget, QSplitter, QHBoxLayout,
                             QVBoxLayout, QTreeView)

from client import Client


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.hostTextEdit = QLineEdit()
        self.usernameTextEdit = QLineEdit()
        self.passwordTextEdit = QLineEdit()
        self.portTextEdit = QLineEdit()
        self.logEditText = QTextEdit()

        self.client = None

        self.localPathLabel = QLabel(os.getcwd())
        self.serverPathLabel = QLabel('/')
        self.localFileLable = QLabel('')
        self.serverFileLable = QLabel('')

        self.localModel = QFileSystemModel()
        self.localModel.setRootPath(QDir.rootPath())
        self.serverModel = QStringListModel()
        self.serverFileList = []
        self.serverFileListWidget = QListWidget()

        self.isConnect = False

        self.initUI()

    def saveHistory(self):
        with open("history.ini", "w+") as fp:
            fp.write("{},{},{},{}\n".format(
                self.hostTextEdit.text(),
                self.usernameTextEdit.text(),
                self.passwordTextEdit.text(),
                self.portTextEdit.text(),
            ))

    @staticmethod
    def readHistory():
        records = []
        with open("history.ini", "r") as fp:
            record = fp.readline()
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
        self.hostTextEdit.setText(connInfo[0])
        self.usernameTextEdit.setText(connInfo[1])
        self.passwordTextEdit.setText(connInfo[2])
        self.portTextEdit.setText(connInfo[3])

    def clearInput(self):
        self.hostTextEdit.clear()
        self.usernameTextEdit.clear()
        self.passwordTextEdit.clear()
        self.portTextEdit.clear()

    def initToolbar(self):
        toolbar = self.addToolBar('FTP client')
        toolbar.setMovable(False)

        refreshAction = QAction(QIcon('res/refresh.png'), '&Refresh', self)
        refreshAction.setStatusTip('Refresh')
        refreshAction.triggered.connect(self._initServerFileList)
        toolbar.addAction(refreshAction)

        disconectAction = QAction(QIcon('res/disconnect.png'), '&Disconnect', self)
        disconectAction.setStatusTip('Disconnect')
        disconectAction.triggered.connect(self._disconnect)
        toolbar.addAction(disconectAction)

        exitAction = QAction(QIcon('res/exit.png'), '&Exit', self)
        exitAction.setStatusTip('Exit')
        exitAction.triggered.connect(qApp.quit)
        toolbar.addAction(exitAction)

    def moveCenter(self):
        qr = self.frameGeometry()
        cp = QDesktopWidget().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())

    def initConnectRow(self):
        self.hostTextEdit.setFixedWidth(120)
        self.usernameTextEdit.setFixedWidth(120)
        self.passwordTextEdit.setFixedWidth(120)
        self.portTextEdit.setMaxLength(5)
        self.portTextEdit.setFixedWidth(80)

        hbox = QHBoxLayout()
        hbox.addWidget(QLabel('Host:', self))
        hbox.addWidget(self.hostTextEdit)

        hbox.addWidget(QLabel('Username:', self))
        hbox.addWidget(self.usernameTextEdit)

        hbox.addWidget(QLabel('Password:', self))
        hbox.addWidget(self.passwordTextEdit)

        hbox.addWidget(QLabel('Port:', self))
        hbox.addWidget(self.portTextEdit)

        connButton = QPushButton('Quickconnect', self)
        connButton.clicked.connect(self.connectServer)
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
            recordAction.triggered.connect(lambda: self.loadHistory(record))
            recordActions.append(recordAction)
        menu.addSeparator()
        menu.addActions(recordActions)

        moreButton = QToolButton()
        moreButton.setArrowType(Qt.DownArrow)
        moreButton.setMenu(menu)
        moreButton.setPopupMode(QToolButton.InstantPopup)

        hbox.addWidget(connButton)
        hbox.addWidget(moreButton)
        hbox.addStretch(1)
        return hbox

    def initLogArea(self):
        self.logEditText.setAlignment(Qt.AlignLeft | Qt.AlignTop)
        self.logEditText.setStyleSheet("font-size: 12; font-family: Segoe UI; padding: 5px")
        self.logEditText.setMinimumHeight(80)
        self.logEditText.setText("Log output:")
        self.logEditText.setReadOnly(True)
        self.logEditText.setMaximumHeight(120)
        self.logEditText.ensureCursorVisible()
        scrollBar = QScrollBar()
        self.logEditText.setVerticalScrollBar(scrollBar)
        return self.logEditText

    def logOut(self, title, content):
        self.logEditText.append(str(title) + ': ' + str(content))
        cursor = self.logEditText.textCursor()
        cursor.movePosition(QTextCursor.End, QTextCursor.MoveAnchor)
        self.logEditText.setTextCursor(cursor)

    def connectServer(self):
        try:
            self.client.quit()
        except Exception as err:
            print(err)
        ipPattern = re.compile(r"^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?["
                               r"0-9][0-9]?)$")
        if not ipPattern.fullmatch(self.hostTextEdit.text()):
            self.logOut('Wrong', 'Invaild host!')
            return
        if not self.portTextEdit.text().isdigit():
            self.logOut('Wrong', 'Invaild port!')
            return
        self.client = Client({
            'host': self.hostTextEdit.text(),
            'username': self.usernameTextEdit.text(),
            'password': self.passwordTextEdit.text(),
            'port': int(self.portTextEdit.text())
        })
        status, msg = self.client.connect()
        if not status:
            self.logOut('Status', msg)
            return
        self.logOut('Status', 'Connection established, waiting for welcome message...')
        self.logOut('Status', msg)
        status, msg = self.client.login()
        if not status:
            self.logOut('Wrong', 'Login failed!')
            return
        self.isConnect = True
        self.logOut('Status', 'Logged in')
        self._initServerFileList()

    def _refreshFileInfoList(self):
        if not self.isConnect:
            return
        self.serverFileList = [{
            'name': 'Name',
            'isDir': 'Type',
            'size': 'Size',
            'time': 'Modify time'
        }]
        if self.serverPathLabel.text() != '/':
            self.serverFileList.append({
                'name': '..',
                'isDir': 'Directory',
                'size': '',
                'time': ''
            })
        self.logOut('Status', 'Retrieving directory listing of "{}"...'.format(self.serverPathLabel.text()))
        status, fileInfoList, _ = self.client.listFiles()
        if not status:
            self.logOut('Wrong', 'Retrieve directory listing failed')
            return
        self.logOut('Status', 'Directory listing of "/" successful')
        for fil in fileInfoList:
            try:
                findResult = \
                    re.findall(r"[\S\s]*?([0-9]+) ([A-Za-z]+ [0-9][0-9] [0-9][0-9]:[0-9][0-9]) ([\S\s]+)", fil)[0]
                newFile = {
                    'isDir': 'Directory' if fil.startswith('d') else 'File',
                    'name': findResult[2],
                    'time': findResult[1],
                    'size': findResult[0]
                }
                self.serverFileList.append(newFile)
            except Exception as err:
                print(err)
                print(fil)
        # for i in self.serverFileList:
        #     print(i)

    def _disconnect(self):
        try:
            self.serverFileListWidget.blockSignals(True)
            self.serverFileListWidget.clear()
            self.serverFileListWidget.blockSignals(False)
            item, widget = self._getFileListItem('Name', 'Type', 'Size', 'Modify time')
            self.serverFileListWidget.addItem(item)
            self.serverFileListWidget.setItemWidget(item, widget)
            self.isConnect = False
            self.client.quit()
        finally:
            self.logOut('Status', 'Disconnected from server')

    def _localTreeViewClicked(self, indexItem):
        localPathOrFile = self.localModel.filePath(indexItem).replace('/', '\\')
        if self.localModel.isDir(indexItem):
            self.localPathLabel.setText(localPathOrFile)
        else:
            self.localFileLable.setText(localPathOrFile)
            localPathOrFile = re.findall(r'([\s\S]+)\\[\s\S]+?', localPathOrFile)[0]
            self.localPathLabel.setText(localPathOrFile)
        os.chdir(localPathOrFile)

    def initLocalArea(self):
        localWidget = QWidget()
        localArea = QVBoxLayout()
        siteBar = QHBoxLayout()
        self.localPathLabel.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.localFileLable.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        siteBar.addWidget(QLabel('<b>Local site</b>:'))
        siteBar.addWidget(self.localPathLabel, stretch=1)
        siteBar.addStretch(1)
        siteBar.addWidget(QLabel('<b>Selected file</b>:'))
        siteBar.addWidget(self.localFileLable, stretch=1)
        siteBar.addStretch(1)
        localArea.addLayout(siteBar)

        treeView = QTreeView()
        treeView.setModel(self.localModel)
        treeView.clicked.connect(self._localTreeViewClicked)
        localPath = self.localPathLabel.text()
        for i in range(len(localPath)):
            if localPath[i] == '\\':
                treeView.expand(self.localModel.index(localPath[:i]))
        treeView.expand(self.localModel.index(localPath))

        localArea.addWidget(treeView)
        localWidget.setLayout(localArea)
        return localWidget

    @staticmethod
    def _getDevideLine():
        devideLine = QPushButton()
        devideLine.setMaximumWidth(1)
        devideLine.setFocusPolicy(Qt.NoFocus)
        return devideLine

    def _getFileListItem(self, name, isDir, size, modifyTime):
        widget = QWidget()
        layout = QHBoxLayout()

        nameLabel = QLabel(name)
        nameLabel.setObjectName('name')
        layout.addWidget(nameLabel, stretch=1)
        layout.addWidget(self._getDevideLine())

        isDirLabel = QLabel(isDir)
        isDirLabel.setObjectName('isDir')
        layout.addWidget(isDirLabel, stretch=1)
        layout.addWidget(self._getDevideLine())

        sizeLabel = QLabel(size)
        sizeLabel.setObjectName('size')
        layout.addWidget(sizeLabel, stretch=1)
        layout.addWidget(self._getDevideLine())

        modifyTimeLabel = QLabel(modifyTime)
        modifyTimeLabel.setObjectName('modifyTime')
        layout.addWidget(modifyTimeLabel, stretch=1)

        widget.setLayout(layout)

        item = QListWidgetItem()
        item.setSizeHint(QSize(0, 43))

        return item, widget

    def _initServerFileList(self):
        self._refreshFileInfoList()
        self.serverFileListWidget.blockSignals(True)
        self.serverFileListWidget.clear()
        self.serverFileListWidget.blockSignals(False)
        for file in self.serverFileList:
            item, widget = self._getFileListItem(file['name'], file['isDir'], file['size'], file['time'])
            self.serverFileListWidget.addItem(item)
            self.serverFileListWidget.setItemWidget(item, widget)

    def initServerArea(self):
        remoteWidget = QWidget()
        remoteArea = QVBoxLayout()
        siteBar = QHBoxLayout()
        self.serverPathLabel.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.serverFileLable.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        siteBar.addWidget(QLabel('<b>Remote site</b>:'))
        siteBar.addWidget(self.serverPathLabel, stretch=1)
        siteBar.addStretch(1)
        siteBar.addWidget(QLabel('<b>Selected file</b>:'))
        siteBar.addWidget(self.serverFileLable, stretch=1)
        siteBar.addStretch(1)
        remoteArea.addLayout(siteBar)

        self.serverFileListWidget.clicked.connect(self._selectServerFile)
        self.serverFileListWidget.doubleClicked.connect(self._preChangeDir)

        remoteArea.addWidget(self.serverFileListWidget)
        self._initServerFileList()

        remoteWidget.setLayout(remoteArea)
        return remoteWidget

    def _preUpload(self):
        filename = self.localFileLable.text()
        if not self.isConnect:
            self.logOut('Wrong', "No server connection")
            return
        if filename == '':
            self.logOut('Wrong', "No local file selected")
            return
        self.logOut('Status', 'Begin to upload \'{}\''.format(filename))
        status = self.client.upload(filename)
        if status:
            self.logOut('Status', 'Upload "{}" successfully'.format(filename))

    def initLocalButtonArea(self):
        uploadButton = QPushButton('Upload', self)
        uploadButton.clicked.connect(lambda: threading.Thread(target=self._preUpload).start())

        localWidget = QWidget()
        hBox = QHBoxLayout()
        hBox.addStretch(1)
        hBox.addWidget(uploadButton)
        localWidget.setLayout(hBox)

        return localWidget

    def _preDownload(self):
        filename = self.serverFileLable.text()
        if not self.isConnect:
            self.logOut('Wrong', "No server connection")
            return
        if filename == '':
            self.logOut('Wrong', "No server file selected")
            return
        self.logOut('Status', 'Begin to download \'{}\''.format(filename))
        status = self.client.download(filename)
        if status:
            self.logOut('Status', 'File "{}" transfer successful'.format(filename))

    def _preMkdir(self):
        if not self.isConnect:
            self.logOut('Wrong', "No server connection")
            return
        value, ok = QInputDialog.getText(self, "New Folder", "Input the name of the new folder:", QLineEdit.Normal)
        if not value or not ok:
            return
        status = self.client.makeDir(value)
        if status:
            self.logOut('Status', 'New Directory \'{}\' created'.format(value))
        else:
            self.logOut('Wrong', 'Fail to create folder \'{}\''.format(value))

    def _preChangeDir(self):
        if not self.isConnect:
            self.logOut('Wrong', "No server connection")
            return
        currentItem = self.serverFileListWidget.currentItem()
        currentWidget = self.serverFileListWidget.itemWidget(currentItem)
        name = currentWidget.findChild(QLabel, 'name').text()
        isDir = currentWidget.findChild(QLabel, 'isDir').text()
        if isDir != 'Directory':
            return
        status = self.client.changeWrokDir(name)
        if status:
            serverPath = self.client.printWorkDir()
            self.serverPathLabel.setText(serverPath)
            self._initServerFileList()

    def _selectServerFile(self):
        if not self.isConnect:
            self.logOut('Wrong', "No server connection")
            return
        currentItem = self.serverFileListWidget.currentItem()
        currentWidget = self.serverFileListWidget.itemWidget(currentItem)
        name = currentWidget.findChild(QLabel, 'name').text()
        isDir = currentWidget.findChild(QLabel, 'isDir').text()
        if isDir != 'File':
            return
        serverPath = self.serverPathLabel.text()
        if serverPath == '/':
            serverPath = ''
        self.serverFileLable.setText(serverPath + '/' + name)

    def _preRemoveDir(self):
        if not self.isConnect:
            self.logOut('Wrong', "No server connection")
            return
        currentItem = self.serverFileListWidget.currentItem()
        currentWidget = self.serverFileListWidget.itemWidget(currentItem)
        name = currentWidget.findChild(QLabel, 'name').text()
        isDir = currentWidget.findChild(QLabel, 'isDir').text()
        if isDir != 'Directory':
            self.logOut("Wrong", "No directory selected")
            return
        status = self.client.removeDir(name)
        print('status: ', status)

    def _preRenameDir(self):
        if not self.isConnect:
            self.logOut('Wrong', "No server connection")
            return
        currentItem = self.serverFileListWidget.currentItem()
        currentWidget = self.serverFileListWidget.itemWidget(currentItem)
        name = currentWidget.findChild(QLabel, 'name').text()
        isDir = currentWidget.findChild(QLabel, 'isDir').text()
        if isDir != 'Directory':
            self.logOut("Wrong", "No directory selected")
            return
        value, ok = QInputDialog.getText(self, "Rename", "Input new name for folder {}:".format(name), QLineEdit.Normal)
        if not value or not ok:
            return
        status = self.client.rename(name, value)
        self.logOut('Status', 'Renaming "{}" to "{}"'.format(name, value))
        print(status)
        # if status:
        #     self.logOut('Status', 'New Directory \'{}\' created'.format(value))
        # else:
        #     self.logOut('Wrong', 'Fail to create folder \'{}\''.format(value))

    def initServerButtonArea(self):
        downloadButton = QPushButton('Download', self)
        downloadButton.clicked.connect(lambda: threading.Thread(target=self._preDownload).start())
        mkdirButton = QPushButton('New Folder', self)
        mkdirButton.clicked.connect(self._preMkdir)
        refreshButton = QPushButton('Refresh', self)
        refreshButton.clicked.connect(self._initServerFileList)
        rmdirButton = QPushButton('Remove folder', self)
        rmdirButton.clicked.connect(self._preRemoveDir)
        renameButton = QPushButton('Rename', self)
        renameButton.clicked.connect(self._preRenameDir)

        serverWidget = QWidget()
        hBox = QHBoxLayout()
        hBox.addStretch(1)
        hBox.addWidget(downloadButton)
        hBox.addWidget(mkdirButton)
        hBox.addWidget(refreshButton)
        hBox.addWidget(rmdirButton)
        hBox.addWidget(renameButton)
        serverWidget.setLayout(hBox)
        return serverWidget

    def initUI(self):
        self.statusBar()
        self.initToolbar()

        mainVBox = QVBoxLayout()
        mainVBox.addLayout(self.initConnectRow())
        mainVBox.addWidget(self.initLogArea())

        vSpliter = QSplitter(Qt.Vertical)
        vSpliter.addWidget(self.initLocalArea())
        vSpliter.addWidget(self.initLocalButtonArea())
        vSpliter.addWidget(self.initServerArea())
        vSpliter.addWidget(self.initServerButtonArea())

        mainVBox.addWidget(vSpliter)
        mainVBox.addStretch(1)

        widget = QWidget()
        self.setCentralWidget(widget)
        widget.setLayout(mainVBox)

        self.resize(1200, 800)
        self.moveCenter()
        self.setWindowTitle('FTP-Client')
        self.setFont(QFont('Segoe UI', 10))
        self.setWindowIcon(QIcon('./res/icon.png'))
        self.show()


if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = MainWindow()
    sys.exit(app.exec_())
