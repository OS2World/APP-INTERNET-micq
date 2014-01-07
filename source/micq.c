/*********************************************
**********************************************
This is the main ICQ file. Currently it
logs in and sits in a loop. It can receive
messages and keeps the connection alive.
Use crtl-break to exit.
This software is provided AS IS to be used in
whatever way you see fit and is placed in the
public domain.
Author : Matthew Smith April 19, 1998
Contributors : Nicolas Sahlqvist April 27, 1998
			   Ulf Hedlund (guru@slideware.com) April 28, 1998
            Michael Ivey May 4, 1998
            Michael Holzt May 5, 1998
Changes :
   4-28-98 support for WIN32 [UH]
   4-20-98 added variable time_delay between keep_alive packets mds
   4-20-98 added instant message from server support mds
   4-21-98 changed so that long ( 250+ characters ) messages work
            new maximum is ~900 which is hopefully big enough.
            When I know more about udp maybe I can come up with
            a general solution. mds I now think ICQ has a max that is
            smaller than this so everything is ok mds I now think that
            the icq client's maximum is arbitrary and can be ignored :)
   4-23-98 Added beginnings of a user interface
   4-26-98 Changed the version to 0.2a :)
   4-27-98 Nicco added feature to use nick names to log in
   5-05-98 Authorization Messages
   5-13-98 Added time stamps for most things.
   6-17-98 Changed condition on which we should send auto_reply message. 
            Fryslan
   6-18-98 Added different auto reply messages for different status types 
            see also ui.c and util.c Fryslan
   6-20-98 Added an alter command to alter your command names online. Fryslan
   7-18-98 Formatted for 80 characters per line. 
            Re-Indented (M-x indent-region)
            Added OS/2 support. 
            Added GNU Readline support (Including Histoy)
	    Removed colored prompts (doesnt work correctly with readline) 
	    Auto-Reconnect
	    colors can be changed in .micqrc
	    Added cls during startup
	    changed clrscr()
	    Volker
**********************************************
**********************************************/
#include "micq.h"
#include "datatype.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <conio.h>
#include <io.h>
#include <winsock2.h>
#include <time.h>
#elif defined (__EMX__)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termio.h>
#include <sys/wait.h>
#else
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/wait.h>
#endif
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "rl_support.h"
#include <process.h>

extern USER_INFO_STRUCT user;
BYTE Sound = SOUND_ON; /* Beeps on by default */
BYTE Sound_Str[150]="";
  /* the command to run from the shell to play sound files */
BOOL Russian = FALSE;
BOOL Logging = TRUE;
BOOL Color = TRUE; /* Do we use ANSI color? */
BOOL Quit = FALSE;
BOOL Verbose = FALSE;     /* this is needed because I leeched so code 
                             from another project I'm working on.      */
BOOL AutoReconnect = TRUE;
BOOL serv_mess[ 1024 ];
WORD last_cmd[ 1024 ];    /* command issued for the first 1024 SEQ #'s */
/*********** if we have more than 1024 SEQ this will need some hacking */
WORD seq_num = 1;         /* current sequence number             */
DWORD our_ip = 0x0100007f;/* localhost for some reason           */
DWORD our_port;           /* the port to make tcp connections on */
int sok;                  /* The socket                          */
/************ We don't make tcp connections yet though :( */
DWORD UIN;                 /* current User Id Number */
BOOL Contact_List = FALSE;
Contact_Member Contacts[ 100 ]; /* no more than 100 contacts max */
int Num_Contacts=0;
DWORD Current_Status=STATUS_OFFLINE;
DWORD last_recv_uin=0;
char passwd[100];
char server[100];
DWORD set_status;
DWORD remote_port;
BOOL Done_Login=FALSE;
BOOL auto_resp=FALSE;
char auto_rep_str_dnd[450] = { "Don't page me, my head is hurting!" };
char auto_rep_str_away[450] = { "I told you I wasn't here!" };
char auto_rep_str_na[450] = { "Working, working always working..." };
char auto_rep_str_occ[450] = 
{ "I am working on opening this beer so I am busy." };
char auto_rep_str_inv[450] = { "So you can see me, so you can't!" };
char message_cmd[16];
char info_cmd[16];
char quit_cmd[16];
char reply_cmd[16];
char again_cmd[16];
char add_cmd[16];
char list_cmd[16];
char away_cmd[16];
char na_cmd[16];
char dnd_cmd[16];
char online_cmd[16];
char occ_cmd[16];
char ffc_cmd[16];
char inv_cmd[16];
char status_cmd[16];
char auth_cmd[16];
char auto_cmd[16];
char change_cmd[16];
char search_cmd[16];
char save_cmd[16];
char alter_cmd[16];
char msga_cmd[16];
char url_cmd[16];
char update_cmd[16];

