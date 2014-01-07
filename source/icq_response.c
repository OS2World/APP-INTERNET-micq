#include "datatype.h"
#include "micq.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef _WIN32
  #include <winsock2.h>
#else
  #include <netinet/in.h>
  #include <unistd.h>
  #include <netdb.h>
#endif
#include <time.h>
#include <string.h>


/*********************************
This must be called every 2 min.
so the server knows we're still alive.
JAVA client sends two different commands
so we do also :)
**********************************/
void Keep_Alive( int sok )
{
   net_icq_pak pak;
   
   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_KEEP_ALIVE );
   Word_2_Chars( pak.head.seq, seq_num++ );
   DW_2_Chars( pak.head.UIN, UIN );
   
   SOCKWRITE( sok, &(pak.head.ver), sizeof( pak.head ) - 2 );
   last_cmd[(seq_num - 1) & 0x3ff ] = Chars_2_Word( pak.head.cmd );

   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_KEEP_ALIVE2 );
   Word_2_Chars( pak.head.seq, seq_num++ );
   DW_2_Chars( pak.head.UIN, UIN );
   
   SOCKWRITE( sok, &(pak.head.ver), sizeof( pak.head ) - 2 );
   last_cmd[(seq_num - 1) & 0x3ff ] = Chars_2_Word( pak.head.cmd );
   if ( Verbose )
   {
#ifdef FUNNY_MSGS
     printf( "\nIf you go to" " %sZ'Ha'Dum %s" "you will die!!!!\n",
	     MESSCOL, NOCOL ); 
     /* or if you don't send this packet */
#else
     printf( "\nSend Keep_Alive packet to the server\n" );
#endif
   }
}

void Recv_Message( int sok, srv_net_icq_pak pak )
{
   RECV_MESSAGE_PTR r_mesg;

   printf( "\n" );
   r_mesg = ( RECV_MESSAGE_PTR )pak.data;
   last_recv_uin = Chars_2_DW( r_mesg->uin );
   Print_UIN_Name( Chars_2_DW( r_mesg->uin ) );
   printf( ":\a\nDate %d/%d/%d\t%d:%02d GMT\n", r_mesg->month, r_mesg->day, 
            Chars_2_Word( r_mesg->year ), r_mesg->hour ,  r_mesg->minute );
            
   printf( "Type : %d \t Len : %d\n", Chars_2_Word( r_mesg->type ),
       Chars_2_Word( r_mesg->len ) );
   Do_Msg( sok, Chars_2_Word( r_mesg->type ), Chars_2_Word( r_mesg->len ), ( r_mesg->len + 2 ), last_recv_uin ); 
   
   ack_srv( sok, Chars_2_Word( pak.head.seq ) );
}

/*********************************
This must be called to remove messages
from the server
**********************************/
void snd_got_messages( int sok )
{
   net_icq_pak pak;
   
   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_ACK_MESSAGES );
   Word_2_Chars( pak.head.seq, seq_num++ );
   DW_2_Chars( pak.head.UIN, UIN );
   
   SOCKWRITE( sok, &(pak.head.ver), sizeof( pak.head ) - 2 );
   last_cmd[ (seq_num - 1) & 0x3ff ] = Chars_2_Word( pak.head.cmd );
}

/*************************************
this sends over the contact list
**************************************/
void snd_contact_list( int sok )
{
   net_icq_pak pak;
   int num_used;
   int i, size;
   char *tmp;
   
   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_CONT_LIST );
   Word_2_Chars( pak.head.seq, seq_num++ );
   DW_2_Chars( pak.head.UIN, UIN );
   
   tmp = pak.data;
   tmp++;
   for ( i=0, num_used=0; i < Num_Contacts ; i++ )
   {
      if ( (S_DWORD) Contacts[ i ].uin >  0 )
      {
         DW_2_Chars( tmp, Contacts[i].uin );
         tmp+=4;
         num_used++;
      }
   }
   pak.data[0] = num_used;
   Word_2_Chars( pak.data, num_used );
   size = ( ( int ) tmp - ( int ) pak.data );
   size += sizeof( pak.head ) - 2;
   SOCKWRITE( sok, &(pak.head.ver), size );
   last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
}

