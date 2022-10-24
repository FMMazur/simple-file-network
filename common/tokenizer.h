//
// Created by felipemanzur on 22/10/22.
//

#ifndef FTP_TOKENIZER_H
#define FTP_TOKENIZER_H

#include <stdlib.h>
#include <string.h>

#include "types.h"

typedef struct token_node_s {
    char *string;
    struct token_node_s* next;
} token_node_t;

typedef struct token_s {
    u32 count;

    const char* input;

    token_node_t* head;
    token_node_t* current;
} token_t;

token_t* tokenizer(const char* string, const char* delimiters);

char* token_get_string(token_t* token);
void token_reset(token_t* token);

void token_free(token_t* token);

#if defined(FTP_TOKENIZER_INPLACE)
token_t *tokenizer(const char *string, const char* delimiters) {
    token_t* token = malloc(sizeof (token_t));

    if (token) {
        token->input = string;
        token->count = 0;
        token->head = NULL;

        while(*token->input != '\0') {
            const char* begin = token->input;
            const char* end = NULL;

            token->input += strspn (token->input, delimiters);
            if (*token->input == '\0')
            {
                token->head = NULL;
            }

            end = token->input + strcspn (token->input, delimiters);
            token->input = end + 1;

            size_t strSize = end - begin;
            token_node_t* node = malloc(sizeof(token_node_t));

            if (node) {
                node->next = NULL;
                node->string = malloc(strSize + 1);
                strncpy(node->string, begin, strSize);
                node->string[strSize] = '\0';

                if (!token->head) {
                    token->head = node;
                }

                if (!token->current){
                    token->current = node;
                } else {
                    token->current->next = node;
                    token->current = node;
                }

                token->count++;
            }
        }

        token->current = token->head;
    }

    return token;
}

void token_reset(token_t* token) {
    token->current = token->head;
}

char* token_get_string(token_t* token) {
    if (!token) return NULL;
    if (!token->current) return NULL;

    char* string = token->current->string;
    token->current = token->current->next;

    return string;
}

void token_free(token_t **token) {
    if (!token || !(*token)) return;

    token_node_t* head = (*token)->head, *tmp;

    while(head) {
        tmp = head->next;

        if (head->string)
            free(head->string);

        free(head);
        head = tmp;
    }

    //token->input = NULL;
    //token->current = NULL;
    //token->head = NULL;

    free(*token);
}

#endif

#endif //FTP_TOKENIZER_H
