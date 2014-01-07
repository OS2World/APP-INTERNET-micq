#include "micq.h"
#include "rl_support.h"

char ModePrompt[3][45];

void init_prompt()
{
/*   sprintf(ModePrompt[0],"%s%s%s",SERVCOL,"Micq> ",NOCOL); */
/*   sprintf(ModePrompt[1],"%s%s%s",SERVCOL,"msg> ",NOCOL); */
/*   sprintf(ModePrompt[2],"%s%s%s",SERVCOL,"msgall> ",NOCOL); */
  /*   Readline lib has problems with control sequences in Propmts :-( */
  sprintf(ModePrompt[0],"%s","Micq> ");
  sprintf(ModePrompt[1],"%s","msg> ");
  sprintf(ModePrompt[2],"%s","msgall> ");

  /* Also tweak readline */
  _rl_convert_meta_chars_to_ascii = 0;
  _rl_output_meta_chars = 1;
  _rl_meta_flag = 1;
}
void clear_line()
{
  char emptyline[screenwidth+1];
  memset(emptyline,' ',screenwidth);
  emptyline[screenwidth]=0;
  printf("\r%s\r",emptyline);
}

void rl_cmd_handler(char * line)
{
  /* The readline handler writes line to global variable */
  completed_command_line=line;
  add_history(completed_command_line);
  /* job done, remove handler */
  rl_callback_handler_remove(); 
};
