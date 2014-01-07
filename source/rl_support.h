#ifndef __RL_SUPPORT_FOR_MICQ__
#define __RL_SUPPORT_FOR_MICQ__

/*
 * Remove the next line if you're compiling this against an installed
 * libreadline.a
 */
#define READLINE_LIBRARY

#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#include "readline.h"
#include "history.h"


extern HIST_ENTRY **history_list ();

extern int screenwidth, screenheight, screenchars;
extern int _rl_convert_meta_chars_to_ascii;
extern int _rl_output_meta_chars;
extern int _rl_meta_flag;


void init_prompt();
void rl_cmd_handler(char * line);










#endif