char * completed_command_line=NULL;
char * Prompt=NULL;

/*/////////////////////////////////////////////
// Connects to hostname on port port
// hostname can be DNS or nnn.nnn.nnn.nnn
// write out messages to the FD aux */
int Connect_Remote( char *hostname, int port, FD_T aux )
{
  int conct, length;
  int sok;
  struct sockaddr_in sin;  /* used to store inet addr stuff */
  struct hostent *host_struct; /* used in DNS llokup */

  /* checks for n.n.n.n notation */
  sin.sin_addr.s_addr = inet_addr( hostname ); 

  /* name isn't n.n.n.n so must be DNS */
  if ( sin.sin_addr.s_addr  == -1 ) 
    {
      host_struct = gethostbyname( hostname );
      if ( host_struct == NULL ) 
	{
	  if ( Verbose )
	    {
	      M_fdprint( aux, "Shakespeare couldn't spell why should I?\n" );
	      M_fdprint( aux, " Especially something like %s\n", hostname );
	      /*herror( "Can't find hostname" );*/
	    }
	  return 0;
	}
      sin.sin_addr = *((struct in_addr *)host_struct->h_addr);
    }
  sin.sin_family = AF_INET; /* we're using the inet not appletalk*/
  sin.sin_port = htons( port );	/* port */
  sok = socket( AF_INET, SOCK_DGRAM, 0 );/* create the unconnected socket*/
  if ( sok == -1 )
    {
      perror( "Socket creation failed" );
      exit( 1 );
    }   
  if ( Verbose )
    {
      M_fdprint( aux, "Socket created attempting to connect\n" );
    }
  conct = connect( sok, (struct sockaddr *) &sin, sizeof( sin ) );
  if ( conct == -1 )/* did we connect ?*/
    {
      if ( Verbose )
	{
	  M_fdprint( aux, " Conection Refused on port %d at %s\n", 
		     port, hostname );
#ifdef FUNNY_MSGS
	  M_fdprint( aux, " D'oh!\n" );
#endif
	  perror( "connect" );
	}
      return 0;
    }
  length = sizeof( sin ) ;
  getsockname( sok, (struct sockaddr *) &sin, &length );
  our_ip = sin.sin_addr.s_addr;
  our_port = sin.sin_port;
  if (Verbose )
    {
#ifdef FUNNY_MSGS
      M_fdprint( aux, "Our port is %d, take her to sea Mr. Mordoch.\n", 
		 ntohs( sin.sin_port ) );
#else
      M_fdprint( aux, "The port that will be used for tcp"
		 " ( not yet implemented ) is %d\n", ntohs( sin.sin_port ) );
#endif
    }
  if ( Verbose )
    {
      M_fdprint( aux, "Connected to %s, waiting for response\n", hostname );
    }
  return sok;
}


void Logon()
{
  BOOL retry;
  do
    {
      retry=FALSE;
#ifdef _WIN32
      i = WSAStartup( 0x0101, &wsaData );
      if ( i != 0 ) {
#ifdef FUNNY_MSGS
	perror("Windows Sockets broken blame Bill -");
#else
	perror("Sorry, can't initialize Windows Sockets...");
#endif
	if (!AutoReconnect) exit(1);
	retry=TRUE;
      }
#endif
      sok = Connect_Remote( server, remote_port, STDERR );
      if ( ( sok == -1 ) || ( sok == 0 ) ) 
	{
	  if (AutoReconnect) 
	    {
	      printf( "\nCouldn't establish connection. Retrying.\n" );
	      retry=TRUE;
	    }
	  else
	    {
	      printf( "\nCouldn't establish connection\n" );
	      exit( 1 );
	    }
	}
      Login( sok, UIN, &passwd[0], our_ip, our_port, set_status);
    }
  while(retry);
}


