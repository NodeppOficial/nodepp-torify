#include <nodepp/nodepp.h>
#include <http.h>

using namespace nodepp;

void onMain() {

    tor_fetch_t args;
    args.timeout = 0; // disable timeout
    args.url     = "http://check.torproject.org/";
    args.headers = header_t({
        { "host", "check.torproject.org" }
    });

    tor::http::fetch( args )

    .then([]( http_t cli ){
        console::log( cli.read() );
    })

    .fail([]( except_t err ){
        console::log(err);
    });

}