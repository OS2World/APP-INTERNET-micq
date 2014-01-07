/*
Send Message Function for ICQ... 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Author : zed@mentasm.com
*/
#include "datatype.h"
#include "micq.h"
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
  #include <winsock2.h>
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/socket.h>
#endif
#include <string.h>
#include <assert.h>

/***************************************************
Sends a message thru the server to uin.  Text is the
message to send.
****************************************************/
void icq_sendmsg( SOK_T sok, DWORD uin, char *text, DWORD msg_type)
{
	SIMPLE_MESSAGE msg;
	net_icq_pak pak;
	int size, len; 

	log_event("You sent instant message",text,uin);
	rus_conv ("kw",text);
	

	len = strlen(text);
	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_SENDM );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );

	DW_2_Chars( msg.uin, uin );
	Word_2_Chars(msg.type, msg_type);
	Word_2_Chars( msg.len, len + 1 );		/* length + the NULL */

	memcpy(&pak.data, &msg, sizeof( msg ) );
	memcpy(&pak.data[8], text, len + 1);

	size = sizeof( msg ) + len + 1;

	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
	last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
}

/**************************************************
Sends a authorixation to the server so the Mirabilis
client can add the user.
***************************************************/
void icq_sendauthmsg( SOK_T sok, DWORD uin)
{
	SIMPLE_MESSAGE msg;
	net_icq_pak pak;
	int size; 

	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_SENDM );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );

	DW_2_Chars( msg.uin, uin );
	DW_2_Chars( msg.type, AUTH_MESSAGE );		/* A type authorization msg*/
	Word_2_Chars( msg.len, 2 );		

	memcpy(&pak.data, &msg, sizeof( msg ) );

   pak.data[ sizeof(msg) ]=0x03;
   pak.data[ sizeof(msg) + 1]=0x00;

	size = sizeof( msg ) + 2;

	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
	last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
}

/***************************************************
Changes the users status on the server
****************************************************/
void icq_change_status( SOK_T sok, DWORD status )
{
	net_icq_pak pak;
	int size ;


	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_STATUS_CHANGE );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );

   DW_2_Chars( pak.data, status);
   Current_Status = status;

	size = 4;

	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
	last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
}

/******************************************************************
Logs off ICQ should handle other cleanup as well
********************************************************************/
void Quit_ICQ( int sok )
{
	net_icq_pak pak;
	int size, len;
   
	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_SEND_TEXT_CODE );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );
   
   len = strlen( "B_USER_DISCONNECTED" ) + 1;
   *(short * ) pak.data = len;
   size = len + 4;
   
   memcpy( &pak.data[2], "B_USER_DISCONNECTED", len );
   pak.data[ 2 + len ] = 05;
   pak.data[ 3 + len ] = 00;

   SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);

   SOCKCLOSE(sok);
   SOCKCLOSE( sok );
   last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
}

/*********************************************************
Sends a request to the server for info on a specific user
**********************************************************/
void send_info_req( SOK_T sok, DWORD uin )
{
	net_icq_pak pak;
	int size ;


	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_INFO_REQ );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );

   Word_2_Chars( pak.data , seq_num++ );
   DW_2_Chars( pak.data + 2, uin );

	size = 6;

	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
	last_cmd[seq_num - 2 ] = Chars_2_Word( pak.head.cmd );
}

/*********************************************************
Sends a request to the server for info on a specific user
**********************************************************/
void send_ext_info_req( SOK_T sok, DWORD uin )
{
	net_icq_pak pak;
	int size ;


	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_EXT_INFO_REQ );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );

   Word_2_Chars( pak.data , seq_num++ );
   DW_2_Chars( pak.data + 2, uin );

	size = 6;

	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
	last_cmd[seq_num - 2 ] = Chars_2_Word( pak.head.cmd );
}