/******************************************
Handles packets that the server sends to us.
*******************************************/
void Handle_Server_Response( SOK_T sok )
{
  srv_net_icq_pak pak;
  SIMPLE_MESSAGE_PTR s_mesg;
  int s,i,len;
   
  s = SOCKREAD( sok, &pak.head.ver, sizeof( pak ) - 2  );
  if ( s < 0 )
    return;
  if ( ( serv_mess[ Chars_2_Word( pak.head.seq ) ] ) && 
       ( Chars_2_Word( pak.head.cmd ) != SRV_NEW_UIN ) )
    {
      if ( Chars_2_Word( pak.head.cmd ) != SRV_ACK ) /* ACKs don't matter */
	{
	  if ( Verbose )
	    printf( "\nIgnored a message cmd  %04x\n", 
		    Chars_2_Word( pak.head.cmd )  );
	  ack_srv( sok, Chars_2_Word( pak.head.seq ) ); /* LAGGGGG!! */
	  return;
	}
    }
  if ( Chars_2_Word( pak.head.cmd ) != SRV_ACK )
    serv_mess[ Chars_2_Word( pak.head.seq ) ] = TRUE;
  switch ( Chars_2_Word( pak.head.cmd ) )
    {
    case SRV_ACK:
      if ( Verbose )
	printf("\nThe server acknowledged the %04x command.\n", 
	       last_cmd[ Chars_2_Word( pak.head.seq ) ] );
      break;
    case SRV_NEW_UIN:
      printf("\nThe new UIN is %ld!\n", Chars_2_DW( &pak.data[2] ) );
      break;
   case SRV_UPDATE_FAIL:
      printf( "\nFailed to update info.\n" );
      ack_srv( sok, Chars_2_Word( pak.head.seq ) );
      break;
   case SRV_UPDATE_SUCCESS:
      printf( "\nUser info successfully updated.\n" );
      ack_srv( sok, Chars_2_Word( pak.head.seq ) );
      break;      
    case SRV_LOGIN_REPLY:
      UIN = Chars_2_DW( &pak.data[0] );
      our_ip = Chars_2_DW( &pak.data[4] );
      printf( "\nLogin successful! UIN : %lu\n", UIN );
      printf( " IP : %u.%u.%u.%u\t", pak.data[4], pak.data[5], 
	      pak.data[6], pak.data[7] );
      Time_Stamp();
      printf( "\n" );
      ack_srv( sok, Chars_2_Word( pak.head.seq ) );
      snd_login_1( sok );
      snd_invis_list( sok );
      snd_contact_list( sok );
      icq_change_status( sok, set_status );
      break;
    case SRV_RECV_MESSAGE:
      Recv_Message( sok, pak );
      break;
    case SRV_X1: /* unknown message  sent after login*/
      if ( Verbose )
	printf( "\nAcknowleged SRV_X1 0x021C Begin messages?\n" );
      ack_srv( sok, Chars_2_Word( pak.head.seq ) );
      break;
    case SRV_X2: /* unknown message  sent after login*/
      if ( Verbose )
	printf( "\nAcknowleged SRV_X2 0x00E6 Done old messages?\n" );
      Show_Quick_Status();
      Done_Login = TRUE;
      ack_srv( sok, Chars_2_Word( pak.head.seq ) );
      snd_got_messages( sok );
      break;
    case SRV_INFO_REPLY:
      Display_Info_Reply( sok, pak );
      break;
    case SRV_EXT_INFO_REPLY:
      Display_Ext_Info_Reply( sok, pak );
      break;
    case SRV_USER_OFFLINE:
      User_Offline( sok, pak );
      break;
   case SRV_BAD_PASS:
      printf( "\n%sYou entered an incorrect password.%s\n",MESSCOL,NOCOL);
      exit( 1 );
      break;
    case SRV_TRY_AGAIN:
      printf( "%s\nServer is busy please try again.\nTrying again...%s\n",
	      MESSCOL, NOCOL);
      sleep(5);
      Login( sok, UIN, &passwd[0], our_ip, our_port, set_status );
      for ( i = 0; i< 1024; i++ )
	{
	  serv_mess[ i ]=FALSE;
	}
      break;
    case SRV_USER_ONLINE:
      User_Online( sok, pak );
      break;
    case SRV_STATUS_UPDATE:
      Status_Update( sok, pak );
      break;
    case SRV_GO_AWAY:
#ifdef FUNNY_MSGS 
      printf( "\nServer sent \"Go away!!\" command." );
#else
      printf("\nServer has forced us to disconnect.  "
	     "This may be because of network lag.");
#endif
      Time_Stamp();
      if (AutoReconnect)
	{
	  printf("Retrying\n");
	  Logon();
	}
      else
	Quit = TRUE;
      break;
    case SRV_END_OF_SEARCH:
      printf( "\nSearch done.\n" );
      ack_srv( sok, Chars_2_Word( pak.head.seq ) );
      break;
    case SRV_USER_FOUND:
      Display_Search_Reply( sok, pak );
      break;
    case SRV_SYS_DELIVERED_MESS:
      printf( "\n" );
      s_mesg = ( SIMPLE_MESSAGE_PTR ) pak.data;
      last_recv_uin = Chars_2_DW( s_mesg->uin );
      Print_UIN_Name( Chars_2_DW( s_mesg->uin  ) );
      printf( " - Instant Message - " );
      if ( SOUND_ON == Sound )
	printf("\a");
      if ( SOUND_CMD == Sound )
	spawnl(P_NOWAIT,"cmd.exe","cmd.exe","/C",Sound_Str,(char*)NULL);      
      if ( Verbose )
	printf( " Type = %04x\t", Chars_2_Word( s_mesg->type ) );
      Time_Stamp();
      Do_Msg( sok, Chars_2_Word( s_mesg->type ), Chars_2_Word( s_mesg->len ), 
	      s_mesg->len + 2,last_recv_uin); 
      ack_srv( sok, Chars_2_Word( pak.head.seq ) );
      if ( 0xfe != *( ((unsigned char *) s_mesg ) + sizeof( s_mesg ) ) )
	{
	  if ( auto_resp &&
	       (Current_Status != STATUS_ONLINE) &&
	       (Current_Status != STATUS_FREE_CHAT)  )
	    {
	      switch ( Current_Status )
		{
		case STATUS_OCCUPIED:
		  icq_sendmsg( sok, Chars_2_DW( s_mesg->uin ), 
			       auto_rep_str_occ, NORM_MESS );
		  break;
		case STATUS_AWAY:
		  icq_sendmsg( sok, Chars_2_DW( s_mesg->uin ), 
			       auto_rep_str_away, NORM_MESS );
		  break;
		case STATUS_DND:
		  icq_sendmsg( sok, Chars_2_DW( s_mesg->uin ), 
			       auto_rep_str_dnd, NORM_MESS );
		  break;
		case STATUS_INVISIBLE:
		  icq_sendmsg( sok, Chars_2_DW( s_mesg->uin ), 
			       auto_rep_str_inv, NORM_MESS );
		  break;
		case STATUS_NA:
		  icq_sendmsg( sok, Chars_2_DW( s_mesg->uin ), 
			       auto_rep_str_na, NORM_MESS );
		  break;
		default:
		  printf("You have encounterd a bug in my code :( "
			 "I now own you a beer!\nGreetings Fryslan!\n");
		}
	      printf( "[ Sent auto-reply message ]\n" );
	      log_event("Sending an auto-reply message to ","",last_recv_uin);
	    }
	}
      break;
    default: /* commands we dont handle yet */
      printf( "%s\nThe response was %04X\t", CLIENTCOL,
	      Chars_2_Word( pak.head.cmd ) );
      printf( "The version was %X\t", Chars_2_Word( pak.head.ver ) );
      Time_Stamp();
      printf( "\nThe SEQ was %04X\t", Chars_2_Word( pak.head.seq ) );
      len = s - ( sizeof( pak.head ) - 2 );
      printf( "The size was %d\n", len );
      if ( Verbose )
	{
	  Hex_Dump( pak.data, len );
	}
      printf( "%s\n", NOCOL );
      ack_srv( sok, Chars_2_Word( pak.head.seq ) ); 
      /* fake like we know what we're doing*/
      break;
    }
}


