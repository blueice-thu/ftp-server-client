import socket

size = 8192
serverPort = 9876
sequenceNumber = 1

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', serverPort))

try:
  while True:
    message, address = sock.recvfrom(size)
    modifiedMessage = str(sequenceNumber) + " " + message.decode()
    sock.sendto(modifiedMessage.encode(), address)
    sequenceNumber += 1
finally:
  sock.close()