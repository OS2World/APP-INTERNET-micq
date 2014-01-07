/*********************************************
**********************************************
This file has the "ui" functions that read input
and send messages etc.
This software is provided AS IS to be used in
whatever way you see fit and is placed in the
public domain.
Author : Matthew Smith April 23, 1998
Contributors : Nicolas Sahlqvist April 27, 1998
               Michael Ivey May 4, 1998
               Ulf Hedlund -- Windows Support
               Michael Holzt May 5, 1998
Changes :
22-6-98 Added the save and alter command and the
        new implementation of auto
**********************************************
**********************************************/
#define color_cmd "color"
#define sound_cmd "sound"
#include "micq.h"
#include "datatype.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <winsock2.h>
#elif defined (__EMX__)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#define strcasecmp stricmp
#else
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "ui.h"

USER_INFO_STRUCT user;


BOOL Do_Multiline( SOK_T sok, char *buf )
{
  static int offset=0;
  static char msg[1024];
   
  msg[ offset ] = 0;
  if ( strcmp( buf, END_MSG_STR ) == 0 )
    {
      icq_sendmsg( sok, multi_uin, msg, NORM_MESS );
      printf( "Message sent to " );
      Print_UIN_Name( multi_uin );
      printf( "\n");
      last_uin = multi_uin;
      offset = 0;
      return FALSE;
    }
  else if ( strcmp( buf, CANCEL_MSG_STR ) == 0 )
    {
      printf( "Message canceled!\n" );
      last_uin = multi_uin;
      offset = 0;
      return FALSE;
    }
  else
    {
      if ( offset + strlen( buf ) < 1024 )
	{
	  strcat( msg, buf );
	  strcat( msg, "\r\n" );
	  offset += strlen( buf ) + 2;
	  return TRUE;
	}
      else
	{
	  printf( "Message sent before last line buffer is full\n" );
	  printf( "Message sent to " );
	  Print_UIN_Name( multi_uin );
	  printf( "\n");
	  icq_sendmsg( sok, multi_uin, msg, NORM_MESS );
	  last_uin = multi_uin;
	  offset = 0;
	  return FALSE;
	}
    }
}

void Info_Update( SOK_T sok, char *buf )
{
  switch ( status ) {
  case NEW_NICK:
    user.nick = strdup( buf );
    printf( "Enter Your New First Name : ");
    fflush(stdout);
    status++;
    break;
  case NEW_FIRST:
    user.first = strdup( buf );
    printf( "Enter Your New Last Name : ");
    fflush(stdout);
    status++;
    break;
  case NEW_LAST:
    user.last = strdup( buf );
    printf( "Enter Your New Email Address : ");
    fflush(stdout);
    status++;
    break;
  case NEW_EMAIL:
    user.email = strdup( buf );
    printf( "Do you want to require Mirabilis users to request "
	    "your authorization? : ");
    fflush(stdout);
    status++;
    break;
  case NEW_AUTH:
    if ( ! strcasecmp( buf, NO_STR ) ) {
      user.auth = FALSE;
      Update_User_Info( sok, &user );
      /*         free( user.nick );
		 free( user.last );
		 free( user.first );
      free( user.email ); */
      status = 0;
    }
    else if ( ! strcasecmp( buf, YES_STR ) )
      {
	user.auth = TRUE;
	Update_User_Info( sok, &user );
	free( user.nick );
	free( user.last );
	free( user.first );
	free( user.email );
	status = 0;
      }
    else
      {
	printf( "Please enter %s or %s.\n", YES_STR, NO_STR );
      	printf( "Do you want to require Mirabilis users to "
		"request your authorization? : ");
      	fflush(stdout);
      }
    break;
  }
}


