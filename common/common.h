//
// Created by felipemanzur on 22/10/22.
//

#ifndef FTP_COMMON_H
#define FTP_COMMON_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <inttypes.h>

#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <strings.h>

#pragma push_macro("__USE_XOPEN_EXTENDED")
#undef __USE_XOPEN_EXTENDED
#define __USE_XOPEN_EXTENDED 700
#include <ftw.h>
#pragma pop_macro("__USE_XOPEN_EXTENDED")

#include <sys/socket.h>
#include <arpa/inet.h>

// parse args
#include <argp.h>

#include "types.h"
#include "tokenizer.h"

#define SOCKET_ERROR (-1)
#define BUFSIZE 4096

#define LOCALHOST "127.0.0.1"
#define DEFAULT_IP LOCALHOST
#define DEFAULT_PORT 21
#define DEFAULT_PATH "./"

#define UNUSED(x) (void)x

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define NSTR(x) x"\n"

#define VERSION(name, version) STR(name)" "STR(version)
#define DOCUMENTATION(text) text

// Terminal Colors
// F = Foreground - B = background
// L = Light - M = MODIFIERS
#define MDEFAULT "\033[0m"
#define MBOLD "\033[1m"
#define MUNDERLINE "\033[4m"
#define MNOUNDERLINE "\033[24m"
#define MNEGATIVE "\033[7m"
#define MPOSITVE "\033[27m"
#define FBLACK "\033[30m"
#define FRED "\033[31m"
#define FGREEN "\033[32m"
#define FYELLOW "\033[33m"
#define FBLUE "\033[34m"
#define FMAGENTA "\033[35m"
#define FCYAN "\033[36m"
#define FLRED "\033[91m"
#define FLGREEN "\033[92m"
#define FLYELLOW "\033[93m"
#define FLBLUE "\033[94m"
#define FLMAGENTA "\033[95m"
#define FLCYAN "\033[96m"
#define FWHITE "\033[97m"

#define COLORIZED(color, text) color text MDEFAULT

static int socket_check(int code, const char *message) {
    if (code == SOCKET_ERROR) {
        printf(COLORIZED(FRED, "[-] Socket Error: %s\n\t%s\n"), strerror(errno), message);
        return errno;
    }

    return 0;
}

#endif //FTP_COMMON_H
