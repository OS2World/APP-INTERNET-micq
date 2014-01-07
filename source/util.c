/*********************************************
**********************************************
This is a file of general utility functions useful
for programming in general and icq in specific
This software is provided AS IS to be used in
whatever way you see fit and is placed in the
public domain.
Author : Matthew Smith April 23, 1998
Contributors :  airog (crabbkw@rose-hulman.edu) May 13, 1998

Changes :
  6-18-98 Added support for saving auto reply messages. Fryslan
  7-19-98 Changed clrscr()
**********************************************
**********************************************/
#include "micq.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#define S_IRUSR		_S_IREAD
#define S_IWUSR		_S_IWRITE
#elif defined (__EMX__)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/select.h>
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif
#ifdef UNIX
#include <unistd.h>
#include <termios.h>
#endif
static char rcfile[256];

#define 	 ADD_COMMAND(a, b)     else if ( ! strcasecmp( tmp, a) )   \
	      {                                                          \
		      strncpy( b,strtok(NULL," \n\t"), 16 );                  \
	      }                                                          

#define 	 ADD_MESS(a, b)     else if ( ! strcasecmp( tmp, a) )   \
	      {                                                          \
		      strncpy( b,strtok(NULL,"\n"), 450 );                  \
	      }                                                          


typedef struct 
{
  const char *name;
  WORD code;
} COUNTRY_CODE;
static COUNTRY_CODE Country_Codes[] = { {"USA",1 },
                                 {"Russia",7 },
                                 {"Australia",61 },
                                 {"Denmark",45 },
                                 {"Sweden",46 },
                                 {"Norway",47 },
                                 {"Canada",107 },
                                 {"Brazil",55 },
                                 {"UK",0x2c },
                                 {"Finland", 358 },
                                 {"Iceland", 354 },
                                 {"Algeria", 213 },
                                 {"American Samoa", 684 },
                                 {"Argentina", 54 },
                                 {"Aruba", 297 },
                                 {"Austria", 43 },
                                 {"Bahrain", 973 },
                                 {"Bangladesh", 880 },
                                 {"Belgium", 32 },
                                 {"Belize", 501 },
                                 {"Bolivia", 591 },
                                 {"Cameroon", 237 },
                                 {"Chile", 56 },
                                 {"China", 86 },
                                 {"Columbia", 57 },
                                 {"Costa Rice", 506 },
                                 {"Croatia", 385 }, /* Observerd */
                                 {"Cyprus", 357 },
                                 {"Czech Republic", 42 },
                                 {"Ecuador", 593 },
                                 {"Egypt", 20 },
                                 {"El Salvador", 503 },
                                 {"Ethiopia", 251 },
                                 {"Fiji", 679 },
                                 {"France", 33 },
                                 {"French Antilles", 596 },
                                 {"French Polynesia", 689 },
                                 {"Gabon", 241 },
                                 {"German", 49 },
                                 {"Ghana", 233 },
                                 {"Greece", 30 },
                                 {"Guadeloupe", 590 },
                                 {"Guam", 671 },
                                 {"Guantanomo Bay", 53 },
                                 {"Guatemala", 502 },
                                 {"Guyana", 592 },
                                 {"Haiti", 509 },
                                 {"Honduras", 504 },
                                 {"Hong Kong", 852 },
                                 {"Hungary", 36 },
                                 {"India", 91 },
                                 {"Indonesia", 62 },
                                 {"Iran", 98 },
                                 {"Iraq", 964 },
                                 {"Ireland", 353 },
                                 {"Israel", 972 },
                                 {"Italy", 39 },
                                 {"Ivory Coast", 225 },
                                 {"Japan", 81 },
                                 {"Jordan", 962 },
                                 {"Kenya", 254 },
                                 {"South Korea", 82 },
                                 {"Kuwait", 965 },
                                 {"Liberia", 231 },
                                 {"Libya", 218 },
                                 {"Liechtenstein", 41 },
                                 {"Luxembourg", 352 },
                                 {"Malawi", 265 },
                                 {"Malaysia", 60 },
                                 {"Mali", 223 },
                                 {"Malta", 356 },
                                 {"Mexico", 52 },
                                 {"Monaco", 33 },
                                 {"Morocco", 212 },
                                 {"Namibia", 264 },
                                 {"Nepal", 977 },
                                 {"Netherlands", 31 },
                                 {"Netherlands Antilles", 599 },
                                 {"New Caledonia", 687 },
                                 {"New Zealand", 64 },
                                 {"Nicaragua", 505 },
                                 {"Nigeria", 234 },
                                 {"Oman", 968 },
                                 {"Pakistan", 92 },
                                 {"Panama", 507 },
                                 {"Papua New Guinea", 675 },
                                 {"Paraguay", 595 },
                                 {"Peru", 51 },
                                 {"Philippines", 63 },
                                 {"Poland", 48 },
                                 {"Portugal", 351 },
                                 {"Qatar", 974 },
                                 {"Romania", 40 },
                                 {"Saipan", 670 },
                                 {"San Marino", 39 },
                                 {"Saudia Arabia", 966 },
                                 {"Saipan", 670 },
                                 {"Senegal", 221},
                                 {"Singapore", 65 },
                                 {"Slovakia", 42 },
                                 {"South Africa", 27 },
                                 {"Spain", 34 },
                                 {"Sri Lanka", 94 },
                                 {"Suriname", 597 },
                                 {"Switzerland", 41 },
                                 {"Taiwan", 886 },
                                 {"Tanzania", 255 },
                                 {"Thailand", 66 },
                                 {"Tunisia", 216 },
                                 {"Turkey", 90 },
                                 {"United Arab Emirates", 971 },
                                 {"Uruguay", 598 },
                                 {"Vatican City", 39 },
                                 {"Venezuela", 58 },
                                 {"Vietnam", 84 },
                                 {"Yemen", 967 },
                                 {"Yugoslavia", 38 },
                                 {"Zaire", 243 },
                                 {"Zimbabwe", 263 },
#ifdef FUNNY_MSGS
                                 {"Illegal alien",0xffff } };