/*************************************
this sends over the Invisible list
that allows certain users to see you
if you're invisible.
**************************************/
void snd_invis_list( int sok )
{
   net_icq_pak pak;
   int num_used;
   int i, size;
   char *tmp;
   
   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_INVIS_LIST );
   Word_2_Chars( pak.head.seq, seq_num++ );
   DW_2_Chars( pak.head.UIN, UIN );
   
   tmp = pak.data;
   tmp++;
   for ( i=0, num_used=0; i < Num_Contacts ; i++ )
   {
      if ( (S_DWORD) Contacts[ i ].uin >  0 )
      {
         if ( Contacts[i].invis_list )
         {
            DW_2_Chars( tmp, Contacts[i].uin );
            tmp+=4;
            num_used++;
         }
      }
   }
   if ( num_used != 0 )
   {
      pak.data[0] = num_used;
      size = ( ( int ) tmp - ( int ) pak.data );
      size += sizeof( pak.head ) - 2;
      SOCKWRITE( sok, &(pak.head.ver), size );
      last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
   }
}

/**************************************
This sends the second login command
this is necessary to finish logging in.
***************************************/
void snd_login_1( int sok )
{
   net_icq_pak pak;
   
   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_LOGIN_1 );
   Word_2_Chars( pak.head.seq, seq_num++ );
   DW_2_Chars( pak.head.UIN, UIN );
   
   SOCKWRITE( sok, &(pak.head.ver), sizeof( pak.head ) - 2 );
   last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
}

/************************************************
This is called when a user goes offline
*************************************************/
void User_Offline( int sok, srv_net_icq_pak pak )
{
   int remote_uin;
   int index;

   remote_uin = pak.data[3];
   remote_uin <<=8;
   remote_uin += pak.data[2];
   remote_uin <<=8;
   remote_uin += pak.data[1];
   remote_uin <<=8;
   remote_uin += pak.data[0];

   printf( "\n" );
   printf("%s", CONTACTCOL );
   index = Print_UIN_Name( remote_uin );
   printf("%s", NOCOL );
   printf( " logged off.\t" );
   Time_Stamp();
   printf( "\n" );

   log_event("User logged off","",remote_uin);
   if ( index != -1 )
   {
      Contacts[ index ].status = STATUS_OFFLINE;
      Contacts[ index ].last_time = time( NULL );
   }
   ack_srv( sok, Chars_2_Word( pak.head.seq ) );
}

void User_Online( int sok, srv_net_icq_pak pak )
{
   int remote_uin, new_status;
   int index;

   remote_uin = Chars_2_DW( &pak.data[0] );

   new_status = Chars_2_DW( &pak.data[17] );
   
   if ( Done_Login )
   {
      printf( "\n" );
      printf("%s", CONTACTCOL );
      index = Print_UIN_Name( remote_uin );
      printf("%s", NOCOL );
      if ( index != -1 )
      {
         Contacts[ index ].status = new_status;
         Contacts[ index ].current_ip[0] =  pak.data[4];
         Contacts[ index ].current_ip[1] =  pak.data[5];
         Contacts[ index ].current_ip[2] =  pak.data[6];
         Contacts[ index ].current_ip[3] =  pak.data[7];
         Contacts[ index ].port = Chars_2_DW( &pak.data[8] );
         Contacts[ index ].last_time = time( NULL );
      }
      printf( " (" );
      Print_Status( new_status );
      printf( ") logged on.\t" );
      Time_Stamp();
      printf( "\n" );

      log_event("User logged on","",remote_uin);
      if ( Verbose )
      {
         printf( "The IP address is %u.%u.%u.%u\n", pak.data[4], pak.data[5], pak.data[6], pak.data[7] );
         printf( "The \"real\" IP address is %u.%u.%u.%u\n", pak.data[12], pak.data[13], pak.data[14], pak.data[15] );
      }
   }
   else
   {
      for ( index=0; index < Num_Contacts; index++ )
      {
         if ( Contacts[index].uin == remote_uin )
         {
            Contacts[ index ].status = new_status;
            Contacts[ index ].current_ip[0] =  pak.data[4];
            Contacts[ index ].current_ip[1] =  pak.data[5];
            Contacts[ index ].current_ip[2] =  pak.data[6];
            Contacts[ index ].current_ip[3] =  pak.data[7];
            Contacts[ index ].port = Chars_2_DW( &pak.data[8] );
            Contacts[ index ].last_time = time( NULL );
            break;
         }
      }
   }
   ack_srv( sok, Chars_2_Word( pak.head.seq ) );
}

