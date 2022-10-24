//
// Created by felipemazur on 22/10/22.
//

#include "server.h"

int main(int argc, char** argv) {
    arguments_t arguments;
    arguments.root_path = realpath(DEFAULT_PATH, NULL);
    arguments.port = DEFAULT_PORT;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    int server_socket, client_socket;
    SA_IN server_address, client_address;
    socklen_t address_size;
    server_info_t serverInfo = {.total = 0, .root_path = NULL, .directories = NULL};

    server_socket =  socket(AF_INET, SOCK_STREAM, 0);
    if (socket_check(server_socket, "")) {
        goto close_server;
    }

    printf(COLORIZED(FGREEN, "[+] socket created.\n"));

    memset(&server_address, 0, sizeof (server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(arguments.port);

    if (socket_check(bind(server_socket, (SA *) &server_address, sizeof(server_address)), "[-] binding failed.")) {
        goto close_server;
    }

    printf(COLORIZED(FGREEN, "[+] binding successfully.\n"));

    if (socket_check(listen(server_socket, SERVER_BACKLOG), "[-] Listening failed.")) {
        goto close_server;
    }

    printf(COLORIZED(FGREEN, "[+] listening at: %s:%d.\n"), inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));


    serverInfo.root_path = arguments.root_path;
    server_path_scan(&serverInfo);
    if (socket_check(serverInfo.total, "[-] directory scan failed.")) {
        goto close_server;
    }
    printf(COLORIZED(FGREEN, "[+] directory successfully scanned : %s\n"), serverInfo.root_path);

    bool quit = false;
    while (!quit)
    {
        address_size = sizeof(SA_IN);
        client_socket = accept(server_socket, (SA *) &client_address, (socklen_t *) &address_size);
        if (socket_check(client_socket,
                         "[-] connection failed.")) {
            continue;
        }

        printf(COLORIZED(FGREEN, "[+] client connected : %s:%d\n"), inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        int code = server_handle_connection(&serverInfo, client_socket);

        if (!code) continue;

        switch (code) {
            case SERVER_QUIT:
                quit = true;
                break;
            case SERVER_DISCONNECT:
                printf(COLORIZED(FYELLOW, "[*] socket disconnected: %s:%d\n"), inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
                break;
            default:
                break;
        }
    }

    close_server:
    socket_check(close(server_socket), "[-] server socket close failed.");
    server_info_clear(&serverInfo);

    if (errno) {
        return errno;
    }

    return 0;
}