#else
                                 {"Not entered",0xffff } };
#endif


/*********  Leeched from Xicq :) xtrophy@it.dk ***************/
/*********  changed to use escape codes like you should :) ***/
/*********  changed colors ***********************************/
/*********  changed from #define's to char[] *****************/
/*          ESC[attr;attr;...m      Set display attributes;  */
/*                                  see table of attribute values below  */
/*   Display Attributes  */
/*          0           All attributes off (normal white on black)  */
/*          1           High intensity (bold)   */
/*          2           Normal intensity  */
/*          4           Underline (usually effective only on */
/*                      monochrome displays)  */
/*          5           Blinking  */
/*          7           Reverse Video  */
/*          8           Invisible  */
/*          30-37       Set the foreground color:  */
/*                          30=Black   31=Red       32=Green   33=Yellow  */
/*                          34=Blue    35=Magenta   36=Cyan    37=White  */
/*          40-47       Set the background color, same values as above  */
/* 	             but substitute 40 for 30 etc.  */         
/*  Settings are cumulative, so (for example) to set bright red foreground  */
/*  set all attributes off, then set red, then bold, use:  */
/*    echo ^e[0;31;1m */

char SERVCOL[30]=        { "\x1B[0;31m" };
char MESSCOL[30]=        { "\x1B[1;34m" };
char CONTACTCOL[30]=     { "\x1B[1;35m" };
char CLIENTCOL[30]=      { "\x1B[0;32m" };
char NOCOL[30]=          { "\x1B[0m"    };
/* char SERVCOL[]=        { "\x1B[0;1;31;47m"   }; */
/* char MESSCOL[]=        { "\x1B[0;2;34;47;5m" }; */
/* char CONTACTCOL[]=     { "\x1B[0;1;33;44m"   }; */
/* char CLIENTCOL[]=      { "\x1B[0;2;32;47;5m" }; */
/* char NOCOL[]=          { "\x1B[0;2;30;47;5m" }; */



const char *Get_Country_Name( int code )
{
  int i;
  for ( i = 0; Country_Codes[i].code != 0xffff; i++) 
    if ( Country_Codes[i].code == code ) 
      return Country_Codes[i].name;
  if ( Country_Codes[i].code == code )
    return Country_Codes[i].name;
  return NULL;
}

/***************************************************************
Turns keybord echo off for the password
****************************************************************/
S_DWORD Echo_Off( void )
{
#ifdef UNIX
  struct termios attr; /* used for getting and setting terminal
			  attributes */
  /* Now turn off echo */
  if (tcgetattr(STDIN_FILENO, &attr) != 0) return(-1);
  /* Start by getting current attributes.  This call copies
     all of the terminal paramters into attr */
  attr.c_lflag &= ~(ECHO);
  /* Turn off echo flag.  NOTE: We are careful not to modify any
     bits except ECHO */
  if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&attr) != 0) return(-2);
  /* Wait for all of the data to be printed. */
  /* Set all of the terminal parameters from the (slightly)
     modified struct termios */
  /* Discard any characters that have been typed but not yet read */
#endif
  return 0;
}

/***************************************************************
Turns keybord echo back on after the password
****************************************************************/
S_DWORD Echo_On( void )
{
#ifdef UNIX
  struct termios attr; /* used for getting and setting terminal
			  attributes */
  if (tcgetattr(STDIN_FILENO, &attr) != 0) return(-1);
  attr.c_lflag |= ECHO;
  if(tcsetattr(STDIN_FILENO,TCSANOW,&attr) != 0) return(-1);
#endif
  return 0;
}

/**************************************************************
Same as fprintf but for FD_T's
***************************************************************/
void M_fdprint( FD_T fd, char *str, ... )
{
  va_list args;
  int k;
  char buf[2048]; /* this should big enough */
  
  assert( buf != NULL );
  assert( 2048 >= strlen( str ) );
   
  va_start( args, str );
  vsprintf( buf, str, args );
  k = write( fd, buf, strlen( buf ) );
  assert( k == strlen( buf ) );
  va_end( args );
}

/***********************************************************
Reads a line of input from the file descriptor fd into buf
an entire line is read but no more than len bytes are 
actually stored
************************************************************/
int M_fdnreadln( FD_T fd, char *buf, size_t len )
{
  int i,j;
  char tmp;
  assert( buf != NULL );
  assert( len > 0 );
  tmp = 0;
  for ( i=-1; ( tmp != '\n' )  ; ) {
    if  ( ( i < len ) || ( i == -1 ) ) {
      i++;
      j = read( fd, &buf[i], 1 );
      tmp = buf[i];
    }
    else {
      j = read( fd, &tmp, 1 );
    }
    assert( j != -1 );
    if ( j == 0 ) {
      buf[i] =  0;
      return -1;
    }
  }
  if ( i < 1 ) {
    buf[i] = 0;
  }
  else {
    if ( buf[i-1] == '\r' ) {
      buf[i-1] = 0;
    }
    else {
      buf[i] = 0;
    }
  } 
  return 0;
}
/********************************************
returns a string describing the status or
a NULL if no such string exists
*********************************************/
char *Convert_Status_2_Str( int status )
{
  if ( STATUS_OFFLINE == status ) /* this because -1 & 0xFFFF is not -1 */
    return "Offline";
   
  switch ( status & 0xffff )
    {
    case STATUS_ONLINE:
      return "Online";
      break;
    case STATUS_DND:
      return "Do not disturb";
      break;
    case STATUS_AWAY:
      return "Away";
      break;
    case STATUS_OCCUPIED:
      return "Occupied";
      break;
    case STATUS_NA:
      return "Not available";
      break;
    case STATUS_INVISIBLE:
      return "Invisible";
      break;
    case STATUS_INVISIBLE_2:
      return "Invisible mode 2";
      break;
    case STATUS_FREE_CHAT:
      return "Free for chat";
      break;
    default :
      return NULL;
      break;
    }
}
/********************************************
prints out the status of new_status as a string
if possible otherwise as a hex number
*********************************************/
void Print_Status( DWORD new_status  )
{
  if ( Convert_Status_2_Str( new_status ) != NULL ) {
    printf( "%s", Convert_Status_2_Str( new_status ) );
    if ( Verbose )
      printf( " %02X",( WORD ) ( new_status >> 16 ) );
  }
  else {
    printf( "%08lX", new_status );
  }
}
/**********************************************
 * Returns the nick of a UIN if we know it else
 * it will return Unknow UIN
 **********************************************/
