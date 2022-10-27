import os
import socket

SERVER_FILE_INFO_WAIT = 0b00000001
SERVER_FILE_INFO_OK = 0b00000010
SERVER_FILE_INFO_ERROR = 0b00000100
SERVER_FILE_WAITING = 0b00001000
SERVER_FILE_PENDING = 0b00010000
SERVER_FILE_OK = 0b01000000
SERVER_FILE_FAILED = 0b10000000

HOST = "127.0.0.1"  # The server's hostname or IP address
PORT = 8080 # The port used by the server

BUFSIZE = 4096

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    welcome = s.recv(1024).decode('ascii')
    print(f"{welcome}")

    connected = True
    while connected:
        inp = input("CLIENT: ")
        s.send((inp + '\n').encode('ascii'))
        inp = inp.split(' ')

        if inp[0] == 'd' or inp[0] == 'disconnect':
            connected = False
        elif inp[0] == 's' or inp[0] == 'send':
            file = os.path.realpath(inp[1])

            if not os.path.exists(file):
                print("file don't exists")
                s.send(SERVER_FILE_FAILED.to_bytes(1, 'big'))
                print(SERVER_FILE_FAILED.to_bytes(1, 'big'))
                continue

            filesize = os.path.getsize(file)
            # s.send(inp[0].encode('ascii'))
            status = int.from_bytes(s.recv(1), 'big')

            message = ''

            if status == SERVER_FILE_INFO_WAIT:
                s.send(f"name: {inp[1]} size: {filesize}".encode('ascii'))
                status = s.recv(BUFSIZE)

                if len(status) == 1 and int.from_bytes(status, 'big') == SERVER_FILE_WAITING:
                    with open(file, encoding="ascii") as f:
                        content = ''.join(f.readlines()).encode('ascii')

                        for i in range(0, len(content), BUFSIZE):
                            sliceToSend = content[i:i + BUFSIZE]

                            s.send(sliceToSend)

                        status = s.recv(BUFSIZE)

                        if len(status) == 1 and int.from_bytes(status, 'big') == SERVER_FILE_OK:
                            message = s.recv(BUFSIZE).decode('ascii')
                        else:
                            message = status.decode('ascii')
                else:
                    message = status.decode('ascii')

            print(f"{message}", end='' if message.endswith('\n') else '\n')
        else:
            data = s.recv(BUFSIZE).decode('ascii')
            print(f"{data}", end='' if data.endswith('\n') else '\n')
