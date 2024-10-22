#ifndef NODEPP_TOR_FETCH_HTTPS
#define NODEPP_TOR_FETCH_HTTPS

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>
#include <nodepp/https.h>

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TOR_FETCH_T
#define NODEPP_TOR_FETCH_T
namespace nodepp { struct tor_fetch_t : public fetch_t {
    string_t tor = "tcp://localhost:9050";
};}
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace tor { namespace https {

    promise_t<https_t,except_t> fetch ( const tor_fetch_t& cfg, ssl_t* ctx, agent_t* opt=nullptr ) { 
           auto agn = type::bind( opt==nullptr?agent_t():*opt ); 
           auto gfc = type::bind( cfg ); 
           auto ssl = type::bind( ctx );
    return promise_t<https_t,except_t>([=]( function_t<void,https_t> res, function_t<void,except_t> rej ){

        if( !url::is_valid( gfc->url ) ){ rej(except_t("invalid URL")); return; }
        
        url_t    uri = url::parse( gfc->url );
        string_t dip = uri.hostname ;
        string_t dir = uri.pathname + uri.search + uri.hash;
       
        auto client = tcp_t ([=]( socket_t cli ){ int c = 0; cli.set_timeout( gfc->timeout );

            if( ssl->create_client() == -1 )
              { rej(except_t("Error Initializing SSL context")); return; }

            cli.write( ptr_t<char>({ 0x05, 0x01, 0x00, 0x00 }) );
            if( cli.read(2)!=ptr_t<char>({ 0x05, 0x00, 0x00 }) ){ 
                rej(except_t("Error while Handshaking Sock5"));
                cli.close(); return; 
            }

            int len = (int) dip.size(); int prt = (int) uri.port; 

            cli.write( ptr_t<char>({ 0x05, 0x01, 0x00, 0x03, len, 0x00 }) );
            cli.write( dip ); cli.write( ptr_t<char>({ 0x00, prt, 0x00 }) );
            cli.read();

            https_t sk = *type::cast<ssocket_t>(&cli); 
            sk.ssl = new ssl_t( *ssl, cli.get_fd() );
            sk.ssl->set_hostname( dip );

            if( sk.ssl->connect() <= 0 ){ 
                rej(except_t("Error while handshaking TLS"));
                sk.close(); return; 
            }   cli.write_header( gfc, dir );

            while(( c=sk.read_header() )>0 ){ process::next(); }
            if( c==0 ){ res( sk ); return; } else { 
                rej(except_t("Could not connect to server"));
                sk.close(); return; 
            }

        }, &agn );

        client.onError([=]( except_t error ){ rej(error); });
        
        client.connect( 
            url::hostname(gfc->tor), 
            url::port(gfc->tor) 
        );

    }); }

}}}
    
/*────────────────────────────────────────────────────────────────────────────*/

#endif