char *UIN2nick( DWORD uin)
{
  int i;
	
  for ( i=0; i < Num_Contacts; i++ ) {
    if ( Contacts[i].uin == uin )
      break;
  }
	
  if ( i == Num_Contacts ) {
    return "Unknow UIN";
  }
  else {
    return Contacts[i].nick;
  }
}
/**********************************************
Prints the name of a user or there UIN if name
is not know.
***********************************************/
int Print_UIN_Name( DWORD uin )
{
  int i;
   
  for ( i=0; i < Num_Contacts; i++ ) {
    if ( Contacts[i].uin == uin )
      break;
  }
  if ( i == Num_Contacts ) {
    printf( "%s%lu%s", CLIENTCOL, uin, NOCOL );
    return -1 ;
  }
  else {
    printf( "%s%s%s", CONTACTCOL, Contacts[i].nick, NOCOL );
    return i;
  }
}
/*********************************************
Converts a nick name into a uin from the contact
list.
**********************************************/
DWORD nick2uin( char *nick )
{
  int i;
  BOOL non_numeric=FALSE;
   
  for ( i=0; i< Num_Contacts; i++ ) {
    if ( ! strncasecmp( nick, Contacts[i].nick, 19  ) ) {
      if ( (S_DWORD) Contacts[i].uin > 0 )
	return Contacts[i].uin;
      else
	return -Contacts[i].uin; /* alias */
    }
  }
  for ( i=0; i < strlen( nick ); i++ ) {
    if ( ! isdigit( (int) nick[i] ) ) {
      non_numeric=TRUE;
      break;
    }
  }
  if ( non_numeric )
    return -1; /* not found and not a number */
  else
    return atoi( nick );
}


/**************************************************
Automates the process of creating a new user.
***************************************************/
void Init_New_User( void )
{
  SOK_T sok; 
  srv_net_icq_pak pak;
  int s;
  struct timeval tv;
  fd_set readfds;
#ifdef _WIN32
  WSADATA wsaData;
  i = WSAStartup( 0x0101, &wsaData );
  if ( i != 0 ) {
#ifdef FUNNY_MSGS
    perror("Windows Sockets broken blame Bill -");
#else
    perror("Sorry, can't initialize Windows Sockets...");
#endif
    exit(1);
  }
#endif
  printf( "\nCreating Connection...\n");
  sok = Connect_Remote( server, remote_port, STDERR );
  if ( ( sok == -1 ) || ( sok == 0 ) ) 
    {
      printf( "Couldn't establish connection\n" );
      exit( 1 );
    }
  printf( "Sending Request...\n" );
  reg_new_user( sok, passwd );
  for ( ; ; )
    {
#ifdef UNIX
      tv.tv_sec = 3;
      tv.tv_usec = 500000;
#else
      tv.tv_sec = 0;
      tv.tv_usec = 100000;
#endif
      
      FD_ZERO(&readfds);
      FD_SET(sok, &readfds);
      
      /* don't care about writefds and exceptfds: */
      select(sok+1, &readfds, NULL, NULL, &tv);
      printf( "Waiting for response....\n" );
      if (FD_ISSET(sok, &readfds))
	{
	  s = SOCKREAD( sok, &pak.head.ver, sizeof( pak ) - 2  );
	  if ( Chars_2_Word( pak.head.cmd ) == SRV_NEW_UIN )
	    {
	      UIN = Chars_2_DW( &pak.data[2] );
	      printf( "\nYour new UIN is %s%ld%s!\n",
		      SERVCOL, Chars_2_DW( &pak.data[2] ), NOCOL );
	      return;
	    }
	}
    }
}



