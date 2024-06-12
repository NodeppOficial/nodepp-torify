#include <nodepp/nodepp.h>
#include <nodepp/fs.h>
#include <https.h>

using namespace nodepp;

void onMain() {

    ssl_t ssl ( "ssl/cert.key", "ssl/cert.crt" );

    tor_fetch_t args;
    args.timeout = 0; // disable timeout
    args.url     = "https://check.torproject.org/";
    args.headers = header_t({
        { "host", "check.torproject.org" }
    });

    tor::https::fetch( args, &ssl )

    .then([]( https_t cli ){
        console::log( cli.read() );
    })

    .fail([]( except_t err ){
        console::log(err);
    });

}