/***************************************************************
Initializes a server search for the information specified
****************************************************************/
void start_search( SOK_T sok, char *email, char *nick, char* first, char* last )
{
	net_icq_pak pak;
	int size ;

	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_SEARCH_USER );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );

   Word_2_Chars( pak.data , seq_num++ );
   size = 2;
   Word_2_Chars( pak.data + size, strlen( nick ) + 1 );
   size += 2;
   strcpy( pak.data + size , nick );
   size += strlen( nick ) + 1;
   Word_2_Chars( pak.data + size, strlen( first ) + 1 );
   size += 2;
   strcpy( pak.data + size , first );
   size += strlen( first ) + 1;
   Word_2_Chars( pak.data + size, strlen( last ) + 1);
   size += 2;
   strcpy( pak.data + size , last );
   size += strlen( last ) +1 ;
   Word_2_Chars( pak.data + size, strlen( email ) + 1  );
   size += 2;
   strcpy( pak.data + size , email );
   size += strlen( email ) + 1;
	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
	last_cmd[seq_num - 2 ] = Chars_2_Word( pak.head.cmd );
}

/***************************************************
Registers a new UIN in the ICQ network
****************************************************/
void reg_new_user( SOK_T sok, char *pass)
{
	srv_net_icq_pak pak;
   char len_buf[2];
	int size, len; 

	len = strlen(pass);
	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_REG_NEW_USER );
	Word_2_Chars( pak.head.seq, seq_num++ );
   Word_2_Chars( len_buf, len );

	memcpy(&pak.data, "\x02\x00", 2 );
   memcpy(&pak.data[2], len_buf, 2 );
	memcpy(&pak.data[4], pass, len + 1);
   DW_2_Chars( &pak.data[4+len], 0x0072 );
   DW_2_Chars( &pak.data[8+len], 0x0000 );

	size = len + 12;

	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
	last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
}


void Update_User_Info( SOK_T sok, USER_INFO_PTR user)
{
	net_icq_pak pak;
	int size ;

	Word_2_Chars( pak.head.ver, ICQ_VER );
/*        Word_2_Chars( pak.head.cmd, CMD_NEW_USER_INFO ); */
	Word_2_Chars( pak.head.cmd, CMD_UPDATE_INFO );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );

   Word_2_Chars( pak.data , seq_num++ );
   size = 2;
   Word_2_Chars( pak.data + size, strlen( user->nick ) + 1 );
   size += 2;
   strcpy( pak.data + size , user->nick );
   size += strlen( user->nick ) + 1;
   Word_2_Chars( pak.data + size, strlen( user->first ) + 1 );
   size += 2;
   strcpy( pak.data + size , user->first );
   size += strlen( user->first ) + 1;
   Word_2_Chars( pak.data + size, strlen( user->last ) + 1);
   size += 2;
   strcpy( pak.data + size , user->last );
   size += strlen( user->last ) +1 ;
   Word_2_Chars( pak.data + size, strlen( user->email ) + 1  );
   size += 2;
   strcpy( pak.data + size , user->email );
   size += strlen( user->email ) + 1;
   pak.data[ size ] = user->auth;
   size++;
	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
	last_cmd[ ( seq_num - 1 ) & 0x3ff ] = Chars_2_Word( pak.head.cmd );
}

void icq_sendurl( SOK_T sok, DWORD uin, char *description, char *url )
{
   char buf[450];
   
   sprintf( buf, "%s\xFE%s", url, description );
   icq_sendmsg( sok, uin, buf, URL_MESS );
}

/***************************************************************
This handles actually sending a packet after it's been assembled.
When V5 is implemented this will wrinkle the packet and calculate 
the checkcode.
****************************************************************/
size_t SOCKWRITE( SOK_T sok, void * ptr, size_t len )
{
#if 1
      if ( Verbose > 1 )
         Hex_Dump( ptr, len );
#endif 
#ifdef SPAM
   /* Kinda cheap way of hopefully making connection more stable */
   /*        :)                                                  */
   sockwrite( sok, ptr, len );
   sockwrite( sok, ptr, len );
   sockwrite( sok, ptr, len );
   sockwrite( sok, ptr, len );
   assert( sockwrite( sok, ptr, len ) == len );
#endif
   return sockwrite( sok, ptr, len );
}

size_t SOCKREAD( SOK_T sok, void * ptr, size_t len )
{
   size_t sz;
   
   sz = sockread( sok, ptr, len );
   if ( ( Verbose > 2 ) && ( sz > 0 ) )
      Hex_Dump( ptr, sz );
   return sz;
}