BOOL Do_Multiline_All( SOK_T sok, char *buf )
{
  static int offset=0;
  static char msg[1024];
  int i;
   
  msg[ offset ] = 0;
  if ( strcmp( buf, END_MSG_STR ) == 0 )
    {
      for ( i=0; i < Num_Contacts; i++ )
        icq_sendmsg( sok, Contacts[i].uin, msg, MRNORM_MESS );
      printf( "Message sent!\n" );
      offset = 0;
      return FALSE;
    }
  else if ( strcmp( buf, CANCEL_MSG_STR ) == 0 )
    {
      printf( "Message canceled!\n" );
      offset = 0;
      return FALSE;
    }
  else
    {
      if ( offset + strlen( buf ) < 1024 )
	{
	  strcat( msg, buf );
	  strcat( msg, "\r\n" );
	  offset += strlen( buf ) + 2;
	  return TRUE;
	}
      else
	{
	  printf( "Message sent before last line buffer is full\n" );
	  for ( i=0; i < Num_Contacts; i++ )
	    icq_sendmsg( sok, Contacts[i].uin, msg, MRNORM_MESS );
	  offset = 0;
	  return FALSE;
	}
    }
}

/******************************************************
Takes a line of input and processes it.
*******************************************************/
void Get_Input( SOK_T sok, char * buf )
{
  char *cmd;
  char *arg1;
  char *arg2;
  int i;
   
  if ( status == 1 )
    {
      if ( ! Do_Multiline( sok, buf ) )
	{
	  status = 2;
	  Prompt=ModePrompt[0];	      
	}	      
    }
  else if ( status == 3 )
    {
      if ( ! Do_Multiline_All( sok, buf ) )
	{
	  status = 2;
	  Prompt=ModePrompt[0];	      
	}
    }
  else if ( ( status >= NEW_NICK) && ( status <= NEW_AUTH) )
    {  Info_Update( sok, buf ); }
  else
    {
      if ( buf[0] != 0 )
	{
	  cmd = strtok( buf, " \n\t" );
	  if ( cmd == NULL )
	    return;
	  /* goto's removed and code fixed by Paul Laufer. Enjoy! */
	  if ( ( strcasecmp( cmd , "quit" ) == 0 ) ||
	       ( strcasecmp( cmd , "/quit" ) == 0 ) )
	    {
	      Quit = TRUE;
	    }
	  else if ( strcasecmp( cmd, quit_cmd ) == 0 )
	    {
	      Quit = TRUE;
	    }
	  else if ( strcasecmp( cmd, sound_cmd ) == 0 )
	    {
	      if ( SOUND_CMD == Sound )
		{
		  Sound = SOUND_ON;
		  printf( "Sound %sON%s.\n",SERVCOL,NOCOL);
		}
	      else if ( SOUND_ON == Sound )
		{
		  Sound = SOUND_OFF;
		  printf( "Sound %sOFF%s.\n",SERVCOL,NOCOL);
		}
	      else if ( SOUND_OFF == Sound )
		{
		  Sound = SOUND_CMD;
		  printf( "Sound %s%s%s.\n",SERVCOL,Sound_Str,NOCOL);
		}
	    }
	  else if ( strcasecmp( cmd, change_cmd ) == 0 )
            {  Change_Function( sok ); }
	  else if ( ! strcasecmp( cmd, color_cmd ) )
            { 
	      Color = !Color;
	      if ( Color )
		{
                  printf( "Color is %sON%s.\n",MESSCOL,NOCOL);
		}
	      else
		{
                  printf( "Color is %sOFF%s.\n",MESSCOL,NOCOL);
		}
            }	 
	  else if ( ! strcasecmp( cmd, online_cmd ) ) /* online command */
	    {   CHANGE_STATUS( STATUS_ONLINE ); }
	  else if ( ! strcasecmp( cmd, away_cmd ) ) /* away command */
	    {   CHANGE_STATUS( STATUS_AWAY ); }
	  else if ( ! strcasecmp( cmd, na_cmd ) ) /* Not Available command */
	    {   CHANGE_STATUS( STATUS_NA ); }
	  else if ( ! strcasecmp( cmd, occ_cmd ) ) /* Occupied command */
	    {   CHANGE_STATUS( STATUS_OCCUPIED ); }
	  else if ( ! strcasecmp( cmd, dnd_cmd ) ) /* Do not Disturb command */
	    {   CHANGE_STATUS( STATUS_DND ); }
	  else if ( ! strcasecmp( cmd, ffc_cmd ) ) /* Free For Chat command */
	    {   CHANGE_STATUS( STATUS_FREE_CHAT ); }
	  else if ( ! strcasecmp( cmd, inv_cmd ) ) /* Invisible command */
	    {   CHANGE_STATUS( STATUS_INVISIBLE ); }
	  else if ( ! strcasecmp( cmd, search_cmd ) )
	    {
	      arg1 = strtok( NULL, "\n" );
	      if ( arg1 == NULL )
		{
		  printf("Must enter email address.\n");
		}
	      else
		{
   	          start_search( sok, arg1, "", "", "" );
		}
	    }
	  else if ( ! strcasecmp( cmd, status_cmd ) ) 
	    {   Show_Status();  }
	  else if ( ! strcasecmp( cmd, list_cmd ) ) 
	    {   Show_Quick_Status();  }
	  else if ( ! strcasecmp( cmd, msga_cmd ) )
	    {
	      status = 3;
	      Prompt=ModePrompt[2];	      
	    }
	  else if ( ! strcasecmp( cmd, reply_cmd ) ) /* reply command */
	    {
	      Reply_Function( sok );
	    }
	  else if ( ! strcasecmp( cmd, "reg" ) )
	    {
	      arg1 = strtok( NULL, "" );
	      if ( arg1 != NULL )
		{
		  reg_new_user( sok, arg1 );
		}
	    }
	  else if ( ! strcasecmp( cmd, again_cmd ) ) /* again command */
	    {  Again_Function( sok ); }
	  else if ( ! strcasecmp( cmd, "clear"))
	    {  clrscr();  }
	  else if ( ! strcasecmp( cmd, info_cmd ) )
	    {  Info_Function( sok ); }
	  else if ( ! strcasecmp( cmd, "ver" ) )
	    {
	      printf( "Micq Version : %s%s%s" " Compiled on %s%s" "\n",
		      MESSCOL, MICQ_VERSION, NOCOL, 
		      __TIME__, __DATE__ );
	    }
	  else if ( ! strcasecmp( cmd, add_cmd ) )
	    {
	      arg1 = strtok( NULL, " \t" );
	      if ( arg1 != NULL )
		{
		  uin = atoi( arg1 );
		  arg1 = strtok( NULL, "" );
		  if ( arg1 != NULL )
		    {
		      Add_User( sok, uin, arg1 );
		      printf( "%s added.\n", arg1 );
		    }
		}
	      else
		{
		  printf( "%sMust specify a nick name%s" "\n",
			  SERVCOL, NOCOL);
		}
	    }
	  else if ( strcasecmp( cmd, "verbose" ) == 0 )
	    {   Verbose_Function(); }
	  else if ( strcasecmp( cmd, "rinfo" ) == 0 )
	    {
	      Print_UIN_Name( last_recv_uin );
	      printf( "'s IP address is " );
	      Print_IP( last_recv_uin );
	      printf( "\tThe port is %d\n",(WORD) Get_Port( last_recv_uin ) );
	      printf( "\n" );
	      send_info_req( sok, last_recv_uin );
	      send_ext_info_req( sok, last_recv_uin );
	    }
	  else if ( ( strcasecmp( cmd, "/help" ) == 0 ) ||/* Help command */
		    ( strcasecmp( cmd, "help" ) == 0 ) )
	    
	    {  Help_Function(); }
	  else if ( strcasecmp( cmd, auth_cmd ) == 0 )
	    {
	      arg1 = strtok( NULL, "" );
	      if ( arg1 == NULL )
		{
		  printf( "Need uin to send to\n" );
		}
	      else
		{
		  uin = nick2uin( arg1 );
		  if ( -1 == uin )
		    {
		      printf( "%s not recognized as a nick name\n", arg1 );
		    }
		  else icq_sendauthmsg( sok, uin );
		}         
	    }
	  else if ( strcasecmp( cmd, message_cmd ) == 0 ) /* "/msg" */
	    {   Message_Function( sok );  }
	  else if ( strcasecmp( cmd, url_cmd ) == 0 ) /* "/msg" */
	    {
	      arg1 = strtok( NULL, " " );
	      if ( arg1 == NULL )
		{
		  printf( "Need uin to send to\n" );
		}
	      else
		{
		  uin = nick2uin( arg1 );
		  if ( uin == -1 )
		    {
		      printf( "%s not recognized as a nick name\n", arg1 );
		    }
		  else
		    {
		      arg1 = strtok( NULL, " " );
		      last_uin = uin;
		      if ( arg1 != NULL )
			{
			  arg2 = strtok(NULL, "");
			  icq_sendurl( sok, uin, arg1, arg2 );
			  printf( "URL sent to ");
			  Print_UIN_Name( last_uin );
			  printf( "!\n");
			} else {
			  printf("Need URL please.\n");
			} 
		    }
		}
	    }
	  else if ( ! strcasecmp( cmd, alter_cmd ) ) /* alter command */
	    {   Alter_Function(); }
	  else if ( ! strcasecmp( cmd, save_cmd ) ) /* save command */
	    {
	      i=Save_RC();
	      if (i==-1)
		printf("Sorry saving your personal reply messages "
		       "went wrong!\n");
	      else
		printf("Your personal settings have been saved!\n");
	    }
	  else if ( ! strcasecmp( cmd, "update" ) )
	    {
	      status = NEW_NICK;
	      printf( "Enter Your New Nickname : " );
	      fflush(stdout);
	    }
	  else if ( strcasecmp( cmd, auto_cmd ) == 0 )
	    {   Auto_Function( sok ); }
	  else 
	    {  printf( "Unknown command %s, type /help for help.\n", cmd ); }
	}
    }
  multi_uin = last_uin;
}

