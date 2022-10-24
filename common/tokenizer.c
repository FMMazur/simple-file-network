//
// Created by felipemanzur on 23/10/22.
//

#include "tokenizer.h"

token_t *tokenizer(const char *string, const char* delimiters) {
    token_t* token = (token_t*) calloc(1, sizeof (token_t));

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
    token->currentIdx = 0;
}

char* token_get_string(token_t* token) {
    if (!token) return NULL;
    if (!token->current) return NULL;

    char* string = token->current->string;
    token->current = token->current->next;
    token->currentIdx++;

    return string;
}

void token_free(token_t *token) {
    if (!token) return;

    token_node_t* head = token->head;
    token_node_t * tmp;

    while(head) {
        tmp = head->next;

        if (head->string) {
            free(head->string);
            head->string = NULL;
        }

        free(head);
        head = tmp;
    }

    token->input = NULL;
    token->current = NULL;
    token->head = NULL;

    free(token);
}
