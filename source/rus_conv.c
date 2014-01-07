/**  rus_conv.c  **/
#include "micq.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
typedef unsigned char uchar;

#ifdef OS2
/* Ibm866 to win1251 translation */
uchar kw[] = {0x0C0, 0x0C1, 0x0C2, 0x0C3, 0x0C4, 0x0C5, 0x0C6, 0x0C7,
              0x0C8, 0x0C9, 0x0CA, 0x0CB, 0x0CC, 0x0CD, 0x0CE, 0x0CF,
              0x0D0, 0x0D1, 0x0D2, 0x0D3, 0x0D4, 0x0D5, 0x0D6, 0x0D7,
              0x0D8, 0x0D9, 0x0DA, 0x0DB, 0x0DC, 0x0DD, 0x0DE, 0x0DF,
              0x0E0, 0x0E1, 0x0E2, 0x0E3, 0x0E4, 0x0E5, 0x0E6, 0x0E7,
              0x0E8, 0x0E9, 0x0EA, 0x0EB, 0x0EC, 0x0ED, 0x0EE, 0x0EF,
              0x023, 0x023, 0x023, 0x07C, 0x07C, 0x07C, 0x07C, 0x02B,
              0x02B, 0x02B, 0x07C, 0x07C, 0x02B, 0x02B, 0x02B, 0x02B,
              0x02B, 0x097, 0x097, 0x07C, 0x097, 0x02B, 0x07C, 0x07C,
              0x02B, 0x02B, 0x097, 0x097, 0x07C, 0x097, 0x02B, 0x097,
              0x097, 0x097, 0x097, 0x02B, 0x02B, 0x02B, 0x02B, 0x02B,
              0x02B, 0x02B, 0x02B, 0x088, 0x088, 0x088, 0x088, 0x088,
              0x0F0, 0x0F1, 0x0F2, 0x0F3, 0x0F4, 0x0F5, 0x0F6, 0x0F7,
              0x0F8, 0x0F9, 0x0FA, 0x0FB, 0x0FC, 0x0FD, 0x0FE, 0x0FF,
              0x0A8, 0x0B8, 0x088, 0x088, 0x088, 0x088, 0x088, 0x088,
              0x0B0, 0x0B7, 0x095, 0x088, 0x0A9, 0x0A4, 0x088, 0x020};
/* win1251 to ibm866 translation */
uchar wk[] = {0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023,
              0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023,
              0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023,
              0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023,
              0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023,
              0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023,
              0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023,
              0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023, 0x023,
              0x080, 0x081, 0x082, 0x083, 0x084, 0x085, 0x086, 0x087,
              0x088, 0x089, 0x08A, 0x08B, 0x08C, 0x048, 0x08E, 0x08F,
              0x090, 0x091, 0x092, 0x093, 0x094, 0x095, 0x096, 0x097,
              0x098, 0x099, 0x09A, 0x09B, 0x09C, 0x09D, 0x09E, 0x09F,
              0x0a0, 0x0a1, 0x0a2, 0x0a3, 0x0a4, 0x0a5, 0x0a6, 0x0a7,
              0x0a8, 0x0a9, 0x0aA, 0x0aB, 0x0aC, 0x0aD, 0x0aE, 0x0aF,
              0x0e0, 0x0e1, 0x0e2, 0x0e3, 0x0e4, 0x0e5, 0x0e6, 0x0e7,
              0x0e8, 0x0e9, 0x0eA, 0x0eB, 0x0eC, 0x0eD, 0x0eE, 0x0eF};
#else
uchar kw[] = {128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
              144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
              160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
              176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
              254,224,225,246,228,229,244,227,245,232,233,234,235,236,237,238,
              239,255,240,241,242,243,230,226,252,251,231,248,253,249,247,250,
              222,192,193,214,196,197,212,195,213,200,201,202,203,204,205,206,
              207,223,208,209,210,211,198,194,220,219,199,216,221,217,215,218};
uchar wk[] = {128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
              144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
              160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
              176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
              225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,
              242,243,244,245,230,232,227,254,251,253,255,249,248,252,224,241,
              193,194,215,199,196,197,214,218,201,202,203,204,205,206,207,208,
              210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,209};
#endif
/********************************************************
Russian language ICQ fix.
Usual Windows ICQ users do use Windows 1251 encoding but
unix users do use koi8 encoding, so we need to convert it.
This function will convert string from windows 1251 to koi8
or from koi8 to windows 1251.
Andrew Frolov dron@ilm.net
*********************************************************/
void rus_conv( char to[4], char *t_in )
{
   uchar *table;
   int i;

/* The following code block is broken. Fixed below */
/* 
   if(!strcmp(to, "wk"))
      table = wk;
   else if(!strcmp(to, "kw"))
      table = kw;
   else
      printf ("error - unknown option\n");
*/
/* 6-17-1998 by Linux_Dude
 * Moved initialization of table out front of 'if' block to prevent compiler
 * warning. Improved error message, and now return without performing string
 * conversion to prevent addressing memory out of range (table pointer would
 * previously have remained uninitialized ( = bad)).
*/
   table = wk;
   if(strcmp(to, "kw") == 0)
      table = kw;
   else if(strcmp(to, "wk") != 0)
   {
      fprintf(stderr, "Error - \"to\" in call to rus_conv() is \"%s\", must " \
                      "be \"wk\" or \"kw\".\nWarning - Sending message " \
                      "without converting string.\n", to);
      return;
   }
      
/* End Linux_Dude's changes ;)
*/
   if ( Russian )
   {
      for (i=0;t_in[i]!=0;i++)
      {
         t_in[i] &= 0377;
         if(t_in[i] & 0200)
            t_in[i] = table[t_in[i] & 0177];
      }
   }
}