void Status_Update( int sok, srv_net_icq_pak pak )
{
   int remote_uin, new_status;
   int index;

   remote_uin = pak.data[3];
   remote_uin <<=8;
   remote_uin += pak.data[2];
   remote_uin <<=8;
   remote_uin += pak.data[1];
   remote_uin <<=8;
   remote_uin += pak.data[0];

   new_status = pak.data[7];
   new_status <<=8;
   new_status += pak.data[6];
   new_status <<=8;
   new_status += pak.data[5];
   new_status <<=8;
   new_status += pak.data[4];

   printf( "\n" );
   printf("%s", CONTACTCOL );
   index = Print_UIN_Name( remote_uin );
   printf("%s", NOCOL );
   if ( index != -1 )
   {
      Contacts[ index ].status = new_status;
   }
   printf( " changed status to " );
   Print_Status( new_status );
   printf( "\t" );
   Time_Stamp();
   printf( "\n" );
   
   ack_srv( sok, Chars_2_Word( pak.head.seq ) );
}

/* This procedure logins into the server with UIN and pass
   on the socket sok and gives our ip and port.
   It does NOT wait for any kind of a response.         */
void Login( int sok, int UIN, char *pass, int ip, int port, DWORD status )
{
   net_icq_pak pak;
   int size;
   login_1 s1;
   login_2 s2;
	struct sockaddr_in sin;  /* used to store inet addr stuff  */
   
   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_LOGIN );
   Word_2_Chars( pak.head.seq, seq_num++ );
   DW_2_Chars( pak.head.UIN, UIN );
   
   DW_2_Chars( s1.port, port + 0x10000);
   Word_2_Chars( s1.len, strlen( pass ) + 1 );
   
   DW_2_Chars( s2.ip, ip );
   sin.sin_addr.s_addr = Chars_2_DW( s2.ip );
   DW_2_Chars( s2.status, status );
   Word_2_Chars( s2.seq, seq_num++ );
   
   DW_2_Chars( s2.X1, LOGIN_X1_DEF );
   s2.X2[0] = LOGIN_X2_DEF;
   DW_2_Chars( s2.X3, LOGIN_X3_DEF );
   DW_2_Chars( s2.X4, LOGIN_X4_DEF );
   DW_2_Chars( s2.X5, LOGIN_X5_DEF );
   
   memcpy( pak.data, &s1, sizeof( s1 ) );
   size = 6;
   memcpy( &pak.data[size], pass, Chars_2_Word( s1.len ) );
   size += Chars_2_Word( s1.len );
   memcpy( &pak.data[size], &s2.X1, sizeof( s2.X1 ) );
   size += sizeof( s2.X1 );
   memcpy( &pak.data[size], &s2.ip, sizeof( s2.ip ) );
   size += sizeof( s2.ip );
   memcpy( &pak.data[size], &s2.X2, sizeof( s2.X2 ) );
   size += sizeof( s2.X2 );
   memcpy( &pak.data[size], &s2.status, sizeof( s2.status ) );
   size += sizeof( s2.status );
   memcpy( &pak.data[size], &s2.X3, sizeof( s2.X3 ) );
   size += sizeof( s2.X3 );
   memcpy( &pak.data[size], &s2.seq, sizeof( s2.seq ) );
   size += sizeof( s2.seq );
   memcpy( &pak.data[size], &s2.X4, sizeof( s2.X4 ) );
   size += sizeof( s2.X4 );
   memcpy( &pak.data[size], &s2.X5, sizeof( s2.X5 ) );
   size += sizeof( s2.X5 );
   SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head )- 2 );
   last_cmd[ seq_num - 2 ] = Chars_2_Word( pak.head.cmd );
} 

/* This routine sends the aknowlegement cmd to the
   server it appears that this must be done after
   everything the server sends us                 */
void ack_srv( int sok, int seq )
{
   net_icq_pak pak;
   
   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_ACK );
   Word_2_Chars( pak.head.seq, seq );
   DW_2_Chars( pak.head.UIN, UIN);
   
   SOCKWRITE( sok, &(pak.head.ver), sizeof( pak.head ) - 2 );
}

