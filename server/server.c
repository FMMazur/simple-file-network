//
// Created by felipemazur on 22/10/22.
//

#include "server.h"

int server_handle_connection(server_info_t *serverInfo, int socket)
{
  char readBuffer[BUFSIZE];
  char writeBuffer[BUFSIZE];

  ssize_t bytesRead;
  size_t messageSize;

  bool disconnect = false;

  send(socket, MESSAGE_WELCOME, sizeof(MESSAGE_WELCOME), 0);
  while (!disconnect) {
    messageSize = 0;

    while ((bytesRead = recv(socket, readBuffer, sizeof(readBuffer) - 1, 0)) > 0) {
      messageSize += bytesRead;

      if (messageSize > BUFSIZE - 1 || readBuffer[messageSize - 1] == '\n') { break; }
    }

    socket_check((i32) bytesRead, "[-] Error reading command");
    readBuffer[messageSize] = '\0';
    if (bytesRead) {
      printf(COLORIZED(FCYAN, "CLIENT: %s"), readBuffer);
    }

    token_t *tokens = tokenizer(readBuffer, " \n");

    if (bytesRead <= 0) {
      close(socket);
      disconnect = true;
    }
    else {
      char *command = token_get_string(tokens);
      // TODO: disable any kind of jailbreak from serverInfo.root_path

      if (strncasecmp(command, COMMAND_SHORT_DISCONNECT, sizeof(COMMAND_SHORT_DISCONNECT)) == 0 ||
          strncasecmp(command, COMMAND_DISCONNECT, sizeof(COMMAND_DISCONNECT)) == 0) {
        close(socket);
        disconnect = true;
      }
      else if (strncasecmp(command, COMMAND_SHORT_LIST, sizeof(COMMAND_SHORT_LIST)) == 0 ||
          strncasecmp(command, COMMAND_LIST, sizeof(COMMAND_LIST)) == 0) {
        messageSize = 0;
        const char *directory = token_get_string(tokens);

        int totalDirectoryInfo = serverInfo->total;
        dirent **directories = serverInfo->directories;

        if (directory) {
          char *path = realpath(directory, NULL);

          if (server_send_check(socket, (char *) writeBuffer, !path, "") == SERVER_SUCCESS) {
            totalDirectoryInfo = scandir(path, &directories, NULL, alphasort);
            free(path);

            if (server_send_check(socket, (char *) writeBuffer, totalDirectoryInfo == 0,
                                  "[-] directory scan failed.")) {
              continue;
            }
          }
        }

        if (totalDirectoryInfo == 0) {
          messageSize = sizeof(NSTR(MESSAGE_NO_DIRECTORIES)) + 1;
          snprintf(writeBuffer, messageSize - 1, NSTR(MESSAGE_NO_DIRECTORIES));
          writeBuffer[messageSize] = '\0';
        }
        else {
          messageSize = 0;
          size_t wroteCount = sizeof(MESSAGE_LIST_DIRECTORY) - 1;
          strncpy(writeBuffer, MESSAGE_LIST_DIRECTORY, wroteCount);

          for (int cnt = 0; cnt < totalDirectoryInfo; ++cnt) {
            dirent *dir = directories[cnt];

            if (dir->d_type != DT_DIR && dir->d_type != DT_REG && dir->d_type != DT_LNK) { continue; }

            const char *color =
                dir->d_type == DT_DIR ? MBOLD FBLUE : dir->d_type == DT_LNK ? FLBLUE : FWHITE;

            size_t maxlen = BUFSIZE - wroteCount - 1;
            if (strlen(dir->d_name) > maxlen) {
              writeBuffer[wroteCount] = '\0';
              send(socket, writeBuffer, wroteCount, MSG_MORE); // send packet

              // clear counts to keep listing
              wroteCount = 0;
              maxlen = BUFSIZE - 1;
            }

            wroteCount += snprintf(writeBuffer + wroteCount, maxlen,
                                   COLORIZED("%s", "%s\n"), color, dir->d_name);
          }

          messageSize = wroteCount;
          writeBuffer[messageSize] = '\0';
        }

        if (messageSize) {
          send(socket, writeBuffer, messageSize, 0); // send packet
        }
      }
      else if (strncasecmp(command, COMMAND_SHORT_CREATE_DIR, sizeof(COMMAND_SHORT_CREATE_DIR)) == 0 ||
          strncasecmp(command, COMMAND_CREATE_DIR, sizeof(COMMAND_CREATE_DIR)) == 0) {
        const char *root_path = serverInfo->root_path;
        const char *directory = token_get_string(tokens);

        if (!directory) {
          printf(MESSAGE_DIR_NO_PATH);
          send(socket, MESSAGE_DIR_NO_PATH, sizeof(MESSAGE_DIR_NO_PATH), 0); // send packet
        }
        else {
          char *path = malloc(PATH_MAX);
          if (path) {
            size_t pathLen = snprintf(path, PATH_MAX - 1, "%s/%s", serverInfo->root_path, directory);
            path[pathLen] = '\0';

            if (SERVER_SUCCESS == server_send_check(socket, (char *) writeBuffer, mkdir(path, S_IRWXU),
                                                    "[-] Failed to create directory")) {
              size_t wroteCount = snprintf(writeBuffer, sizeof(MESSAGE_CREATE_DIR_SUCCESS) + pathLen,
                                           MESSAGE_CREATE_DIR_SUCCESS, path);
              writeBuffer[wroteCount] = '\0';

              server_path_scan(serverInfo);
              if (SERVER_SUCCESS ==
                  server_send_check(serverInfo->total, writeBuffer, serverInfo->total < 0,
                                    "[-] directory scan failed.")) {
                printf("%s", writeBuffer);
                printf(COLORIZED(FLCYAN, NSTR("[^] directory rescanned: %s")), serverInfo->root_path);
                send(socket, writeBuffer, wroteCount, 0);
              }
            }

            free(path);
          }
        }
      }
      else if (strncasecmp(command, COMMAND_SHORT_REMOVE_DIR, sizeof(COMMAND_SHORT_REMOVE_DIR)) == 0 ||
          strncasecmp(command, COMMAND_REMOVE_DIR, sizeof(COMMAND_REMOVE_DIR)) == 0) {
        bool recursive = false;
        char *directory = NULL;
        size_t totalDirectories = 0;

        // TODO: delete multiple directory
        for (size_t i = tokens->currentIdx; i < tokens->count; ++i) {
          command = token_get_string(tokens);

          if (strncasecmp(command, COMMAND_OPTION_RECURSIVE, sizeof(COMMAND_OPTION_RECURSIVE)) == 0
              && !recursive) {
            recursive = true;
          }
          else if (!directory) {
            directory = command;
          }

        }

        if (!directory) {
          printf(MESSAGE_DIR_NO_PATH);
          send(socket, MESSAGE_DIR_NO_PATH, sizeof(MESSAGE_DIR_NO_PATH), 0); // send packet
        }
        else {
          char *path = realpath(directory, NULL);
          if (SERVER_SUCCESS == server_send_check(socket, (char *) writeBuffer, !path, "")) {
            int status = SERVER_SUCCESS;
            if (recursive) {
              status = server_send_check(socket, (char *) writeBuffer,
                                         server_rmrf(path) != SERVER_SUCCESS,
                                         "[-] Failed to recursive delete.");
            }
            else {
              status = server_send_check(socket, (char *) writeBuffer, rmdir(path), "");
            }

            if (status == SERVER_SUCCESS) {
              server_path_scan(serverInfo);

              if (SERVER_SUCCESS ==
                  server_send_check(serverInfo->total, writeBuffer, serverInfo->total < 0,
                                    "[-] directory scan failed.")) {
                size_t wroteCount = snprintf(writeBuffer,
                                             sizeof(MESSAGE_REMOVE_DIR_SUCCESS) + strlen(path),
                                             MESSAGE_REMOVE_DIR_SUCCESS, path);
                writeBuffer[wroteCount] = '\0';

                printf("%s", writeBuffer);
                printf(COLORIZED(FLCYAN, NSTR("[^] directory rescanned: %s")), serverInfo->root_path);
                send(socket, writeBuffer, wroteCount, 0);
              }
            }

            free(path);
          }
        }
      }
      else if (strncasecmp(command, COMMAND_SHORT_SEND_FILE, sizeof(COMMAND_SHORT_SEND_FILE)) == 0 ||
          strncasecmp(command, COMMAND_SEND_FILE, sizeof(COMMAND_SEND_FILE)) == 0) {
        writeBuffer[0] = SERVER_FILE_INFO_WAIT;

        if (SOCKET_ERROR
            != socket_check(send(socket, writeBuffer, 1, 0),
                            "[-] sending file info waiting status.")) {
          messageSize = recv(socket, readBuffer, sizeof(readBuffer) - 1, 0);

          if (SERVER_SUCCESS == socket_check(messageSize, "[-] receiving file info from socket.")) {
            readBuffer[messageSize] = '\0';

            if (messageSize == 1 && *((u8*) readBuffer) == SERVER_FILE_FAILED) {
              printf(COLORIZED(FLRED, NSTR("file don't exists on client side.")));
              continue;
            }

            // FIXME: because of the tokenizer, we don't support file with spaces for now
            token_t *fileParser = tokenizer(readBuffer, ": ");

            const char *fileToken = token_get_string(fileParser);
            const char *filename = token_get_string(fileParser);
            const char *fileSizeToken = token_get_string(fileParser);
            const char *fileSizeStr = token_get_string(fileParser);
            size_t filesize = strtoumax(fileSizeStr, NULL, 10);

            // TODO: use SERVER_FILE_INFO_ERROR instead of message?
            if (SERVER_SUCCESS == server_send_check(socket,
                                                    writeBuffer,
                                                    !fileToken || !filename || !fileSizeToken || !fileSizeStr
                                                        || filesize == UINTMAX_MAX,
                                                    "[-] on file data.")) {

              char *finalFileName = malloc(PATH_MAX);
              if (SERVER_SUCCESS == server_send_check(socket, writeBuffer, finalFileName == NULL, "")) {
                size_t pathLen = snprintf(finalFileName, PATH_MAX - 1, "%s/%s", serverInfo->root_path, filename);
                finalFileName[pathLen] = '\0';

                FILE *receivedFile = fopen(finalFileName, "wb");

                if (SERVER_SUCCESS
                    != server_send_check(socket, writeBuffer, receivedFile == NULL, "[-] on create server side file")) {
                  continue;
                }
                bool success = true;
                ssize_t dataRecoveredCount = 0;

                writeBuffer[0] = SERVER_FILE_WAITING;
                if (SERVER_SUCCESS
                    == socket_check(send(socket, writeBuffer, 1, 0),
                                    "[-] on sending file info ok status.")) {
                  for (size_t currentByte = 0; currentByte < filesize && success; currentByte += dataRecoveredCount) {
                    dataRecoveredCount = recv(socket, readBuffer, BUFSIZE, 0);
                    size_t wroteCount = fwrite(readBuffer, sizeof(readBuffer[0]), dataRecoveredCount, receivedFile);

                    if (dataRecoveredCount <= 0 || wroteCount < dataRecoveredCount) {
                      success = false;
                    }
                  }

                  if (SERVER_SUCCESS == server_send_check(socket, writeBuffer, !success, "[-] on upload to server.")) {
                    size_t writeSize = ftell(receivedFile);
                    fclose(receivedFile);

                    if (SERVER_SUCCESS == server_send_check(socket,
                                                            writeBuffer,
                                                            writeSize < dataRecoveredCount,
                                                            "[-] on write file on server.")) {

                      writeBuffer[0] = SERVER_FILE_OK;
                      if (SERVER_SUCCESS == socket_check(send(socket, writeBuffer, 1, 0),
                                                         "[-] on send ok status")) {
                        server_path_scan(serverInfo);

                        if (SERVER_SUCCESS ==
                            server_send_check(serverInfo->total, writeBuffer, serverInfo->total < 0,
                                              "[-] directory scan failed.")) {
                          size_t wroteCount = snprintf(writeBuffer,
                                                       sizeof(MESSAGE_SEND_FILE_SUCCESS) + strlen(filename),
                                                       MESSAGE_SEND_FILE_SUCCESS, filename);
                          writeBuffer[wroteCount] = '\0';

                          printf("%s", writeBuffer);
                          printf(COLORIZED(FLCYAN, NSTR("[^] directory rescanned: %s")), serverInfo->root_path);
                          send(socket, writeBuffer, wroteCount, 0);
                        }
                      }
                    }
                  }
                }
              }

              token_free(fileParser);
            }
          }
        }
      }
      else if (strncasecmp(command, COMMAND_SHORT_REMOVE_FILE, sizeof(COMMAND_SHORT_REMOVE_FILE)) == 0 ||
          strncasecmp(command, COMMAND_REMOVE_FILE, sizeof(COMMAND_REMOVE_FILE)) == 0) {
        char *file = NULL;
        // TODO: delete multiple files
        for (size_t i = tokens->currentIdx; i < tokens->count; ++i) {
          command = token_get_string(tokens);

          if (!file) {
            file = command;
          }
        }

        char *filepath = realpath(file, NULL);
        if (SERVER_SUCCESS == server_send_check(socket, (char *) writeBuffer, !filepath, "")) {
          struct stat path_stat;

          if (!server_send_check(socket, (char *) writeBuffer, stat(filepath, &path_stat), "")) {
            if (SERVER_SUCCESS == server_send_check(socket, (char *) writeBuffer, remove(filepath), "")) {
              server_path_scan(serverInfo);

              if (SERVER_SUCCESS ==
                  server_send_check(serverInfo->total, writeBuffer, serverInfo->total < 0,
                                    "[-] directory scan failed.")) {
                bool isDir = S_ISDIR(path_stat.st_mode);
                const char *successMessage = isDir ? MESSAGE_REMOVE_DIR_SUCCESS : MESSAGE_REMOVE_FILE_SUCCESS;
                messageSize = isDir ? sizeof(MESSAGE_REMOVE_DIR_SUCCESS)
                                    : sizeof(MESSAGE_REMOVE_FILE_SUCCESS);

                size_t wroteCount = snprintf(writeBuffer,
                                             messageSize + strlen(filepath),
                                             successMessage, filepath);
                writeBuffer[wroteCount] = '\0';

                printf("%s", writeBuffer);
                printf(COLORIZED(FLCYAN, NSTR("[^] directory rescanned: %s")), serverInfo->root_path);
                send(socket, writeBuffer, wroteCount, 0);
              }
            }
          }

          free(filepath);
        }
      }
      else {
        send(socket, MESSAGE_HELP, sizeof(MESSAGE_HELP), 0); // send packet
      }
    }

    if (tokens) {
      token_free(tokens);
    }
  }

  return SERVER_DISCONNECT;
}

