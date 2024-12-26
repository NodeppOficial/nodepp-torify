#ifndef NODEPP_TOR_FETCH_HTTP
#define NODEPP_TOR_FETCH_HTTP

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>
#include <nodepp/http.h>

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TOR_FETCH_T
#define NODEPP_TOR_FETCH_T
namespace nodepp { struct tor_fetch_t : public fetch_t {
    string_t tor = "tcp://localhost:9050";
};}
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace tor { namespace http {

    promise_t<http_t,except_t> fetch ( const tor_fetch_t& cfg, agent_t* opt=nullptr ) { 
           auto agn = type::bind( opt==nullptr?agent_t():*opt ); 
           auto gfc = type::bind( cfg ); 
    return promise_t<http_t,except_t>([=]( function_t<void,http_t> res, function_t<void,except_t> rej ){

        if( !url::is_valid( gfc->url ) ){ rej(except_t("invalid URL")); return; }
        
        url_t    uri = url::parse( gfc->url );
        string_t dip = uri.hostname ;
        string_t dir = uri.pathname + uri.search + uri.hash;
       
        auto client = tcp_t ([=]( http_t cli ){ int c = 0; cli.set_timeout( gfc->timeout );

            cli.write( ptr_t<char>({ 0x05, 0x01, 0x00, 0x00 }) );
            if( cli.read(2)!=ptr_t<char>({ 0x05, 0x00, 0x00 }) ){ 
                rej(except_t("Error while Handshaking Sock5"));
                cli.close(); return; 
            }

            int len = (int) dip.size(); int prt = (int) uri.port; 

            cli.write( ptr_t<char>({ 0x05, 0x01, 0x00, 0x03, len, 0x00 }) );
            cli.write( dip ); cli.write( ptr_t<char>({ 0x00, prt, 0x00 }) );
            cli.read(); cli.write_header( gfc, dir );

            while(( c=cli.read_header() )>0 ){ process::next(); }
            if( c==0 ){ res( cli ); return; } else { 
                rej(except_t("Could not connect to server"));
                cli.close(); return; 
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