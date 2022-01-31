//
// Created by ivanbrekman on 04.10.2021.
//

#ifndef SIMPLEPROCESSOR_CONFIG_H
#define SIMPLEPROCESSOR_CONFIG_H

// Header in binary files to check them state
struct header {
    char signature[3] = "IB";
    char version[10]  = "1.5.8";
};
const header HEADER = { };

#define MAX_ARG_SIZE      50    /* How much symbols can be in one argument definition */

#define LOG_PROCESSOR_STATE 1   /* Output state of processor while executing commands (0 or 1) */
#define SKIP_EMPTY_STRINGS  1   /* Skip empty strings in source file (0 or 1) */
/* Skip spaces before first and after last not spaces symbol.
    from "       text, some text,,,, !    "
    to   "text, some text,,,, !"
*/
#define SKIP_FISRT_LAST_SPACES  1

#define COMMENT_SYMBOL '#'  /* Symbol which be recognized in source_file as start of comment */
#define SLEEP    1          /* Seconds to sleep after each command */
#define ACCURACY 0          /* Dergee of 10, which be multiplies in IN command and devides in OUTV command */

#endif //SIMPLEPROCESSOR_CONFIG_H