/**************************************************************
most detailed contact list display
***************************************************************/
static void Show_Status( void )
{
  int i;

  printf( W_SEPERATOR );
  printf( "Your status is " );
  Print_Status( Current_Status );
  printf( "\n" );
  /*  First loop sorts thru all offline users */
  printf( W_SEPERATOR );
  printf( "Users offline: \n" );
  for ( i=0; i< Num_Contacts; i++ ) {
    if ( ( S_DWORD )Contacts[ i ].uin > 0 ) {
      if ( Contacts[ i ].status == STATUS_OFFLINE ) {
	if ( Contacts[i].invis_list ) {
	  printf( "%s*%s", SERVCOL, NOCOL );
	} else {
	  printf( " " );
	}
	printf( "%8ld=", Contacts[ i ].uin );
	printf( "%s%-20s\t%s(", CONTACTCOL, Contacts[ i ].nick, MESSCOL );
	Print_Status( Contacts[ i ].status );
	printf( ")%s", NOCOL );
	if ( -1L != Contacts[ i ].last_time )
	  {
	    if ( Contacts[ i ].status  == STATUS_OFFLINE )
	      printf( " Last online at %s", 
		      ctime( (time_t *) &Contacts[ i ].last_time ) );
	    else
	      printf( " Online since %s", 
		      ctime( (time_t *) &Contacts[ i ].last_time )  );
	  }
	else
	  {
	    printf( " Last on-line unknown.\n" );
	    /* if time is unknow they can't be logged on cause we */
	    /* set the time at login */
	  }
      }
    }
  }
  /* The second loop displays all the online users */
  printf( W_SEPERATOR );
  printf( "Users online: \n" );
  for ( i=0; i< Num_Contacts; i++ ) {
    if ( ( S_DWORD )Contacts[ i ].uin > 0 ) {
      if ( Contacts[ i ].status != STATUS_OFFLINE ) {
	if ( Contacts[i].invis_list ) {
	  printf( "%s*%s", SERVCOL, NOCOL );
	} else {
	  printf( " " );
	}
	printf( "%8ld=", Contacts[ i ].uin );
	printf( "%s%-20s\t%s(", CONTACTCOL, Contacts[ i ].nick, MESSCOL );
	Print_Status( Contacts[ i ].status );
	printf( ")%s", NOCOL );
	if ( -1L != Contacts[ i ].last_time )
	  {
	    if ( Contacts[ i ].status  == STATUS_OFFLINE )
	      printf( " Last online at %s", 
		      ctime( (time_t *) &Contacts[ i ].last_time ) );
	    else
	      printf( " Online since %s", 
		      ctime( (time_t *) &Contacts[ i ].last_time )  );
	  }
	else
	  {
	    printf( " Last on-line unknown.\n" );
	    /* if time is unknow they can't be logged on cause we */
	    /* set the time at login */
	  }
      }
    }
  }
  printf( W_SEPERATOR );
}

