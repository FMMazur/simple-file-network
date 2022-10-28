import os
import socket
import time

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

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as socket:
    socket.connect((HOST, PORT))
    welcome = socket.recv(1024).decode('ascii')
    print(f"{welcome}")

    connected = True
    while connected:
        inp = input("CLIENT: ")
        socket.send((inp + '\n').encode('ascii'))
        inp = inp.split(' ')

        if inp[0] == 'd' or inp[0] == 'disconnect':
            connected = False
        elif inp[0] == 's' or inp[0] == 'send':
            start = time.process_time_ns()
            file = os.path.realpath(inp[1])

            if not os.path.exists(file):
                print("file don't exists")
                socket.send(SERVER_FILE_FAILED.to_bytes(1, 'big'))
                print(SERVER_FILE_FAILED.to_bytes(1, 'big'))
                continue

            filesize = os.path.getsize(file)
            # s.send(inp[0].encode('ascii'))
            status = int.from_bytes(socket.recv(1), 'big')

            message = ''

            if status == SERVER_FILE_INFO_WAIT:
                socket.send(f"name: {inp[1]} size: {filesize}".encode('ascii'))
                status = socket.recv(BUFSIZE)

                if len(status) == 1 and int.from_bytes(status, 'big') == SERVER_FILE_WAITING:
                    with open(file, "rb") as f:
                        content = f.readlines()

                        for i in range(0, len(content), BUFSIZE):
                            sliceToSend = content[i:i + BUFSIZE]

                            socket.send(b''.join(sliceToSend))

                        status = socket.recv(BUFSIZE)

                        if len(status) == 1 and int.from_bytes(status, 'big') == SERVER_FILE_OK:
                            message = socket.recv(BUFSIZE).decode('ascii')
                        else:
                            message = status.decode('ascii')
                else:
                    message = status.decode('ascii')

            end = time.process_time_ns()
            print(f"{message}", end='' if message.endswith('\n') else '\n')
            elapsed = end-start
            ms = elapsed/10**6
            s = ms/10**3
            print(f'{elapsed}ns = {ms}ms = {s}s')
        else:
            data = socket.recv(BUFSIZE).decode('ascii')
            print(f"{data}", end='' if data.endswith('\n') else '\n')
