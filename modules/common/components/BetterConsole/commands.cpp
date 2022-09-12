// Copyright 2016-2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include "esp_log.h"
#include "better_console.hpp"
#include "esp_system.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "sys/queue.h"

#define ANSI_COLOR_DEFAULT 39 /** Default foreground color */

#define SS_FLAG_ESCAPE 0x8

typedef enum
{
    /* parsing the space between arguments */
    SS_SPACE = 0x0,
    /* parsing an argument which isn't quoted */
    SS_ARG = 0x1,
    /* parsing a quoted argument */
    SS_QUOTED_ARG = 0x2,
    /* parsing an escape sequence within unquoted argument */
    SS_ARG_ESCAPED = SS_ARG | SS_FLAG_ESCAPE,
    /* parsing an escape sequence within a quoted argument */
    SS_QUOTED_ARG_ESCAPED = SS_QUOTED_ARG | SS_FLAG_ESCAPE,
} split_state_t;

/* helper macro, called when done with an argument */
#define END_ARG()                      \
    do                                 \
    {                                  \
        char_out = 0;                  \
        argv[argc++] = next_arg_start; \
        state = SS_SPACE;              \
    } while (0)

typedef struct cmd_item_
{
    /**
     * Command name (statically allocated by application)
     */
    const char *command;
    /**
     * Help text (statically allocated by application), may be NULL.
     */
    const char *help;
    /**
     * Hint text, usually lists possible arguments, dynamically allocated.
     * May be NULL.
     */
    char *hint;
    better_console_cmd_func_t func; //!< pointer to the command handler
    void *argtable;                 //!< optional pointer to arg table
    SLIST_ENTRY(cmd_item_)
    next; //!< next command in the list
} cmd_item_t;

/** linked list of command structures */
static SLIST_HEAD(cmd_list_, cmd_item_) s_cmd_list;

/** run-time configuration options */
static better_console_config_t s_config;

/** temporary buffer used for command line parsing */
static char *s_tmp_line_buf;

static const cmd_item_t *find_command_by_name(const char *name);

