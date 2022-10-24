import socket

HOST = "127.0.0.1"  # The server's hostname or IP address
PORT = 21  # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    welcome = s.recv(1024).decode('ascii')
    print(f"{welcome}")

    connected = True
    while connected:
        inp = input("CLIENT: ")
        s.send((inp + "\n").encode('ascii'))

        if inp == 'd' or inp == 'disconnect':
            connected = False

        data = s.recv(4096).decode('ascii')
        print(f"{data}", end='' if data.endswith('\n') else '\n')

