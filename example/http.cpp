#include <nodepp/nodepp.h>
#include <nodepp/url.h>
#include <torify/http.h>

using namespace nodepp;

void onMain() {

    tor_fetch_t args;
    args.timeout = 0; // Disable Fetch timeout
    args.method  = "GET";
    args.tor     = "tcp://localhost:9050";
    args.url     = "http://check.torproject.org/";
    args.headers = header_t({
        { "host", url::hostname( args.url ) }
    });

    tor::http::fetch( args )

    .then([]( http_t cli ){
        console::log( cli.read() );
    })

    .fail([]( except_t err ){
        console::log(err);
    });

}
