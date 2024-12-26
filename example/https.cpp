#include <nodepp/nodepp.h>
#include <nodepp/url.h>
#include <torify/https.h>

using namespace nodepp;

void onMain() {

    ssl_t ssl;
    
    tor_fetch_t args;
    args.timeout = 0; // Disable Fetch timeout
    args.method  = "GET";
    args.tor     = "tcp://localhost:9050";
    args.url     = "https://check.torproject.org/";
    args.headers = header_t({
        { "host", url::hostname( args.url ) }
    });

    tor::https::fetch( args, &ssl )

    .then([]( https_t cli ){
        console::log( cli.read() );
    })

    .fail([]( except_t err ){
        console::log(err);
    });

}