/***************************************************************
nice clean "w" display
****************************************************************/
void Show_Quick_Status( void )
{
  int i;
   
  printf( "\n\n" W_SEPERATOR );
  printf( "%lu: ", UIN );
  printf( "Your status is " );
  Print_Status( Current_Status );
  printf( "\n" );
  /*  First loop sorts thru all offline users */
  /*  This comes first so that if there are many contacts */
  /*  The online ones will be less likely to scroll off the screen */
  printf( W_SEPERATOR );
  printf( "Users offline: \n" );
  for ( i=0; i< Num_Contacts; i++ ) {
    if ( ( S_DWORD )Contacts[ i ].uin > 0 ) {
      if ( Contacts[ i ].status == STATUS_OFFLINE ) {
	if ( Contacts[i].invis_list ) {
	  printf( "%s*%s", SERVCOL, NOCOL );
	} else {
	  printf( " " );
	}
	printf( "%s%-20s\t%s(" , CONTACTCOL, Contacts[ i ].nick, MESSCOL);
	Print_Status( Contacts[ i ].status );
	printf( ")%s\n", NOCOL);
      }
    }
  }
  /* The second loop displays all the online users */
  printf( W_SEPERATOR );
  printf( "Users online: \n" );
  for ( i=0; i< Num_Contacts; i++ ) {
    if ( ( S_DWORD )Contacts[ i ].uin > 0 ) {
      if ( Contacts[ i ].status != STATUS_OFFLINE ) {
	if ( Contacts[i].invis_list ) {
	  printf( "%s*%s", SERVCOL, NOCOL );
	} else {
	  printf( " " );
	}
	printf( "%s%-20s\t%s(", CONTACTCOL, Contacts[ i ].nick, MESSCOL);
	Print_Status( Contacts[ i ].status );
	printf( ")%s\n", NOCOL);
      }
    }
  }
  printf( W_SEPERATOR );
}

