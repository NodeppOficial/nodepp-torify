/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TORIFY_TCP
#define NODEPP_TORIFY_TCP

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/tcp.h>

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TOR_FETCH_T
#define NODEPP_TOR_FETCH_T
namespace nodepp { struct torify_agent_t : public agent_t {
    string_t proxy = "tcp://localhost:9050";
};}
namespace nodepp { struct torify_fetch_t : public fetch_t {
    string_t proxy = "tcp://localhost:9050";
};}
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

/*────────────────────────────────────────────────────────────────────────────*/

class tcp_torify_t {
protected:

    struct NODE {
        int                       state = 0;
        bool                      chck  = 1;
        torify_agent_t            agent;
        poll_t                    poll ;
        function_t<void,socket_t> func ;
    };  ptr_t<NODE> obj;
    
public: tcp_torify_t() noexcept : obj( new NODE() ) {}

    event_t<socket_t> onConnect;
    event_t<socket_t> onSocket;
    event_t<>         onClose;
    event_t<except_t> onError;
    event_t<socket_t> onOpen;
    
    /*─······································································─*/
    
    tcp_torify_t( decltype(NODE::func) _func, torify_agent_t* opt=nullptr ) noexcept : obj( new NODE() )
         { obj->func=_func; obj->agent=opt==nullptr?torify_agent_t():*opt; }
    
    /*─······································································─*/
    
    void     close() const noexcept { if( obj->state<=0 ){ return; } obj->state=-1; onClose.emit(); }
    
    bool is_closed() const noexcept { return obj == nullptr ? 1 : obj->state <= 0; }
    
    /*─······································································─*/

    void poll( bool chck ) const noexcept { obj->chck = chck; }

    /*─······································································─*/

    void connect( const string_t& host, int port, decltype(NODE::func)* fn=nullptr ) const noexcept {
        if( obj->state == 1 ){ return; } obj->state = 1; if( dns::lookup(host).empty() )
          { _EERROR(onError,"dns couldn't get ip"); close(); return; }
        
        ptr_t<decltype( NODE::func )> cb = type::bind( fn );
        auto self = type::bind( this );

        socket_t sk;
                 sk.SOCK    = SOCK_STREAM; 
                 sk.IPPROTO = IPPROTO_TCP;
                 sk.socket( dns::lookup(
                       url::hostname( obj->agent.proxy ) 
                    ), url::port( obj->agent.proxy )
                ); sk.set_sockopt( self->obj->agent );

        process::task::add([=](){
        coStart

            while( sk._connect() == -2 ){ coNext; } 
            if   ( sk._connect()  <  0 ){ 
                _EERROR(self->onError,"Error while connecting TCP"); 
                self->close(); coEnd; 
            }

            if( self->obj->chck ){ 
                self->obj->poll.push_write( sk.get_fd() );
                while( self->obj->poll.get_last_poll()==nullptr )
                     { self->obj->poll.emit(); coNext; }
            }

            do { int len = (int) host.size();

                sk.write( ptr_t<char>({ 0x05, 0x01, 0x00, 0x00 }) );
                if( sk.read(2)!=ptr_t<char>({ 0x05, 0x00, 0x00 }) ){ 
                    _EERROR(self->onError,"Error while Handshaking Sock5"); 
                    self->close(); coEnd; 
                }

                sk.write( ptr_t<char>({ 0x05, 0x01, 0x00, 0x03, len, 0x00 }) );
                sk.write( host ); sk.write( ptr_t<char>({ 0x00,port, 0x00 }) );
                sk.read();

            } while(0);
            
            sk.onClose.once([=](){ self->close(); }); sk.onOpen.emit(); 
            self->onSocket.emit( sk );      self->onOpen.emit(sk); 
            if( cb != nullptr ){(*cb)(sk);} self->obj->func(sk);
            
        coStop
        });

    }

    void connect( const string_t& host, int port, decltype(NODE::func) cb ) const noexcept { 
         connect( host, port, &cb ); 
    }

};

/*────────────────────────────────────────────────────────────────────────────*/

namespace torify { namespace tcp {

    tcp_torify_t client( const tcp_torify_t& client ){ client.onOpen.once([=]( socket_t cli ){
        cli.onDrain.once([=](){ cli.free(); cli.onData.clear(); });
        ptr_t<_file_::read> _read = new _file_::read;

        process::poll::add([=](){
            if(!cli.is_available() )    { cli.close(); return -1; }
            if((*_read)(&cli)==1 )      { return 1; }
            if(  _read->state<=0 )      { return 1; }
            cli.onData.emit(_read->data); return 1;
        }); 

    }); return client; }

    /*─······································································─*/

    tcp_torify_t client( torify_agent_t* opt=nullptr ){
        auto client = tcp_torify_t( [=]( socket_t /*unused*/ ){}, opt );
        tcp::client( client ); return client; 
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

}

#endif