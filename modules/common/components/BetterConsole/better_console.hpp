#ifndef BETTER_CONSOLE_HPP
#define BETTER_CONSOLE_HPP

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

#include <string>
#include <stddef.h>
#include "sdkconfig.h"
#include "esp_err.h"

#include "esp_log.h"

// Forward declaration. Definition in linenoise/linenoise.h.
typedef struct linenoiseCompletions linenoiseCompletions;

/**
 * @brief Parameters for console initialization
 */
typedef struct
{
    size_t max_cmdline_length; //!< length of command line buffer, in bytes
    size_t max_cmdline_args;   //!< maximum number of command line arguments to parse
    int hint_color;            //!< ASCII color code of hint text
    int hint_bold;             //!< Set to 1 to print hint text in bold
} better_console_config_t;

/**
 * @brief Default console configuration value
 *
 */
#define BETTER_CONSOLE_CONFIG_DEFAULT() \
    {                                   \
        .max_cmdline_length = 256,      \
        .max_cmdline_args = 32,         \
        .hint_color = 39,               \
        .hint_bold = 0                  \
    }

/**
 * @brief Parameters for console device: UART
 *
 */
typedef struct
{
    int channel;     //!< UART channel number (count from zero)
    int baud_rate;   //!< Comunication baud rate
    int tx_gpio_num; //!< GPIO number for TX path, -1 means using default one
    int rx_gpio_num; //!< GPIO number for RX path, -1 means using default one
} better_console_dev_uart_config_t;

#ifdef CONFIG_BETTER_CONSOLE_UART_CUSTOM
#define BETTER_CONSOLE_DEV_UART_CONFIG_DEFAULT()           \
    {                                                      \
        .channel = CONFIG_BETTER_CONSOLE_UART_NUM,         \
        .baud_rate = CONFIG_BETTER_CONSOLE_UART_BAUDRATE,  \
        .tx_gpio_num = CONFIG_BETTER_CONSOLE_UART_TX_GPIO, \
        .rx_gpio_num = CONFIG_BETTER_CONSOLE_UART_RX_GPIO, \
    }
#else
#define BETTER_CONSOLE_DEV_UART_CONFIG_DEFAULT()          \
    {                                                     \
        .channel = CONFIG_BETTER_CONSOLE_UART_NUM,        \
        .baud_rate = CONFIG_BETTER_CONSOLE_UART_BAUDRATE, \
        .tx_gpio_num = -1,                                \
        .rx_gpio_num = -1,                                \
    }
#endif

/**
 * @brief Parameters for console device: USB CDC
 *
 * @note It's an empty structure for now, reserved for future
 *
 */
typedef struct
{

} better_console_dev_usb_cdc_config_t;

#define BETTER_CONSOLE_DEV_CDC_CONFIG_DEFAULT() \
    {                                           \
    }

/**
 * @brief initialize console module
 * @param config console configuration
 * @note  Call this once before using other console module features
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_NO_MEM if out of memory
 *      - ESP_ERR_INVALID_STATE if already initialized
 *      - ESP_ERR_INVALID_ARG if the configuration is invalid
 */
esp_err_t better_console_init(const better_console_config_t *config);

/**
 * @brief de-initialize console module
 * @note  Call this once when done using console module functions
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE if not initialized yet
 */
esp_err_t better_console_deinit(void);

/**
 * @brief Console command main function
 * @param argc number of arguments
 * @param argv array with argc entries, each pointing to a zero-terminated string argument
 * @return console command return code, 0 indicates "success"
 */
typedef std::string (*better_console_cmd_func_t)(int argc, char **argv);

/**
 * @brief Console command description
 */
