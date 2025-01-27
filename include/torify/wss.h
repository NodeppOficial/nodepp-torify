/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TORIFY_WSS
#define NODEPP_TORIFY_WSS

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/wss.h>
#include "https.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace torify { namespace wss {

    tls_torify_t client( const string_t& uri, const ssl_t* ssl, torify_agent_t* opt=nullptr ){
    tls_torify_t srv ( [=]( ssocket_t /*unused*/ ){}, ssl, opt ); 
        srv.connect( url::hostname(uri), url::port(uri) );
        srv.onSocket.once([=]( ssocket_t cli ){
            auto hrv = type::cast<https_t>(cli);
            if ( !_ws_::client( hrv, uri ) ){ return; }
            
            cli.onDrain.once([=](){ cli.free(); cli.onData.clear(); });
            ptr_t<_file_::read> _read = new _file_::read;
            cli.set_timeout(0);

            srv.onConnect.once([=]( wss_t ctx ){ process::poll::add([=](){
                if(!cli.is_available() )    { cli.close(); return -1; }
                if((*_read)(&ctx)==1 )      { return 1; }
                if(  _read->state<=0 )      { return 1; }
                ctx.onData.emit(_read->data); return 1;
            }); });

            process::task::add([=](){
                cli.resume(); srv.onConnect.emit(cli); return -1;
            });
            
        });
    
    return srv; }

}}}

#endif