void server_info_clear(server_info_t *info)
{
  if (info->root_path) {
    free(info->root_path);
  }

  if (info->directories) {
    while (info->total--) {
      free(info->directories[info->total]);
    }
    free(info->directories);
  }

  info->total = 0;
  info->root_path = NULL;
  info->directories = NULL;
}

static int server_send_check(int socket, char *writeBuffer, int code, const char *message)
{
  if (code != SERVER_SUCCESS) {
    size_t size = sizeof(char) * BUFSIZE;
    size_t messageSize = strlen(message);
    size_t wroteCount = 0;

    if (messageSize) {
      wroteCount = snprintf(writeBuffer, size - 1, COLORIZED(FRED, "[-] %s\n\t%s\n"), strerror(errno),
                            message);
    }
    else {
      wroteCount = snprintf(writeBuffer, size - 1, COLORIZED(FRED, "[-] %s\n\t"), strerror(errno));
    }
    writeBuffer[wroteCount] = '\0';

    SA_IN client_address;
    socklen_t address_size;

    getpeername(socket, (SA *) &client_address, (socklen_t *) &address_size);
    printf(COLORIZED(FRED, "[-] socket %s:%d\n\t%s"), inet_ntoa(client_address.sin_addr),
           ntohs(client_address.sin_port), writeBuffer);
    send(socket, writeBuffer, wroteCount, 0);

    return errno;
  }

  return 0;
}

void server_path_scan(server_info_t *serverInfo)
{
  serverInfo->total = scandir(serverInfo->root_path, &serverInfo->directories, NULL, alphasort);
}

int server_dir_remove_depth_cb(const char *fpath, const struct stat *sb, int typeFlag, struct FTW *ftwbuf)
{
  int rv = remove(fpath);

  return rv;
}

int server_rmrf(char *path)
{
  return nftw(path, server_dir_remove_depth_cb, 64, FTW_DEPTH | FTW_PHYS);
}