/**********************************************
Verifies that we are in the correct endian
***********************************************/
void Check_Endian( void )
{
  int i;
  char passwd[10];
   
  passwd[0] = 1;
  passwd[1] = 0;
  passwd[2] = 0;
  passwd[3] = 0;
  passwd[4] = 0;
  passwd[5] = 0;
  passwd[6] = 0;
  passwd[7] = 0;
  passwd[8] = 0;
  passwd[9] = 0;
  i = *  ( DWORD *) passwd;
  if ( i == 1 )
    {
      printf( "Using intel byte ordering.\n" );
    }
  else
    {
      printf( "Using motorola byte ordering.\n" );
    }
}



/******************************
Main function connects gets UIN
and passwd and logins in and sits
in a loop waiting for server responses.
******************************/
int main( int argc, char *argv[] )
{
  int i;
  int next;
  int time_delay = 120; 
  struct timeval tv;
  fd_set readfds;
#ifdef __EMX__
  struct termio tio;
  ioctl(STDIN, TCGETA, &tio);
  tio.c_lflag = tio.c_lflag & ~IDEFAULT;
  ioctl(STDIN, TCSETA, &tio);
#endif

  Get_Config_Info();
  init_prompt();
  printf( "%s", NOCOL ); clrscr();
  printf( "%sMatt's ICQ clone%s compiled on %s %s\n", 
	  SERVCOL, NOCOL, __TIME__, __DATE__ );
  printf( "%sVersion %s%s" " uses GNU Readline %s\n"
	  , SERVCOL, MICQ_VERSION, NOCOL,
	  rl_library_version);
#ifdef FUNNY_MSGS
  printf( "No Mirabilis client was maimed, hacked, tortured, "\
	  "sodomized or otherwise harmed\nin the making of this utility.\n" );
#else
  printf( "This program was made without any help from Mirabilis "
	  "or their consent.\n" );
  printf( "Not reverse engineering or decompilation of any "
	  "Mirabilis code took place\nto make this program.\n" );
#endif
  if ( !strcmp( passwd,"" ) )
    {
      /* We should write a readline display function that echoes nice stars */
      Echo_Off();
      completed_command_line=readline("Enter password : "); 
      Echo_On();
      printf("\n");
      strncpy(passwd,completed_command_line,sizeof(passwd)-1);
      passwd[sizeof(passwd)-1]='\0'; /* anyone using large pwd? */
      /* always erase pwd from memory if not needed */
      memset(completed_command_line, FALSE, strlen(completed_command_line) );
      free(completed_command_line);
    }
  memset( serv_mess, FALSE, 1024 );
  if (argc > 1 )
    {
      for ( i=1; i< argc; i++ )
	{
	  if ( argv[i][0] != '-' )
	    ;
	  else if ( (argv[i][1] == 'v' ) || (argv[i][1] == 'V' ) )
	    {
	      Verbose = ! Verbose;
	    }
	}
    }
  Check_Endian();
  Logon();
  next = time( NULL );
  next += time_delay/2;
  Prompt=ModePrompt[0];
  rl_callback_handler_install(Prompt, &rl_cmd_handler); 
  for ( ; !Quit; )
    {
#ifdef UNIX
      tv.tv_sec = 2;
      tv.tv_usec = 500000;
#else
      tv.tv_sec = 0;
      tv.tv_usec = 100000;
#endif
      FD_ZERO(&readfds);
      FD_SET(sok, &readfds);
#ifndef _WIN32
      FD_SET(STDIN, &readfds);
#endif
      /* don't care about writefds and exceptfds: */
      select(sok+1, &readfds, NULL, NULL, &tv);
      if (FD_ISSET(sok, &readfds))
	{
	  clear_line();
	  Handle_Server_Response( sok );
	  rl_forced_update_display();
	}
#if _WIN32
      else if (_kbhit())		
	/* sorry, this is a bit ugly...   [UH]*/
#else      
	else if (FD_ISSET( STDIN, &readfds ) )
#endif
	  {
	    rl_callback_read_char(); 
	    if (completed_command_line!=NULL)
	      {  
		Get_Input(sok,completed_command_line);
		completed_command_line=NULL;
		if (!Quit)
		  /* reinstall for next command */
		  rl_callback_handler_install(Prompt, &rl_cmd_handler); 
	      }
	  }
      /*          printf("Timed out.\n");*/
      if ( time( NULL ) > next )
	{	  
	  next = time( NULL ) + time_delay;
	  if (Verbose) clear_line();
	  Keep_Alive( sok );
	  if (Verbose) rl_forced_update_display();
	}
    }
  Quit_ICQ( sok );
  return 0;
}

  