/***************************************************
Do not call unless you've already made a call to strtok()
****************************************************/
static void Change_Function( SOK_T sok )
{
  char *arg1;
  arg1 = strtok( NULL, " \n\r" );
  if ( arg1 == NULL )
    {
      printf( "%sStatus modes: \n", CLIENTCOL );
      printf( "Status online         %d\n", STATUS_ONLINE );
      printf( "Status Away           %d\n", STATUS_AWAY );
      printf( "Status Do not Disturb %d\n", STATUS_DND );
      printf( "Status Not Available  %d\n", STATUS_NA );
      printf( "Status Free for Chat  %d\n", STATUS_FREE_CHAT );
      printf( "Status Occupied       %d\n", STATUS_OCCUPIED );
      printf( "Status Invisible      %d\n", STATUS_INVISIBLE );
      printf( "%s\n", NOCOL);
   }
  else
   {
     icq_change_status( sok, atoi( arg1 ) );
     Print_Status( Current_Status );
     printf( "\n" );
   }
}

/*****************************************************************
Displays help information.
******************************************************************/
static void Help_Function( void )
{
  printf("%sreg password%s\t"
	 "Creates a new UIN with the specified password.\n",MESSCOL,NOCOL);
  printf("%sverbose #%s\tSet the verbosity level ( default = 0 ).\n",
	 MESSCOL,NOCOL);
  printf("%s%s%s\t\tMark as Online.\n",MESSCOL,online_cmd,NOCOL);
  printf("%s%s%s\t\tMark as Away.\n",MESSCOL,away_cmd,NOCOL);
  printf("%s%s%s\t\tMark as Not Available.\n",MESSCOL,na_cmd,NOCOL);
  printf("%s%s%s\t\tMark as Occupied.\n",MESSCOL,occ_cmd,NOCOL);
  printf("%s%s%s\t\tMark as Do not Disturb.\n",MESSCOL,dnd_cmd,NOCOL);
  printf("%s%s%s\t\tMark as Free for Chat.\n",MESSCOL,ffc_cmd,NOCOL);
  printf("%s%s%s\t\tMark as Invisible.\n",MESSCOL,inv_cmd,NOCOL);
  printf("%sclear%s\t\tClears the screen.\n",MESSCOL,NOCOL);
  printf("%s%s%s\t\tDisplays your autoreply status\n",MESSCOL,auto_cmd,NOCOL);
  printf("%s%s [on|off]%s\tToggles sending messages when your status "
	 "is DND, NA, etc.\n",MESSCOL,auto_cmd,NOCOL);
  printf("%s%s message%s\tSets the message to send as an auto reply\n",
	 MESSCOL,auto_cmd,NOCOL);
  printf("%s%s old command new command%s\n\t\tThis command allows you "
	 "to alter your command set on the fly.\n",MESSCOL,alter_cmd,NOCOL);
  printf("%s%s%s\t\tDisplays the current status of everyone on your "
	 "contact list\n",MESSCOL,list_cmd,NOCOL);
  printf("%s%s [#]%s\tChanges your status to the status number.\n\t\t"
	 "Without a number it lists the available modes.\n",
	 MESSCOL,change_cmd,NOCOL);
  printf("%s%s uin%s\tDisplays general info on uin\n",MESSCOL,info_cmd,NOCOL);
  printf("%s%s email@host%s\n\t\tSearches for a ICQ user with the specified "
	 "email address.\n",MESSCOL,search_cmd,NOCOL);
  printf("%s%s%s\t\tUpdates your basic info (email, nickname, etc.)\n",
	 MESSCOL,update_cmd,NOCOL);
  printf("%s%s\t%s\tLogs off and quits\n",MESSCOL,quit_cmd,NOCOL);
  printf("%s%s\t\t%sSends a multiline message to everyone on your list.\n",
	 MESSCOL,msga_cmd,NOCOL);
  printf("%s%s uin\t%sAuthorize uin to add you to their list\n",
	 MESSCOL,auth_cmd,NOCOL);
  printf("%s%s uin/message%s\tSends a message to uin\n",
	 MESSCOL,message_cmd,NOCOL);
  printf("%s%s uin url message%s\tSends a url and message to uin\n",
	 MESSCOL,url_cmd,NOCOL);
  printf("%s%s uin nick%s\tAdds the uin number to your contact list with "
	 "nickname.\n",MESSCOL,add_cmd,NOCOL);
  printf("%s%s message%s\tSends a message to the last person you sent "
	 "a message to\n",MESSCOL,again_cmd,NOCOL);
  printf("%s%s message%s\tReplys to the last person to send you a message\n",
	 MESSCOL,reply_cmd,NOCOL);
  printf("%s%s%s\t\tToggles beeping when recieving new messages.\n",
	 MESSCOL,sound_cmd,NOCOL);
  printf("%s%s%s\t\tToggles displaying colors.\n",MESSCOL,color_cmd,NOCOL);
  printf("%s\tSending a blank message will put the client into multiline "
	 "mode.\n\tUse . on a line by itself to end message.%s\n",
	 CLIENTCOL,NOCOL);
#ifdef FUNNY_MSGS         
  printf("%sFor me it could be auto dnd Don't disturb me! I'm drinking "
	 "beer!%s\n",CLIENTCOL,NOCOL);
#endif   
}