static void Initalize_RC_File( void )
{
  FD_T rcf;
  char passwd2[ sizeof(passwd) ];
  rcf = open( rcfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if ( rcf == -1 )
    {
      perror( "Error creating config file " );
      exit( 1);
    }
  close( rcf );
  strcpy( server, "icq1.mirabilis.com" );
  remote_port = 4000;
  printf( "Enter UIN or 0 for new UIN: #" );
  fflush( stdout );
  scanf( "%ld", &UIN );
password_entry:
  printf( "Enter password : " );
  fflush( stdout );
  Echo_Off();
  memset( passwd, 0, sizeof( passwd ) );
  M_fdnreadln(STDIN, passwd, sizeof(passwd));
  Echo_On();
  if ( UIN == 0 )
    {
      if ( 0 == passwd[0] )
	{
	  printf( "\nMust enter password!\n" );
	  goto password_entry;
	}
      printf( "\nReenter password to verify: " );
      fflush( stdout );
      Echo_Off();
      memset( passwd2, 0, sizeof( passwd2 ) );
      M_fdnreadln(STDIN, passwd2, sizeof(passwd));
      Echo_On();
      if ( strcmp( passwd, passwd2 ) )
	{
	  printf( "\nPasswords did not match reenter\n" );
	  goto password_entry;
	}
      Init_New_User();
    }
  set_status = STATUS_ONLINE;
  Num_Contacts = 2;
  Contacts[ 0 ].invis_list = FALSE;
  Contacts[ 1 ].invis_list = FALSE;
  Contacts[0].uin = 11290140;
  strcpy( Contacts[0].nick, "Micq Author" );
  Contacts[0].status = STATUS_OFFLINE;
  Contacts[0].last_time = -1L;
  Contacts[0].current_ip[0] = 0xff;
  Contacts[0].current_ip[1] = 0xff;
  Contacts[0].current_ip[2] = 0xff;
  Contacts[0].current_ip[3] = 0xff;
  Contacts[ 0 ].port = 0;
  Contacts[ 0 ].sok = (SOK_T ) -1L;
  Contacts[1].uin = -11290140;
  strcpy( Contacts[1].nick, "alias1" );
  Contacts[1].status = STATUS_OFFLINE;
  Contacts[1].current_ip[0] = 0xff;
  Contacts[1].current_ip[1] = 0xff;
  Contacts[1].current_ip[2] = 0xff;
  Contacts[1].current_ip[3] = 0xff;
  Contacts[1].current_ip[0] = 0xff;
  Contacts[1].current_ip[1] = 0xff;
  Contacts[1].current_ip[2] = 0xff;
  Contacts[1].current_ip[3] = 0xff;
  Contacts[ 1 ].port = 0;
  Contacts[ 1 ].sok = (SOK_T ) -1L;
  
  strcpy(message_cmd, "msg");
  strcpy(info_cmd, "info");
  strcpy(add_cmd, "add");
  strcpy(quit_cmd, "q");
  strcpy(reply_cmd, "r");       
  strcpy(again_cmd, "a");
  strcpy(list_cmd, "w");
  strcpy(away_cmd, "away");
  strcpy(na_cmd, "na");
  strcpy(dnd_cmd, "dnd");   
  strcpy(online_cmd, "online");
  strcpy(occ_cmd, "occ");
  strcpy(ffc_cmd, "ffc");
  strcpy(inv_cmd, "inv");
  strcpy(status_cmd, "status");
  strcpy(auth_cmd, "auth");
  strcpy(change_cmd, "change");
  strcpy(auto_cmd, "auto");
  strcpy(search_cmd, "search");
  strcpy(save_cmd, "save");
  strcpy(alter_cmd, "alter");
  strcpy(msga_cmd, "msga");
  strcpy(url_cmd, "url");
  strcpy(update_cmd, "update");

  strcpy(Sound_Str, "play file=message.wav");
  /*  strcpy(SERVCOL,"\x1B[0;31m\n");  */
  /*  strcpy(MESSCOL,"\x1B[1;34m\n");  */
  /*  strcpy(CONTACTCOL,"\x1B[1;35m\n");  */
  /*  strcpy(CLIENTCOL,"\x1B[0;32m\n"); */
  /*  strcpy(NOCOL,"\x1B[0m\n");	    */
  
  Current_Status = STATUS_ONLINE;
  
  if ( Save_RC() == -1 )
    {
      perror( "Error creating config file " );
      exit( 1);
    }
}







static void Read_RC_File( FD_T rcf )
{
  char buf[450];
  char *tmp;
  int i;
  DWORD tmp_uin;
   
  message_cmd[0]='\0'; /* for error checking later */
  quit_cmd[0]='\0'; /* for error checking later */
  info_cmd[0]='\0'; /* for error checking later */
  reply_cmd[0]='\0'; /* for error checking later */
  again_cmd[0]='\0'; /* for error checking later */
  add_cmd[0]='\0'; /* for error checking later */
  list_cmd[0]='\0'; /* for error checking later */
  away_cmd[0]='\0'; /* for error checking later */
  na_cmd[0]='\0'; /* for error checking later */
  dnd_cmd[0]='\0'; /* for error checking later */
  online_cmd[0]='\0'; /* for error checking later */
  occ_cmd[0]='\0'; /* for error checking later */
  ffc_cmd[0]='\0'; /* for error checking later */
  inv_cmd[0]='\0'; /* for error checking later */
  status_cmd[0]='\0'; /* for error checking later */
  auth_cmd[0]='\0'; /* for error checking later */
  auto_cmd[0]='\0'; /* for error checking later */
  change_cmd[0]='\0'; /* for error checking later */
  search_cmd[0]='\0'; /* for error checking later */
  save_cmd[0]='\0'; /* for error checking later */
  alter_cmd[0]='\0'; /* for error checking later */
  msga_cmd[0]='\0'; /* for error checking later */
  url_cmd[0]='\0';   /* for error checking later */
  update_cmd[0]='\0';   /* for error checking later */
  passwd[0] = 0;
  UIN = 0;
	
  Contact_List = FALSE;
  for ( i=1; !Contact_List || buf == 0; i++ )
    {
      M_fdnreadln( rcf, buf, sizeof( buf ) );
      if ( ( buf[0] != '#' ) && ( buf[0] != 0 ) )
	{
	  tmp = strtok( buf, " " );
	  if ( ! strcasecmp( tmp, "Server" ) )
	    {
	      strcpy( server, strtok( NULL, " \n\t" ) );
	    }
	  else if ( ! strcasecmp( tmp, "Password" ) )
	    {
	      strcpy( passwd, strtok( NULL, "\n\t" ) );
	    }
	  else if ( ! strcasecmp( tmp, "Russian" ) )
	    {
	      Russian = TRUE;
	    }
	  else if ( ! strcasecmp( tmp, "No_Log" ) )
	    {
	      Logging = FALSE;
	    }
	  else if ( ! strcasecmp( tmp, "UIN" ) )
	    {
	      UIN = atoi( strtok( NULL, " \n\t" ) );
	    }
	  else if ( ! strcasecmp( tmp, "port" ) )
	    { remote_port = atoi( strtok( NULL, " \n\t" ) ); }
	  else if ( ! strcasecmp( tmp, "Status" ) )
            {  set_status = atoi( strtok( NULL, " \n\t" ) ); }
	  else if ( ! strcasecmp( tmp, "Auto" ) )
            { auto_resp = TRUE; }
	  ADD_COMMAND( "message_cmd", message_cmd )
	    ADD_COMMAND( "info_cmd", info_cmd )
	    ADD_COMMAND( "quit_cmd", quit_cmd )
	    ADD_COMMAND( "reply_cmd", reply_cmd )
	    ADD_COMMAND( "again_cmd", again_cmd )
	    ADD_COMMAND( "list_cmd", list_cmd )
	    ADD_COMMAND( "away_cmd", away_cmd )
	    ADD_MESS( "auto_rep_str_away", auto_rep_str_away )
	    ADD_MESS( "auto_rep_str_na", auto_rep_str_na )
	    ADD_COMMAND( "na_cmd", na_cmd )
	    ADD_COMMAND( "dnd_cmd", dnd_cmd )
	    ADD_MESS( "auto_rep_str_dnd", auto_rep_str_dnd )
	    ADD_MESS( "auto_rep_str_occ", auto_rep_str_occ )
	    ADD_MESS( "auto_rep_str_inv", auto_rep_str_inv )
	    ADD_COMMAND( "online_cmd", online_cmd )
	    ADD_COMMAND( "occ_cmd", occ_cmd )
	    ADD_COMMAND( "ffc_cmd", ffc_cmd )
	    ADD_COMMAND( "inv_cmd", inv_cmd )
	    ADD_COMMAND( "status_cmd", status_cmd )
	    ADD_COMMAND( "auth_cmd", auth_cmd )
	    ADD_COMMAND( "auto_cmd", auto_cmd )
	    ADD_COMMAND( "change_cmd", change_cmd )
	    ADD_COMMAND( "add_cmd", add_cmd )
	    ADD_COMMAND( "search_cmd", search_cmd )
	    ADD_COMMAND( "save_cmd", save_cmd )
	    ADD_COMMAND( "alter_cmd", alter_cmd )
	    ADD_COMMAND( "msga_cmd", msga_cmd )
	    ADD_COMMAND( "update_cmd", update_cmd )
	    ADD_COMMAND( "url_cmd", url_cmd )
	  else if ( ! strcasecmp( tmp, "sound" ) )
	    { Sound = atoi( strtok( NULL, " \n\t" ) ); }
         else if ( ! strcasecmp( tmp, "Sound_Str" ) )
	    {
	      strcpy( Sound_Str, strtok( NULL, "\n\t" ) );
	    }
         else if ( ! strcasecmp( tmp, "SERVCOL" ) )
	    {
	      strcpy( SERVCOL, strtok( NULL, "\n\t" ) );
	    }
	  else if ( ! strcasecmp( tmp, "MESSCOL" ) )
	    {
	      strcpy( MESSCOL, strtok( NULL, "\n\t" ) );
	    }
	  else if ( ! strcasecmp( tmp, "CONTACTCOL" ) )
	    {
	      strcpy( CONTACTCOL, strtok( NULL, "\n\t" ) );
	    }
	  else if ( ! strcasecmp( tmp, "CLIENTCOL" ) )
	    {
	      strcpy( CLIENTCOL, strtok( NULL, "\n\t" ) );
	    }
	  else if ( ! strcasecmp( tmp, "NOCOL" ) )
	    {
	      strcpy( NOCOL, strtok( NULL, "\n\t" ) );
	    }
	  else if ( ! strcasecmp( tmp, "Contacts" ) )
	    {
	      Contact_List = TRUE;
	    }
         else
	   {
	     printf("%sUnrecognized command in rc file : %s, ignored.%s\n", 
		    SERVCOL,tmp,NOCOL);
	   }
   	}
    }
  for ( ; ! M_fdnreadln( rcf, buf, sizeof( buf ) ); )
    {
      if ( Num_Contacts == 100 )
	break;
      if ( ( buf[0] != '#' ) && ( buf[0] != 0 ) )
	{
	  if ( isdigit( (int) buf[0] ) )
	    {
	      Contacts[ Num_Contacts ].uin = atoi( strtok( buf, " " ) );
	      Contacts[ Num_Contacts ].status = STATUS_OFFLINE;
	      Contacts[ Num_Contacts ].last_time = -1L;
	      Contacts[ Num_Contacts ].current_ip[0] = 0xff;
	      Contacts[ Num_Contacts ].current_ip[1] = 0xff;
	      Contacts[ Num_Contacts ].current_ip[2] = 0xff;
	      Contacts[ Num_Contacts ].current_ip[3] = 0xff;
	      tmp = strtok( NULL, "" );
	      if ( tmp != NULL )
		memcpy( Contacts[ Num_Contacts ].nick, tmp, 
			sizeof( Contacts->nick )  );
	      else
		Contacts[ Num_Contacts ].nick[0] = 0;
	      if ( Contacts[ Num_Contacts ].nick[19] != 0 )
		Contacts[ Num_Contacts ].nick[19] = 0;
	      if ( Verbose )
		printf( "%ld = %s\n", Contacts[ Num_Contacts ].uin, 
			Contacts[ Num_Contacts ].nick );
	      Contacts[ Num_Contacts ].invis_list = FALSE;
	      Num_Contacts++;
	    }
	  else if ( buf[0] == '*' )
	    {
	      Contacts[ Num_Contacts ].uin = atoi( strtok( &buf[1], " " ) );
	      Contacts[ Num_Contacts ].status = STATUS_OFFLINE;
	      Contacts[ Num_Contacts ].last_time = -1L;
	      Contacts[ Num_Contacts ].current_ip[0] = 0xff;
	      Contacts[ Num_Contacts ].current_ip[1] = 0xff;
	      Contacts[ Num_Contacts ].current_ip[2] = 0xff;
	      Contacts[ Num_Contacts ].current_ip[3] = 0xff;
	      tmp = strtok( NULL, "" );
	      if ( tmp != NULL )
		memcpy( Contacts[ Num_Contacts ].nick, tmp, 
			sizeof( Contacts->nick )  );
	      else
		Contacts[ Num_Contacts ].nick[0] = 0;
	      if ( Contacts[ Num_Contacts ].nick[19] != 0 )
		Contacts[ Num_Contacts ].nick[19] = 0;
	      if ( Verbose )
		printf( "%ld = %s\n", Contacts[ Num_Contacts ].uin, 
			Contacts[ Num_Contacts ].nick );
	      Contacts[ Num_Contacts ].invis_list = TRUE;
	      Num_Contacts++;
	    }
	  else
	    {
	      tmp_uin = Contacts[ Num_Contacts - 1 ].uin;
	      tmp = strtok( buf, ", \t" ); /* aliases may not have spaces */
	      for ( ; tmp!=NULL; Num_Contacts++ )
		{
		  Contacts[ Num_Contacts ].uin = -tmp_uin;
		  Contacts[ Num_Contacts ].status = STATUS_OFFLINE;
		  Contacts[ Num_Contacts ].last_time = -1L;
		  Contacts[ Num_Contacts ].current_ip[0] = 0xff;
		  Contacts[ Num_Contacts ].current_ip[1] = 0xff;
		  Contacts[ Num_Contacts ].current_ip[2] = 0xff;
		  Contacts[ Num_Contacts ].current_ip[3] = 0xff;
		  Contacts[ Num_Contacts ].port = 0;
		  Contacts[ Num_Contacts ].sok = (SOK_T ) -1L;
		  memcpy( Contacts[ Num_Contacts ].nick, tmp, 
			  sizeof( Contacts->nick )  );
		  tmp = strtok( NULL, ", \t" );
		}
	    }
	}
    }
  if(message_cmd[0]=='\0')
    strcpy(message_cmd, "msg");
  if(update_cmd[0]=='\0')
    strcpy(update_cmd, "update");
  if(info_cmd[0]=='\0')
    strcpy(info_cmd, "info");
  if(quit_cmd[0]=='\0')
    strcpy(quit_cmd, "q");
  if(reply_cmd[0]=='\0')
    strcpy(reply_cmd, "r");       
  if(again_cmd[0]=='\0')
    strcpy(again_cmd, "a");
  if(list_cmd[0]=='\0')
    strcpy(list_cmd, "w");
  if(away_cmd[0]=='\0')
    strcpy(away_cmd, "away");
  if(na_cmd[0]=='\0')
    strcpy(na_cmd, "na");
  if(dnd_cmd[0]=='\0')
    strcpy(dnd_cmd, "dnd");   
  if(online_cmd[0]=='\0')
    strcpy(online_cmd, "online");
  if(occ_cmd[0]=='\0')
    strcpy(occ_cmd, "occ");
  if(ffc_cmd[0]=='\0')
    strcpy(ffc_cmd, "ffc");
  if(inv_cmd[0]=='\0')
    strcpy(inv_cmd, "inv");
  if(status_cmd[0]=='\0')
    strcpy(status_cmd, "status");
  if(add_cmd[0]=='\0')
    strcpy(add_cmd, "add");
  if(auth_cmd[0]=='\0')
    strcpy(auth_cmd, "auth");
  if(auto_cmd[0]=='\0')
    strcpy(auto_cmd, "auto");
  if(search_cmd[0]=='\0')
    strcpy(search_cmd, "search");
  if(save_cmd[0]=='\0')
    strcpy(save_cmd, "save");
  if(alter_cmd[0]=='\0')
    strcpy(alter_cmd, "alter");
  if(msga_cmd[0]=='\0')
    strcpy(msga_cmd, "msga");
  if(url_cmd[0]=='\0')
    strcpy(url_cmd, "url");

  if(change_cmd[0]=='\0')
    strcpy(change_cmd, "change");
  if ( Verbose )
    {
      printf( "UIN = %ld\n", UIN );
      printf( "port = %ld\n", remote_port );
      printf( "passwd = %s\n", passwd );
      printf( "server = %s\n", server );
      printf( "status = %ld\n", set_status );
      printf( "# of contacts = %d\n", Num_Contacts );
      printf( "UIN of contact[0] = %ld\n", Contacts[0].uin );
      printf( "Message_cmd = %s\n", message_cmd );
    }
  if (UIN == 0 ) 
    {
      fprintf( stderr, "Bad .micqrc file.  No UIN found aborting.\a\n" );
      exit( 1);
    }
}
/*******************************************************
Gets config info from the rc file in the users home 
directory.
********************************************************/
void Get_Unix_Config_Info( void )
{
  char *path;
  FD_T rcf;
#ifdef _WIN32
  path = ".\\";
#endif
#ifdef UNIX
  path = getenv( "HOME" );
  strcat( path, "/" );
#endif
#ifdef __amigaos__
  path = "PROGDIR:";
#endif
  strcpy( rcfile, path );
  strcat( rcfile, "micqrc" );
  rcf = open( rcfile, O_RDONLY );
  if ( rcf == -1 )
    {
      if ( errno == ENOENT ) /* file not found */
	{
	  Initalize_RC_File();
	}
      else
	{
	  perror( "Error reading config file exiting " );
	  exit( 1 );
	}
    }
  else
    {
      Read_RC_File( rcf );
    }
}


void Print_IP( DWORD uin )
{
  int i;
  for ( i=0; i< Num_Contacts; i++ )
    {
      if ( Contacts[i].uin == uin )
	{
	  if ( * (DWORD *)Contacts[i].current_ip != -1L )
	    {
	      printf( "%d.%d.%d.%d", Contacts[i].current_ip[0],
		      Contacts[i].current_ip[1],
		      Contacts[i].current_ip[2],
		      Contacts[i].current_ip[3] );
	    }
	  else
	    {
	      printf( "unknown" );
	    }
	  return;
	}
    }
  printf( "unknown" );
}



/************************************************
Gets the TCP port of the specified UIN
************************************************/
DWORD Get_Port( DWORD uin )
{
  int i;
   
  for ( i=0; i< Num_Contacts; i++ )
    {
      if ( Contacts[i].uin == uin )
	{
	  return Contacts[i].port;
	}
    }
  return -1L;
}
/********************************************
Converts an intel endian character sequence to
a DWORD
*********************************************/
DWORD Chars_2_DW( unsigned char *buf )
{
  DWORD i;
   
  i= buf[3];
  i <<= 8;
  i+= buf[2];
  i <<= 8;
  i+= buf[1];
  i <<= 8;
  i+= buf[0];
   
  return i;
}
/********************************************
Converts an intel endian character sequence to
a WORD
*********************************************/
WORD Chars_2_Word( unsigned char *buf )
{
  WORD i;
   
  i= buf[1];
  i <<= 8;
  i += buf[0];
   
  return i;
}
/********************************************
Converts a DWORD to
an intel endian character sequence 
*********************************************/
void DW_2_Chars( unsigned char *buf, DWORD num )
{
  buf[3] = ( unsigned char ) ((num)>>24)& 0x000000FF;
  buf[2] = ( unsigned char ) ((num)>>16)& 0x000000FF;
  buf[1] = ( unsigned char ) ((num)>>8)& 0x000000FF;
  buf[0] = ( unsigned char ) (num) & 0x000000FF;
}
/********************************************
Converts a WORD to
an intel endian character sequence 
*********************************************/
void Word_2_Chars( unsigned char *buf, WORD num )
{
  buf[1] = ( unsigned char ) (((unsigned)num)>>8) & 0x00FF;
  buf[0] = ( unsigned char ) ((unsigned)num) & 0x00FF;
}

void Time_Stamp( void )
{
  struct tm *thetime;
  time_t p;
   
  p=time(NULL);
  thetime=localtime(&p);
  printf( "%.02d:%.02d:%.02d",thetime->tm_hour,thetime->tm_min,
	  thetime->tm_sec );
}
void Add_User( SOK_T sok, DWORD uin, char *name )
{
  FD_T rcf;
  rcf = open( rcfile, O_RDWR | O_APPEND );
  M_fdprint( rcf, "%d %s\n", uin, name );
  close( rcf );
  Contacts[ Num_Contacts ].uin = uin;
  Contacts[ Num_Contacts ].status = STATUS_OFFLINE;
  Contacts[ Num_Contacts ].last_time = -1L;
  Contacts[ Num_Contacts ].current_ip[0] = 0xff;
  Contacts[ Num_Contacts ].current_ip[1] = 0xff;
  Contacts[ Num_Contacts ].current_ip[2] = 0xff;
  Contacts[ Num_Contacts ].current_ip[3] = 0xff;
  Contacts[ Num_Contacts ].port = 0;
  Contacts[ Num_Contacts ].sok = (SOK_T ) -1L;
  Contacts[ Num_Contacts ].invis_list = FALSE;   
  memcpy( Contacts[ Num_Contacts ].nick, name, sizeof( Contacts->nick )  );
  Num_Contacts++;
  snd_contact_list( sok );
}

/************************************************
 *   This function should save your auto reply messages in the rc file.
 *  
 *   NOTE: the code isn't realy neat yet, I hope to change that soon.
 *  
 *   Added on 6-20-98 by Fryslan
 *  
 ***********************************************/
int Save_RC()
{
  FD_T rcf;
  time_t t;
  int i,j;
 
  rcf = open( rcfile, O_RDWR );
  if ( rcf == -1 ) return -1;
  M_fdprint( rcf, "# This file was generated by Micq of %s %s\n",
	     __TIME__,__DATE__);
  t = time( NULL );
  M_fdprint( rcf, "# This file was generated on %s", ctime( &t ) );   
  M_fdprint( rcf, "UIN %d\n", UIN );
  M_fdprint( rcf, "Password %s\n", passwd );
  M_fdprint( rcf, "Status %d\n", Current_Status );
  M_fdprint( rcf, "Server %s\n", "icq1.mirabilis.com" );
  M_fdprint( rcf, "Port %d\n", 4000 );
  if ( Logging )
    M_fdprint( rcf, "#No_Log\n" );
  else
    M_fdprint( rcf, "No_Log\n" );
  if ( Russian )
    M_fdprint( rcf, "\nRussian\n#if you want KOI8-R to CP1251 "
	       "Russain translation uncomment the above line.\n" );
  else
    M_fdprint( rcf, "\n#Russian\n#if you want KOI8-R to CP1251 "
	       "Russain translation uncomment the above line.\n" );
  if ( auto_resp )
    M_fdprint( rcf, "\n#Automatic responses on.\nAuto\n" );
  else
    M_fdprint( rcf, "\n#Automatic responses off.\n#Auto\n" );
  M_fdprint( rcf, "\n# Below are the commands which can be "
	     "changed to most anything you want :)\n" );
  M_fdprint( rcf, "message_cmd %s\n",message_cmd);
  M_fdprint( rcf, "info_cmd %s\n",info_cmd);
  M_fdprint( rcf, "quit_cmd %s\n",quit_cmd);
  M_fdprint( rcf, "reply_cmd %s\n",reply_cmd);
  M_fdprint( rcf, "again_cmd %s\n",again_cmd);
  M_fdprint( rcf, "list_cmd %s\n",list_cmd);
  M_fdprint( rcf, "away_cmd %s\n",away_cmd);
  M_fdprint( rcf, "na_cmd %s\n",na_cmd);
  M_fdprint( rcf, "dnd_cmd %s\n",dnd_cmd);
  M_fdprint( rcf, "online_cmd %s\n",online_cmd);
   
  M_fdprint( rcf, "occ_cmd %s\n",occ_cmd);
  M_fdprint( rcf, "ffc_cmd %s\n",ffc_cmd);
  M_fdprint( rcf, "inv_cmd %s\n",inv_cmd);
  M_fdprint( rcf, "search_cmd %s\n",search_cmd);
  M_fdprint( rcf, "status_cmd %s\n",status_cmd);
  M_fdprint( rcf, "auth_cmd %s\n",auth_cmd);
  M_fdprint( rcf, "auto_cmd %s\n",auto_cmd);
  M_fdprint( rcf, "add_cmd %s\n",add_cmd);
  M_fdprint( rcf, "change_cmd %s\n",change_cmd);
  M_fdprint( rcf, "save_cmd %s\n",save_cmd);
  M_fdprint( rcf, "alter_cmd %s\n",alter_cmd);
  M_fdprint( rcf, "msga_cmd %s\n",msga_cmd);
  M_fdprint( rcf, "url_cmd %s\n",url_cmd);
  M_fdprint( rcf, "update_cmd %s\n",update_cmd);

  M_fdprint( rcf, "\n#Now sound settings: \n" );	 
  M_fdprint( rcf, "#sound [0|1|2]\n" );	 
  M_fdprint( rcf, "sound %i\n", Sound);
  M_fdprint( rcf, "Sound_Str %s\n", Sound_Str);

  M_fdprint( rcf, "\n#Now color settings: \n" );	 
  M_fdprint( rcf, "SERVCOL %s\n", SERVCOL);
  M_fdprint( rcf, "MESSCOL %s\n", MESSCOL);
  M_fdprint( rcf, "CONTACTCOL %s\n", CONTACTCOL);
  M_fdprint( rcf, "CLIENTCOL %s\n", CLIENTCOL);
  M_fdprint( rcf, "NOCOL %s\n", NOCOL);
	
  M_fdprint( rcf, "\n#Now auto response messages\n" );	 
  M_fdprint( rcf, "auto_rep_str_away %s\n",auto_rep_str_away);
  M_fdprint( rcf, "auto_rep_str_na %s\n",auto_rep_str_na);	
  M_fdprint( rcf, "auto_rep_str_dnd %s\n",auto_rep_str_dnd);
  M_fdprint( rcf, "auto_rep_str_occ %s\n",auto_rep_str_occ);
  M_fdprint( rcf, "auto_rep_str_inv %s\n",auto_rep_str_inv);
  
  M_fdprint( rcf, "\n# Ok now the contact list\n" );
  M_fdprint( rcf, "Contacts\n" );
  /* adding contacts to the rc file. */
  /* we start counting at zero in the index. */
	
  for (i=0;i<Num_Contacts;i++)
    {
      if ( ! ( Contacts[i].uin & 0x80000000L ) )
	{
	  if ( Contacts[i].invis_list )
	    {
	      M_fdprint( rcf, "*" );
	    }
	  M_fdprint( rcf, "%d %s\n",Contacts[i].uin,Contacts[i].nick);
	  for ( j=0; j< Num_Contacts; j++ )
	    {
	      if ( Contacts[j].uin == -Contacts[i].uin )
		{
		  M_fdprint( rcf, "%s ", Contacts[j].nick );
		}
	    }
	  M_fdprint( rcf, "\n" );
	}
    }
  M_fdprint( rcf, "\n" );
  return close( rcf );
}

/*************************************************************************
 *
 *      Function: log_event
 *
 *      Purpose: Log the event provided to the log with a time stamp.
 *      Andrew Frolov dron@ilm.net
 * 
 *      6-20-98 Added names to the logs. Fryslan
 * 
 *************************************************************************/
int log_event( char *desc, char *msg, DWORD uin )
{
  FILE    *msgfd;
  char    buffer[256];
  time_t  timeval;
  char *path;
  if ( ! Logging )
    return 0;
      
#ifdef _WIN32
  path = ".\\";
#endif
#ifdef UNIX
  path = getenv( "HOME" );
  path = strdup( path );
  strcat( path, "/" );
#endif
#ifdef __amigaos__
  path = "PROGDIR:";
#endif
  strcpy( buffer, path );
  strcat( buffer, "micq_log" );

  if( ( msgfd = fopen(buffer, "a") ) == (FILE *) NULL ) 
    {
      fprintf(stderr, "Couldn't open %s for logging\n",
	      buffer);
      return(-1);
    }
  timeval = time(0);
  if ( ! strcasecmp(UIN2nick(uin),"Unknow UIN"))
    fprintf(msgfd, "\n%-24.24s %s %ld\n%s\n", ctime(&timeval), 
	    desc, uin, msg);
  else
    fprintf(msgfd, "\n%-24.24s %s %s\n%s\n", ctime(&timeval), 
	    desc, UIN2nick(uin), msg);
	 
  fclose(msgfd);
#ifdef UNIX
  free( path );
  chmod( buffer, 0600 );
#endif
  return(0);
}

void clrscr(void)   // clears the screen 
{
  /*   int x; */
  /*   char newline = '\n';	 */
  /*   for(x = 0; x<=25; x++) */
  /*     printf("%c",newline); */
  /*  :-)  Why not use */  
  printf("\x1B[2J");
}


/************************************************************
Displays a hex dump of buf on the screen.
*************************************************************/
void Hex_Dump( char *buf, size_t len )
{
  int i;
  int j;
  
  for ( i=0 ; i < len; i++ )
    {
      printf( "%02x ", ( unsigned char ) buf[i] );
      if ( ( i & 15 ) == 15 )
	{
	  printf( "  " );
	  for ( j = 15; j >= 0; j-- )
            {
	      if ( buf[i-j] > 31 )
		printf( "%c", buf[i-j] );
	      else
		printf( "." );
	      if ( ( (i-j) & 3 ) == 3 )
		printf( " " );
            }
	  printf( "\n" );
	}
      else if ( ( i & 7 ) == 7 )
	printf( "- " );
      else if ( ( i & 3 ) == 3 )
	printf( "  " );
    }
  printf( "  " );
  for ( j = i % 16; j > 0; j-- )
    {
      if ( buf[i-j] > 31 )
	printf( "%c", buf[i-j] );
      else
	printf( "." );
      if ( ( (i-j) & 3 ) == 3 )
	printf( " " );
    }
  printf( "\n" );
}