void Display_Info_Reply( int sok, srv_net_icq_pak pak )
{
   char *tmp;
   int len;
   printf( "%s\nInfo for %ld\n", SERVCOL, Chars_2_DW( &pak.data [2] ) );
   len = Chars_2_Word( &pak.data[6] );
   rus_conv( "wk", &pak.data[8] );
   printf( "Nick Name :\t%s\n", &pak.data[8] );
   tmp = &pak.data[8 + len ];
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   printf( "First name :\t%s\n", tmp+2 );
   tmp += len + 2;
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   printf( "Last name :\t%s\n", tmp+2 );
   tmp += len + 2;
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   printf( "Email Address :\t%s\n", tmp+2 );
   tmp += len + 2;
   if ( *tmp == 1 )
   {
      printf( "No authorization needed.%s" "\n", NOCOL );
   }
   else
   {
      printf( "Must request authorization.%s" "\n", NOCOL );
   }
   ack_srv( sok, Chars_2_Word( pak.head.seq ) );
}

void Display_Ext_Info_Reply( int sok, srv_net_icq_pak pak )
{
   unsigned char *tmp;
   int len;
   printf( "%s\nMore Info for %ld\n", SERVCOL, Chars_2_DW( &pak.data [2] ) );
   len = Chars_2_Word( &pak.data[6] );
   rus_conv( "wk", &pak.data[8] );
   printf( "City         :\t%s\n", &pak.data[8] );
   if ( Get_Country_Name( Chars_2_Word(&pak.data[8+len]) ) != NULL )
      printf( "Country      :\t%s\n",
	      Get_Country_Name( Chars_2_Word(&pak.data[8+len]) ) );
   else
      printf( "Country Code :\t%d\n", Chars_2_Word( &pak.data[8+len] ) );
   printf( "Country Status :\t%d\n", pak.data[len+10]  );
   tmp = &pak.data[11 + len ];
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   printf( "State        :\t%s\n", tmp+2 );
   if ( Chars_2_Word( tmp+2+len ) != 0xffff )
      printf( "Age          :\t%d\n", Chars_2_Word( tmp+2+len ) );
   else
      printf( "Age          :\tNot Entered\n");
   if (*(tmp + len + 4) == 2 )
      printf( "Sex          :\tMale\n" );
   else if (*(tmp + len + 4) == 1 )
      printf( "Sex          :\tFemale\n" );
   else
#ifdef FUNNY_MSGS
      printf( "Sex          :\tYes please!\n" );
#else
      printf( "Sex          :\tNot specified\n" );
#endif
   tmp += len + 5;
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   printf( "Phone Number :\t%s\n", tmp+2 );
   tmp += len + 2;
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   printf( "Home Page    :\t%s\n", tmp+2 );
   tmp += len + 2;
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   printf( "About        :\n%s%s" "\n" , tmp+2, NOCOL);
   ack_srv( sok, Chars_2_Word( pak.head.seq ) );
}

void Display_Search_Reply( int sok, srv_net_icq_pak pak )
{
   char *tmp;
   int len;
   printf( "%s\nUser found %ld\n", SERVCOL, Chars_2_DW( &pak.data [2] ) );
   len = Chars_2_Word( &pak.data[6] );
   rus_conv( "wk", &pak.data[8] );
   printf( "Nick Name :\t%s\n", &pak.data[8] );
   tmp = &pak.data[8 + len ];
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   printf( "First name :\t%s\n", tmp+2 );
   tmp += len + 2;
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   printf( "Last name :\t%s\n", tmp+2 );
   tmp += len + 2;
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   printf( "Email Address :\t%s\n", tmp+2 );
   tmp += len + 2;
   if ( *tmp == 1 )
   {
      printf( "No authorization needed.%s" "\n", NOCOL );
   }
   else
   {
      printf( "Must request authorization.%s" "\n", NOCOL );
   }
   ack_srv( sok, Chars_2_Word( pak.head.seq ) );
}