/****************************************************
Retrieves info a certain user
*****************************************************/
static void Info_Function( SOK_T sok )
{
  char * arg1;
  
  arg1 = strtok( NULL, "" );
  if ( arg1 == NULL )
    {
      printf( "Need uin to send to\n" );
      return;
    }
  uin = nick2uin( arg1 );
  if ( -1 == uin )
    {
      printf( "%s not recognized as a nick name\n", arg1 );
      return;
    }
  printf( "%s's IP address is ", arg1 );
  Print_IP( uin );
  printf( "\tThe port is %d\n",(WORD) Get_Port( uin ) );
  printf( "\n" );
  send_info_req( sok, uin );
  send_ext_info_req( sok, uin );
}

/**********************************************************
Handles automatic reply messages
***********************************************************/
static void Auto_Function( SOK_T sok )
{
  char *cmd;
  char *arg1;
  
  cmd = strtok( NULL, "" );
  if ( cmd == NULL )
    {
      printf( "Automatic replies are %s\n", auto_resp ? "On" : "Off" );
      printf( "The Do not disturb message is: %s\n", auto_rep_str_dnd );
      printf( "The Away message is          : %s\n", auto_rep_str_away );
      printf( "The Not available message is  : %s\n", auto_rep_str_na );
      printf( "The Occupied message is      : %s\n", auto_rep_str_occ );
      printf( "The Invisible message is     : %s\n", auto_rep_str_inv );
      return;
   }
  else if ( strcasecmp( cmd, "on" ) == 0 )
    {
      auto_resp = TRUE;
      printf( "Automatic replies are on.\n" );
    }
  else if ( strcasecmp( cmd, "off" ) == 0 )
    {
      auto_resp = FALSE;
      printf( "Automatic replies are off.\n" );
    }
  else
    {
      printf( "Automatic reply setting\n" );
      arg1 = strtok( cmd," ");
      if (arg1 == NULL)
	{
	  printf( "Sorry wrong syntax, can't find a status somewhere.\r\n");
	  return;
	}
      if ( ! strcasecmp (arg1 , dnd_cmd ) )
	{
	  cmd = strtok( NULL,"");
	  strcpy( auto_rep_str_dnd, cmd);
	}
      else if ( !strcasecmp (arg1 ,away_cmd ))
	{
	  cmd = strtok( NULL,"");
	  strcpy( auto_rep_str_away,cmd);
	}
      else if ( !strcasecmp (arg1,na_cmd))
	{
	  cmd =strtok(NULL,"");
	  strcpy(auto_rep_str_na,cmd);
	}
      else if ( !strcasecmp (arg1,occ_cmd))
	{
	  cmd = strtok(NULL,"");
	  strcpy (auto_rep_str_occ,cmd);
	}
      else if (!strcasecmp (arg1,inv_cmd))
	{
	  cmd = strtok(NULL,"");
	  strcpy (auto_rep_str_inv,cmd);
	}
      else
	printf ("Sorry wrong syntax. Read tha help man!\r\n");
    }
}