esp_err_t better_console_init(const better_console_config_t *config)
{
    if (!config)
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_tmp_line_buf)
    {
        return ESP_ERR_INVALID_STATE;
    }
    memcpy(&s_config, config, sizeof(s_config));
    if (s_config.hint_color == 0)
    {
        s_config.hint_color = ANSI_COLOR_DEFAULT;
    }
    s_tmp_line_buf = (char *)calloc(config->max_cmdline_length, 1);
    if (s_tmp_line_buf == NULL)
    {
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

esp_err_t better_console_deinit(void)
{
    if (!s_tmp_line_buf)
    {
        return ESP_ERR_INVALID_STATE;
    }
    free(s_tmp_line_buf);
    s_tmp_line_buf = NULL;
    cmd_item_t *it, *tmp;
    SLIST_FOREACH_SAFE(it, &s_cmd_list, next, tmp)
    {
        SLIST_REMOVE(&s_cmd_list, it, cmd_item_, next);
        free(it->hint);
        free(it);
    }
    return ESP_OK;
}

esp_err_t better_console_cmd_register(const better_console_cmd_t *cmd)
{
    cmd_item_t *item = NULL;
    if (!cmd || cmd->command == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (strchr(cmd->command, ' ') != NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }
    item = (cmd_item_t *)find_command_by_name(cmd->command);
    if (!item)
    {
        // not registered before
        item = (cmd_item_t *)calloc(1, sizeof(*item));
        if (item == NULL)
        {
            return ESP_ERR_NO_MEM;
        }
    }
    else
    {
        // remove from list and free the old hint, because we will alloc new hint for the command
        SLIST_REMOVE(&s_cmd_list, item, cmd_item_, next);
        free(item->hint);
    }
    item->command = cmd->command;
    item->help = cmd->help;
    if (cmd->hint)
    {
        /* Prepend a space before the hint. It separates command name and
         * the hint. arg_print_syntax below adds this space as well.
         */
        int unused __attribute__((unused));
        unused = asprintf(&item->hint, " %s", cmd->hint);
    }
    else if (cmd->argtable)
    {
        /* Generate hint based on cmd->argtable */
        char *buf = NULL;
        size_t buf_size = 0;
        FILE *f = open_memstream(&buf, &buf_size);
        if (f != NULL)
        {
            arg_print_syntax(f, (void **)cmd->argtable, NULL);
            fclose(f);
        }
        item->hint = buf;
    }
    item->argtable = cmd->argtable;
    item->func = cmd->func;
    cmd_item_t *last = SLIST_FIRST(&s_cmd_list);
    if (last == NULL)
    {
        SLIST_INSERT_HEAD(&s_cmd_list, item, next);
    }
    else
    {
        cmd_item_t *it;
        while ((it = SLIST_NEXT(last, next)) != NULL)
        {
            last = it;
        }
        SLIST_INSERT_AFTER(last, item, next);
    }
    return ESP_OK;
}

void better_console_get_completion(const char *buf, linenoiseCompletions *lc)
{
    size_t len = strlen(buf);
    if (len == 0)
    {
        return;
    }
    cmd_item_t *it;
    SLIST_FOREACH(it, &s_cmd_list, next)
    {
        /* Check if command starts with buf */
        if (strncmp(buf, it->command, len) == 0)
        {
            linenoiseAddCompletion(lc, it->command);
        }
    }
}

const char *better_console_get_hint(const char *buf, int *color, int *bold)
{
    size_t len = strlen(buf);
    cmd_item_t *it;
    SLIST_FOREACH(it, &s_cmd_list, next)
    {
        if (strlen(it->command) == len &&
            strncmp(buf, it->command, len) == 0)
        {
            *color = s_config.hint_color;
            *bold = s_config.hint_bold;
            return it->hint;
        }
    }
    return NULL;
}

static const cmd_item_t *find_command_by_name(const char *name)
{
    const cmd_item_t *cmd = NULL;
    cmd_item_t *it;
    size_t len = strlen(name);
    SLIST_FOREACH(it, &s_cmd_list, next)
    {
        if (strlen(it->command) == len &&
            strcmp(name, it->command) == 0)
        {
            cmd = it;
            break;
        }
    }
    return cmd;
}

esp_err_t better_console_run(const char *cmdline, std::string *cmd_ret)
{
    if (s_tmp_line_buf == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }
    char **argv = (char **)calloc(s_config.max_cmdline_args, sizeof(char *));
    if (argv == NULL)
    {
        return ESP_ERR_NO_MEM;
    }
    strlcpy(s_tmp_line_buf, cmdline, s_config.max_cmdline_length);

    size_t argc = better_console_split_argv(s_tmp_line_buf, argv,
                                            s_config.max_cmdline_args);
    if (argc == 0)
    {
        free(argv);
        return ESP_ERR_INVALID_ARG;
    }
    const cmd_item_t *cmd = find_command_by_name(argv[0]);
    if (cmd == NULL)
    {
        free(argv);
        return ESP_ERR_NOT_FOUND;
    }

    ESP_LOGD("BetterConsole", "Executando comando '%s' e armazenando retorno.", cmd->command);
    (*cmd_ret) = (*cmd->func)(argc, argv);
    ESP_LOGD("BetterConsole", "Comando executado e retorno armazenado.");

    free(argv);
    return ESP_OK;
}

static std::string help_command(int argc, char **argv)
{
    cmd_item_t *it;

    /* Print summary of each command */
    SLIST_FOREACH(it, &s_cmd_list, next)
    {
        if (it->help == NULL)
        {
            continue;
        }
        /* First line: command name and hint
         * Pad all the hints to the same column
         */
        const char *hint = (it->hint) ? it->hint : "";
        printf("%-s %s\n", it->command, hint);
        /* Second line: print help.
         * Argtable has a nice helper function for this which does line
         * wrapping.
         */
        printf("  "); // arg_print_formatted does not indent the first line
        arg_print_formatted(stdout, 2, 78, it->help);
        /* Finally, print the list of arguments */
        if (it->argtable)
        {
            arg_print_glossary(stdout, (void **)it->argtable, "  %12s  %s\n");
        }
        printf("\n");
    }
    return "OK";
}

esp_err_t better_console_register_help_command(void)
{
    better_console_cmd_t command = {
        .command = "help",
        .help = "Print the list of registered commands",
        .hint = NULL,
        .func = &help_command,
        .argtable = NULL,
    };
    return better_console_cmd_register(&command);
}

size_t better_console_split_argv(char *line, char **argv, size_t argv_size)
{
    const int QUOTE = '"';
    const int ESCAPE = '\\';
    const int SPACE = ' ';
    split_state_t state = SS_SPACE;
    size_t argc = 0;
    char *next_arg_start = line;
    char *out_ptr = line;
    for (char *in_ptr = line; argc < argv_size - 1; ++in_ptr)
    {
        int char_in = (unsigned char)*in_ptr;
        if (char_in == 0)
        {
            break;
        }
        int char_out = -1;

        switch (state)
        {
        case SS_SPACE:
            if (char_in == SPACE)
            {
                /* skip space */
            }
            else if (char_in == QUOTE)
            {
                next_arg_start = out_ptr;
                state = SS_QUOTED_ARG;
            }
            else if (char_in == ESCAPE)
            {
                next_arg_start = out_ptr;
                state = SS_ARG_ESCAPED;
            }
            else
            {
                next_arg_start = out_ptr;
                state = SS_ARG;
                char_out = char_in;
            }
            break;

        case SS_QUOTED_ARG:
            if (char_in == QUOTE)
            {
                END_ARG();
            }
            else if (char_in == ESCAPE)
            {
                state = SS_QUOTED_ARG_ESCAPED;
            }
            else
            {
                char_out = char_in;
            }
            break;

        case SS_ARG_ESCAPED:
        case SS_QUOTED_ARG_ESCAPED:
            if (char_in == ESCAPE || char_in == QUOTE || char_in == SPACE)
            {
                char_out = char_in;
            }
            else
            {
                /* unrecognized escape character, skip */
            }
            state = (split_state_t)(state & (~SS_FLAG_ESCAPE));
            break;

        case SS_ARG:
            if (char_in == SPACE)
            {
                END_ARG();
            }
            else if (char_in == ESCAPE)
            {
                state = SS_ARG_ESCAPED;
            }
            else
            {
                char_out = char_in;
            }
            break;
        }
        /* need to output anything? */
        if (char_out >= 0)
        {
            *out_ptr = char_out;
            ++out_ptr;
        }
    }
    /* make sure the final argument is terminated */
    *out_ptr = 0;
    /* finalize the last argument */
    if (state != SS_SPACE && argc < argv_size - 1)
    {
        argv[argc++] = next_arg_start;
    }
    /* add a NULL at the end of argv */
    argv[argc] = NULL;

    return argc;
}
