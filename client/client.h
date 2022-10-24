//
// Created by felipemazur on 22/10/22.
//

#ifndef FTP_CLIENT_H
#define FTP_CLIENT_H

#include "common.h"

typedef struct in_addr in_addr;

typedef struct {
    in_addr ip;
    u16 port;
} arguments_t;

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    arguments_t *arguments = (arguments_t*) state->input;
    switch (key) {
        case 'h': {
            if (arg == NULL)
                return 0;


            in_addr ip;
            bool success = inet_aton(arg, &ip);

            if (success) {
                arguments->ip.s_addr = ip.s_addr;
            }
        }
        break;
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
        case ARGP_KEY_NO_ARGS:
        case ARGP_KEY_ARGS:
            argp_usage (state);
            break;
        default: return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static const char doc[] = DOCUMENTATION("this is a simple ftp client.");
static const char args_doc[] = DOCUMENTATION("");
static const struct argp_option argument_options[] = {
        { .name = "host", .key = 'h', .arg = "HOST", .flags = OPTION_ARG_OPTIONAL, .doc = "Change the ftp host server. Default: " STR(DEFAULT_IP) },
        { .name = "port", .key = 'p', .arg = "PORT", .flags = OPTION_ARG_OPTIONAL, .doc = "Change the ftp port. Default: " STR(DEFAULT_PORT) },
        { 0 },
};

static struct argp argp = { argument_options, parse_opt, args_doc, doc, 0, 0, 0 };

int client_handle_connection(int socket);

#endif //FTP_CLIENT_H