/*************************************************************
Alters one of the commands
**************************************************************/
static void Alter_Function( void )
{
  char * cmd;
  
  cmd = strtok (NULL," ");
  if ( cmd == NULL )
    {
      printf( "Need a command to alter!\n" );
      return;
    }
  if ( ! strcasecmp(cmd, auto_cmd))
    strncpy(auto_cmd,strtok(NULL," \n\t"),16);
  else if ( !strcasecmp(cmd ,message_cmd))
    strncpy(message_cmd,strtok(NULL," \n\t"),16);
  else if (!strcasecmp(cmd,add_cmd))
    strncpy(add_cmd,strtok(NULL," \n\t"),16);
  else if (!strcasecmp(cmd,info_cmd))
    strncpy(info_cmd,strtok(NULL," \n\t"),16);
  else if (!strcasecmp(cmd,quit_cmd))
    strncpy(quit_cmd,strtok(NULL," \n\t"),16);
  else if (!strcasecmp(cmd,reply_cmd))
    strncpy(reply_cmd,strtok(NULL," \n\t"),16);
  else if (!strcasecmp(cmd,again_cmd))
    strncpy(again_cmd,strtok(NULL," \n\t"),16);
  else if (!strcasecmp(cmd,list_cmd))
    strncpy(list_cmd,strtok(NULL," \n\t"),16);
  else if (!strcasecmp(cmd,away_cmd))
    strncpy(away_cmd,strtok(NULL," \n\t"),16);
  else if (!strcasecmp(cmd,na_cmd))
    strncpy(na_cmd,strtok(NULL," \n\t"),16);
  else if (!strcasecmp(cmd,dnd_cmd))
    strncpy(dnd_cmd,strtok(NULL," \t\n"),16);
  else if (!strcasecmp(cmd,online_cmd))
    strncpy(online_cmd,strtok(NULL," \n\t"),16);
  else if (!strcasecmp(cmd,occ_cmd))
    strncpy(occ_cmd,strtok(NULL," \t\n"),16);
  else if (!strcasecmp(cmd,ffc_cmd))
    strncpy(ffc_cmd,strtok(NULL," \t\n"),16);
  else if (!strcasecmp(cmd,inv_cmd))
    strncpy(inv_cmd,strtok(NULL," \t\n"),16);
  else if (!strcasecmp(cmd,status_cmd))
    strncpy(status_cmd,strtok(NULL," \t\n"),16);
  else if (!strcasecmp(cmd,auth_cmd))
    strncpy(status_cmd,strtok(NULL," \n\t"),16);
  else if (!strcasecmp(cmd,change_cmd))
    strncpy(change_cmd,strtok(NULL," \n\t"),16);
  else if (!strcasecmp(cmd,search_cmd))
    strncpy(search_cmd,strtok(NULL," \t\n"),16);
  else if (!strcasecmp(cmd,save_cmd))
    strncpy(save_cmd,strtok(NULL," \t\n"),16);
  else if (!strcasecmp(cmd,alter_cmd))
    strncpy(alter_cmd,strtok(NULL," \t\n"),16);
  else if (!strcasecmp(cmd,msga_cmd))
    strncpy(msga_cmd,strtok(NULL," \n\t"),16);
  else
    printf("Type help to see your current command, because this "
	   "one you typed wasn't one!\n");
}

