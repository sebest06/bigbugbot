#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>

#include <map>
#include <errno.h>

#include <stdarg.h>
#include <stdlib.h>

#include "../include/libircclient.h"
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "plan.h"

/*
 * We store data in IRC session context.
 */

std::string direccion = "tcp://127.0.0.1:3306";
std::string usuario = "root";
std::string password = "";

typedef struct
{
	char 	* channel;
	char 	* nick;
	std::map <std::string, std::string> userLogged;//nick, usuario

} irc_ctx_t;


void event_part (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
    char nickbuf[128];

	irc_ctx_t * ctx = (irc_ctx_t *) irc_get_ctx (session);
	irc_target_get_nick (origin, nickbuf, sizeof(nickbuf));

	if ( ctx->userLogged.find(nickbuf) != ctx->userLogged.end() )
	{
	    ctx->userLogged.erase (nickbuf);
	}

}

void event_connect (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	irc_ctx_t * ctx = (irc_ctx_t *) irc_get_ctx (session);
	irc_cmd_join (session, ctx->channel, 0);
}


void event_nick (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	char nickbuf[128];

	irc_ctx_t * ctx = (irc_ctx_t *) irc_get_ctx (session);

	if ( !origin || count != 1 )
		return;

	irc_target_get_nick (origin, nickbuf, sizeof(nickbuf));

	if ( ctx->userLogged.find(nickbuf) != ctx->userLogged.end() )
	{
		ctx->userLogged[params[0]] = ctx->userLogged[nickbuf];
		ctx->userLogged.erase (nickbuf);
	}
}


void event_channel (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{

}


void event_numeric (irc_session_t * session, unsigned int event, const char * origin, const char ** params, unsigned int count)
{
	if ( event > 400 )
	{
		std::string fulltext;
		for ( unsigned int i = 0; i < count; i++ )
		{
			if ( i > 0 )
				fulltext += " ";

			fulltext += params[i];
		}

		printf ("ERROR %d: %s: %s\n", event, origin ? origin : "?", fulltext.c_str());
	}
}

void event_privmsg(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{

    char *pch;
    std::string comandos = params[1];
    std::vector<std::string> notificar;

    sql::Driver *driver;
    sql::Connection *con;
    sql::Statement *stmt;
    sql::ResultSet *res;

    driver = get_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "root", "");


    if ( !origin || count != 2 )
    return;

    irc_ctx_t * ctx = (irc_ctx_t *) irc_get_ctx (session);

    char nickbuf[128], text[1024];
    irc_target_get_nick (origin, nickbuf, sizeof(nickbuf));

    pch = strtok ((char *)params[1]," ,.-/\r\n");

    if(pch != NULL)
    {
        if(strcmp(pch,"login") == 0)
        {
            char *arg1;
            char *arg2;

            pch = strtok (NULL," ,.-/\r\n");
            if(pch == NULL){return;}
            arg1 = pch;

            pch = strtok (NULL," ,.-/\r\n");
            if(pch == NULL){return;}
            arg2 = pch;

            con->setSchema("bigbug");
            stmt = con->createStatement();
            char querysql[256];
            sprintf (querysql, "SELECT * from users where usuario = '%s' and clave = md5('%s');", arg1,arg2);
            res = stmt->executeQuery(querysql);
            while (res->next()) {

            if ( ctx->userLogged.find(nickbuf) == ctx->userLogged.end() )
            ctx->userLogged[nickbuf] = arg1;

                sprintf (text, "%s, estas loggeado.", nickbuf);
                irc_cmd_notice (session,  nickbuf, text);
            }
            delete res;
            delete stmt;
            delete con;
        }

        else if(strcmp(pch,"amilogin") == 0)
        {
            if ( ctx->userLogged.find(nickbuf) != ctx->userLogged.end() )
            {
                sprintf (text, "%s, estas loggeado. Usuario: %s", nickbuf,ctx->userLogged[nickbuf].c_str());
                irc_cmd_notice (session,  nickbuf, text);
            }
        }
        else if(strcmp(pch,"plan") == 0)
        {
            if ( ctx->userLogged.find(nickbuf) != ctx->userLogged.end() )
            {
                Plan *plan = new Plan(direccion, usuario, password);
                notificar = plan->procesar(ctx->userLogged[nickbuf],comandos.c_str());
                for(size_t i=0; i<notificar.size(); i++)
                {
                    irc_cmd_notice (session,  nickbuf, irc_color_convert_to_mirc(notificar[i].c_str()));
                }
                delete plan;
            }
        }


    }


}

int main (int argc, char **argv)
{

  	irc_callbacks_t	callbacks;
	irc_ctx_t ctx;
	unsigned short port = 6667;

	if ( argc != 4 )
	{
		printf ("Usage: %s <[#]server[:port]> <nick> <channel>\n", argv[0]);
		return 1;
	}

	// Initialize the callbacks
	memset (&callbacks, 0, sizeof(callbacks));

	// Set up the callbacks we will use
	callbacks.event_connect = event_connect;
	callbacks.event_channel = event_channel;
	callbacks.event_nick = event_nick;
	callbacks.event_numeric = event_numeric;
	callbacks.event_part =  event_part;
	callbacks.event_quit = event_part;

	//callbacks.event_join = event_join;
	callbacks.event_privmsg = event_privmsg;

	// And create the IRC session; 0 means error
	irc_session_t * s = irc_create_session (&callbacks);

	if ( !s )
	{
		printf ("Could not create IRC session\n");
		return 1;
	}

	ctx.channel = argv[3];
    ctx.nick = argv[2];
	irc_set_ctx (s, &ctx);

	// If the port number is specified in the server string, use the port 0 so it gets parsed
	if ( strchr( argv[1], ':' ) != 0 )
		port = 0;

	// To handle the "SSL certificate verify failed" from command line we allow passing ## in front
	// of the server name, and in this case tell libircclient not to verify the cert
	if ( argv[1][0] == '#' && argv[1][1] == '#' )
	{
		// Skip the first character as libircclient needs only one # for SSL support, i.e. #irc.freenode.net
		argv[1]++;

		irc_option_set( s, LIBIRC_OPTION_SSL_NO_VERIFY );
	}

	// Initiate the IRC server connection
	if ( irc_connect (s, argv[1], port, "lm7805", argv[2], 0, 0) )
	{
		printf ("Could not connect: %s\n", irc_strerror (irc_errno(s)));
		return 1;
	}

	// and run into forever loop, generating events
	if ( irc_run (s) )
	{
		printf ("Could not connect or I/O error: %s\n", irc_strerror (irc_errno(s)));
		return 1;
	}



	return 0;
}