typedef struct
{
    /**
     * Command name. Must not be NULL, must not contain spaces.
     * The pointer must be valid until the call to better_console_deinit.
     */
    const char *command;
    /**
     * Help text for the command, shown by help command.
     * If set, the pointer must be valid until the call to better_console_deinit.
     * If not set, the command will not be listed in 'help' output.
     */
    const char *help;
    /**
     * Hint text, usually lists possible arguments.
     * If set to NULL, and 'argtable' field is non-NULL, hint will be generated
     * automatically
     */
    const char *hint;
    /**
     * Pointer to a function which implements the command.
     */
    better_console_cmd_func_t func;
    /**
     * Array or structure of pointers to arg_xxx structures, may be NULL.
     * Used to generate hint text if 'hint' is set to NULL.
     * Array/structure which this field points to must end with an arg_end.
     * Only used for the duration of better_console_cmd_register call.
     */
    void *argtable;
} better_console_cmd_t;

/**
 * @brief Register console command
 * @param cmd pointer to the command description; can point to a temporary value
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_NO_MEM if out of memory
 *      - ESP_ERR_INVALID_ARG if command description includes invalid arguments
 */
esp_err_t better_console_cmd_register(const better_console_cmd_t *cmd);

/**
 * @brief Run command line
 * @param cmdline command line (command name followed by a number of arguments)
 * @param[out] cmd_ret return code from the command (set if command was run)
 * @return
 *      - ESP_OK, if command was run
 *      - ESP_ERR_INVALID_ARG, if the command line is empty, or only contained
 *        whitespace
 *      - ESP_ERR_NOT_FOUND, if command with given name wasn't registered
 *      - ESP_ERR_INVALID_STATE, if better_console_init wasn't called
 */
esp_err_t better_console_run(const char *cmdline, std::string *cmd_ret);

/**
 * @brief Split command line into arguments in place
 * @verbatim
 * - This function finds whitespace-separated arguments in the given input line.
 *
 *     'abc def 1 20 .3' -> [ 'abc', 'def', '1', '20', '.3' ]
 *
 * - Argument which include spaces may be surrounded with quotes. In this case
 *   spaces are preserved and quotes are stripped.
 *
 *     'abc "123 456" def' -> [ 'abc', '123 456', 'def' ]
 *
 * - Escape sequences may be used to produce backslash, double quote, and space:
 *
 *     'a\ b\\c\"' -> [ 'a b\c"' ]
 * @endverbatim
 * @note Pointers to at most argv_size - 1 arguments are returned in argv array.
 *       The pointer after the last one (i.e. argv[argc]) is set to NULL.
 *
 * @param line pointer to buffer to parse; it is modified in place
 * @param argv array where the pointers to arguments are written
 * @param argv_size number of elements in argv_array (max. number of arguments)
 * @return number of arguments found (argc)
 */
size_t better_console_split_argv(char *line, char **argv, size_t argv_size);

/**
 * @brief Callback which provides command completion for linenoise library
 *
 * When using linenoise for line editing, command completion support
 * can be enabled like this:
 *
 *   linenoiseSetCompletionCallback(&better_console_get_completion);
 *
 * @param buf the string typed by the user
 * @param lc linenoiseCompletions to be filled in
 */
void better_console_get_completion(const char *buf, linenoiseCompletions *lc);

/**
 * @brief Callback which provides command hints for linenoise library
 *
 * When using linenoise for line editing, hints support can be enabled as
 * follows:
 *
 *   linenoiseSetHintsCallback((linenoiseHintsCallback*) &better_console_get_hint);
 *
 * The extra cast is needed because linenoiseHintsCallback is defined as
 * returning a char* instead of const char*.
 *
 * @param buf line typed by the user
 * @param[out] color ANSI color code to be used when displaying the hint
 * @param[out] bold set to 1 if hint has to be displayed in bold
 * @return string containing the hint text. This string is persistent and should
 *         not be freed (i.e. linenoiseSetFreeHintsCallback should not be used).
 */
const char *better_console_get_hint(const char *buf, int *color, int *bold);

/**
 * @brief Register a 'help' command
 *
 * Default 'help' command prints the list of registered commands along with
 * hints and help strings.
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE, if better_console_init wasn't called
 */
esp_err_t better_console_register_help_command(void);
#endif