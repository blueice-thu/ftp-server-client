import socket
 
size = 8192
serverName = 'localhost'
serverPort = 9876
 
try:
  for i in range(51):
    msg = "message " + str(i)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(msg.encode(), (serverName, serverPort))
    modifiedMessage, serverAddress = sock.recvfrom(size)
    print(modifiedMessage.decode())
    sock.close()
 
except:
  print("cannot reach the server")