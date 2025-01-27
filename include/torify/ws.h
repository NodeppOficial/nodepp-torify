/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TORIFY_WS
#define NODEPP_TORIFY_WS

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/ws.h>
#include "http.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace torify { namespace ws {

    tcp_torify_t client( const string_t& uri, torify_agent_t* opt=nullptr ){ 
    tcp_torify_t srv ( [=]( socket_t /*unused*/ ){}, opt ); 
        srv.connect( url::hostname(uri), url::port(uri) );
        srv.onSocket.once([=]( socket_t cli ){
            auto hrv = type::cast<http_t>(cli);
            if ( !_ws_::client( hrv, uri ) ){ return; }

            cli.onDrain.once([=](){ cli.free(); cli.onData.clear(); }); 
            ptr_t<_file_::read> _read = new _file_::read;
            cli.set_timeout(0);

            srv.onConnect.once([=]( ws_t ctx ){ process::poll::add([=](){
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