/*************************************************************
Processes user input to send a message to a specified user
**************************************************************/
static void Message_Function( SOK_T sok )
{
  char * arg1;
  
  arg1 = strtok( NULL, UIN_DELIMS );
  if ( arg1 == NULL )
    {
      printf( "Need uin to send to\n" );
      return;
    }
  uin = nick2uin( arg1 );
  if ( -1 == uin )
    {
      printf( "%s not recognized as a nick name\n", arg1 );
      return;
    }
  arg1 = strtok( NULL, "" );
  last_uin = uin;
  if ( arg1 != NULL )
    {
      icq_sendmsg( sok, uin, arg1, NORM_MESS );
      printf( "Message sent to ");
      Print_UIN_Name( last_uin );
      printf( "!\n");
    }
  else
    {
      status = 1;
      Prompt=ModePrompt[1];
    }
}

/*******************************************************
Sends a reply message to the last person to message you.
********************************************************/
static void Reply_Function( SOK_T sok )
{
  char * arg1;
  
  if ( last_recv_uin == 0 )
    {
      printf( "Must receive a message first\n" );
      return;
    }
  arg1 = strtok( NULL, "" );
  last_uin = last_recv_uin;
  if ( arg1 != NULL )
    {
      icq_sendmsg( sok, last_recv_uin, arg1, NORM_MESS );
      printf( "Message sent to " );
      Print_UIN_Name( last_recv_uin );
      printf( "\n");
    }
  else
    {
      status = 1;
      Prompt=ModePrompt[1];
    }
}

static void Again_Function( SOK_T sok )
{
  char * arg1;
  
  if ( last_uin == 0 )
    {
      printf( "Must write one message first\n" );
      return;
    }
  arg1 = strtok( NULL, "" );
  if ( arg1 != NULL )
    {
      icq_sendmsg( sok, last_uin, arg1, NORM_MESS );
      printf( "Message sent to ");
      Print_UIN_Name( last_uin );
      printf( "!\n");					 
    } else {
      status = 1;
      Prompt=ModePrompt[1];
    }
}

static void Verbose_Function( void )
{
  char * arg1;
  
  arg1 = strtok( NULL, "" );
  if ( arg1 != NULL )
    {
      Verbose = atoi( arg1 );
    }
  printf( "Verbosity level is %d.\n", Verbose );
}