void Do_Msg( SOK_T sok, DWORD type, WORD len, char * data, DWORD uin )
{
   char *tmp;
	int   x,m;
   char message[1024];
   char url_data[1024];
   char url_desc[1024];

   if ( type == USER_ADDED_MESS )
   {
      tmp = strchr( data, '\xFE' );
      if ( tmp == NULL )
      {
         printf( "Ack!!!!!!!  Bad packet\n" );
         return;
      }
      *tmp = 0;
      printf( "%s\n%s%s" " has added you to their contact list.\n", 
	      CONTACTCOL, data, NOCOL );
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\xFE' );
      if ( tmp == NULL )
      {
         printf( "Ack!!!!!!!  Bad packet\n" );
         return;
      }
      *tmp = 0;
      rus_conv ("wk",data);
      printf( "First name    : " "%s%s%s" "\n",
	      MESSCOL, data, NOCOL );
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\xFE' );
      if ( tmp == NULL )
      {
         printf( "Ack!!!!!!!  Bad packet\n" );
         return;
      }
      *tmp = 0;
      rus_conv ("wk",data);
      printf( "Last name     : " "%s%s%s" "\n", MESSCOL, data, NOCOL );
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\xFE' );
      *tmp = 0;
      rus_conv ("wk",data);
      printf( "Email address : " "%s%s%s" "\n" , MESSCOL, data, NOCOL );
   }
   else if ( type == AUTH_REQ_MESS )
   {
      tmp = strchr( data, '\xFE' );
      *tmp = 0;
      printf( "%s\n%s%s" " has requested your authorization to be "
	      "added to their contact list.\n", CONTACTCOL, data, NOCOL );
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\xFE' );
      if ( tmp == NULL )
      {
         printf( "Ack!!!!!!!  Bad packet\n" );
         return;
      }
      *tmp = 0;
      rus_conv ("wk",data);
      printf( "First name    : " "%s%s%s" "\n" , MESSCOL, data, NOCOL );
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\xFE' );
      if ( tmp == NULL )
      {
         printf( "Ack!!!!!!!  Bad packet\n" );
         return;
      }
      *tmp = 0;
      rus_conv ("wk",data);
      printf( "Last name     : " "%s%s%s" "\n" , MESSCOL, data, NOCOL );
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\xFE' );
      if ( tmp == NULL )
      {
         printf( "Ack!!!!!!!  Bad packet\n" );
         return;
      }
      *tmp = 0;
      rus_conv ("wk",data);
      printf( "Email address : " "%s%s%s" "\n", MESSCOL, data, NOCOL );
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\xFE' );
      if ( tmp == NULL )
      {
         printf( "Ack!!!!!!!  Bad packet\n" );
         return;
      }
      *tmp = 0;
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\x00' );
      if ( tmp == NULL )
      {
         printf( "Ack!!!!!!!  Bad packet\n" );
         return;
      }
      *tmp = 0;
      rus_conv ("wk",data);
      printf( "Reason : " "%s%s%s" "\n" , MESSCOL, data, NOCOL );
   }
   else if (type == URL_MESS)
   {

      tmp = strchr( data, '\xFE' );
      if ( tmp == NULL )
      {
         printf( "Ack!!!!!!!  Bad packet\n" );
         return;
      }
      *tmp = 0;
      rus_conv ("wk",data);
      strcpy (url_desc,data);
      tmp++;
      data = tmp;
      rus_conv ("wk",data);
      strcpy (url_data,data);
      
      sprintf (message,"Description: %s \n                          URL: %s",
	       url_desc,url_data);  
      log_event("You received URL message",message,uin);
      printf( " URL Message.\n Description: " "%s%s%s" "\n", 
	      MESSCOL, url_desc, NOCOL );
      printf( " URL        : " "%s%s%s" "\n", 
	      MESSCOL, url_data, NOCOL );
   }
   else if (type == CONTACT_MESS || type==MRCONTACT_MESS) {
     tmp = strchr( data, '\xFE' );
     *tmp = 0;
     printf( "\nContact List.\n%s========================================="
	     "===\n%s%d Contacts\n", MESSCOL, NOCOL, atoi(data));
      tmp++;
      m = atoi(data);
      for(x=0; m > x ; x++)
      {
         data = tmp;
         tmp = strchr( tmp, '\xFE' );
         *tmp = 0;
         printf( "%s%s\t\t\t", CONTACTCOL, data );
         tmp++;
         data = tmp;
         tmp = strchr( tmp, '\xFE' );
         *tmp = 0;
         printf( "%s%s%s\n" , MESSCOL, data, NOCOL );
         tmp++;
      }
	}
   else
   {
      rus_conv ("wk",data);
      log_event("You received instant message",data,uin);
      printf( "\n%s%s", MESSCOL, data );
      printf( "%s\n", NOCOL );
   }
}
