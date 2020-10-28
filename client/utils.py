import os
import socket
import random
import re

ipPattern = re.compile(r"^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?["
                       r"0-9][0-9]?)$")


def getLinuxCwd():
    return os.getcwd().replace('\\', '/')


def getLocalIP():
    ip = None
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.connect(('8.8.8.8', 80))
        ip = sock.getsockname()[0]
    finally:
        sock.close()
        return ip


def getRandomPort():
    return random.randint(5001, 65535)


def convert(byte):
    byte = int(byte)
    if byte < 1024:
        return str(byte) + 'B'
    kbyte = byte // 1024
    if kbyte < 1024:
        return str(kbyte) + 'KB'
    mbyte = kbyte // 1024
    if mbyte < 1024:
        return str(mbyte) + 'MB'
    gbyte = mbyte // 1024
    return str(gbyte) + 'GB'
