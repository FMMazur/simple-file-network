//
// Created by felipemazur on 22/10/22.
//

#ifndef FTP_SERVER_H
#define FTP_SERVER_H

#include "common.h"

#define SERVER_BACKLOG 10

#define SERVER_SUCCESS 0
#define SERVER_ERROR (-1)
#define SERVER_QUIT 100001
#define SERVER_DISCONNECT 100002

#define SERVER_FILE_INFO_WAIT   (u8) 0b00000001
#define SERVER_FILE_INFO_OK     (u8) 0b00000010
#define SERVER_FILE_INFO_ERROR  (u8) 0b00000100
#define SERVER_FILE_WAITING     (u8) 0b00001000
#define SERVER_FILE_PENDING     (u8) 0b00010000
#define SERVER_FILE_OK          (u8) 0b01000000
#define SERVER_FILE_FAILED      (u8) 0b10000000

#define MESSAGE_FILE_INFO "name: %s size: %s"

#define COMMAND(command) command
#define COMMAND_HELP COMMAND("help")
#define COMMAND_SHORT_HELP COMMAND("h")
#define COMMAND_DISCONNECT COMMAND("disconnect")
#define COMMAND_SHORT_DISCONNECT COMMAND("d")
#define COMMAND_LIST COMMAND("list")
#define COMMAND_SHORT_LIST COMMAND("ls")
#define COMMAND_CREATE_DIR COMMAND("mkdir")
#define COMMAND_SHORT_CREATE_DIR COMMAND("mk")
#define COMMAND_REMOVE_DIR COMMAND("rmdir")
#define COMMAND_SHORT_REMOVE_DIR COMMAND(" ")
#define COMMAND_SEND_FILE COMMAND("send")
#define COMMAND_SHORT_SEND_FILE COMMAND("s")
#define COMMAND_REMOVE_FILE COMMAND(" ")
#define COMMAND_SHORT_REMOVE_FILE COMMAND("rm")

#define COMMAND_OPTION_RECURSIVE "-r"

#define MESSAGE_WELCOME NSTR("Welcome to the server")
#define MESSAGE_NO_ACCESS NSTR("No Access")
#define MESSAGE_NO_DIRECTORIES NSTR("Couldn't open the directory")
#define MESSAGE_DIR_NO_PATH COLORIZED(FLRED, NSTR("[-] Missing PATH"))
#define MESSAGE_CREATE_DIR_SUCCESS COLORIZED(FLGREEN, NSTR("[+] Directory created: %s"))
#define MESSAGE_REMOVE_DIR_SUCCESS COLORIZED(FLMAGENTA, NSTR("[-] Directory removed: %s"))
#define MESSAGE_SEND_FILE_SUCCESS COLORIZED(FLGREEN, NSTR("[+] File created: %s"))
#define MESSAGE_REMOVE_FILE_SUCCESS COLORIZED(FLMAGENTA, NSTR("[-] File removed: %s"))
#define MESSAGE_LIST_DIRECTORY NSTR("List:")
// TODO: align from '-'
#define MESSAGE_HELP NSTR("Commands:") \
    NSTR(COMMAND_SHORT_HELP        "  " COMMAND_HELP          " \t - shows this message"               )\
    NSTR(COMMAND_SHORT_DISCONNECT  "  " COMMAND_DISCONNECT    " \t - disconnect from the server"       )\
    NSTR(COMMAND_SHORT_LIST        " " COMMAND_LIST           " \t - list directory content"           )\
    NSTR(COMMAND_SHORT_CREATE_DIR  " " COMMAND_CREATE_DIR     " PATH \t - create directory"            )\
    NSTR(COMMAND_SHORT_REMOVE_DIR  "  " COMMAND_REMOVE_DIR    " PATH \t - remove directory"            )\
    NSTR(                                                     "\t\t [-r] for recursive remove"         )\
    NSTR(COMMAND_SHORT_SEND_FILE   "  " COMMAND_SEND_FILE     " FILE \t - to send a file"              )\
    NSTR(COMMAND_SHORT_REMOVE_FILE "    " COMMAND_REMOVE_FILE " PATH \t - remove a directory or file"  )

typedef struct dirent dirent;

typedef struct {
  u16 port;
  char *root_path;
} arguments_t;

typedef struct {
  int total;
  char *root_path;

  dirent **directories;
} server_info_t;

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
  arguments_t *arguments = (arguments_t *) state->input;
  switch (key) {
    case 'p': {
      if (arg == NULL)
        return 0;

      uintmax_t port = strtoumax(arg, NULL, 10);

      if (port == UINTMAX_MAX && errno == ERANGE) {
        return 0;
      }

      arguments->port = (u16) port;
    }
      break;
    case ARGP_KEY_ARG: {
      free(arguments->root_path);

      char *path = realpath(arg, NULL);
      if (path)
        arguments->root_path = path;
    }
      break;
    default: return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

static const char doc[] = DOCUMENTATION("this is a simple ftp server.");
static const char args_doc[] = DOCUMENTATION("PATH");
static const struct argp_option argument_options[] = {
    {.name = "port", .key = 'p', .arg = "PORT", .flags = OPTION_ARG_OPTIONAL, .doc = "Change the ftp port. Default: " STR(
        DEFAULT_PORT)},
    {0},
};

static struct argp argp = {argument_options, parse_opt, args_doc, doc, 0, 0, 0};

int server_handle_connection(server_info_t *info, int socket);
static int server_send_check(int socket, char *writeBuffer, int code, const char *message);
void server_info_clear(server_info_t *info);
void server_path_scan(server_info_t *serverInfo);

int server_dir_remove_depth_cb(const char *fpath, const struct stat *sb, int typeFlag, struct FTW *ftw);

int server_rmrf(char *path);

#endif //FTP_SERVER_H
