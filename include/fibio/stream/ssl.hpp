//
//  ssl.hpp
//  fibio
//
//  Created by Chen Xu on 14-3-28.
//  Copyright (c) 2014 0d0a.com. All rights reserved.
//

#ifndef fibio_stream_ssl_hpp
#define fibio_stream_ssl_hpp

#include <boost/asio/ssl.hpp>
#include <fibio/stream/iostream.hpp>

namespace fibio { namespace stream {
    // Acceptor for SSL over stream socket
    template<typename Stream>
    struct stream_acceptor<fiberized_iostream<boost::asio::ssl::stream<Stream>>> {
        typedef fiberized_iostream<boost::asio::ssl::stream<Stream>> stream_type;
        typedef Stream socket_type;
        typedef typename socket_type::protocol_type::acceptor acceptor_type;
        typedef typename socket_type::protocol_type::endpoint endpoint_type;
        
        stream_acceptor(const std::string &s, unsigned short port_num)
        : acc_(asio::get_io_service(),
               endpoint_type(boost::asio::ip::address::from_string(s.c_str()), port_num))
        {}
        
        stream_acceptor(unsigned short port_num)
        : acc_(asio::get_io_service(),
               endpoint_type(boost::asio::ip::address(), port_num))
        {}
        
        stream_acceptor(stream_acceptor &&other)
        : acc_(std::move(other.acc_))
        {}
        
        stream_acceptor(const stream_acceptor &other)=delete;
        stream_acceptor &operator=(const stream_acceptor &other)=delete;
        
        void close()
        { acc_.close(); }
        
        stream_type accept(boost::asio::ssl::context &ctx) {
            stream_type s(ctx);
            boost::system::error_code ec;
            async_accept(s, ec);
            return s;
        }
        
        stream_type accept(boost::asio::ssl::context &ctx, boost::system::error_code &ec) {
            stream_type s(ctx);
            async_accept(s, ec);
            return s;
        }
        
        void accept(stream_type &s) {
            boost::system::error_code ec;
            async_accept(s, ec);
        }
        
        void accept(stream_type &s, boost::system::error_code &ec) {
            acc_.async_accept(s.streambuf().next_layer(), asio::yield[ec]);
            if(ec) return;
            s.streambuf().async_handshake(boost::asio::ssl::stream_base::server, asio::yield[ec]);
        }
        
        stream_type operator()()
        { return accept(); }
        
        stream_type operator()(boost::system::error_code &ec)
        { return accept(ec); }
        
        void operator()(stream_type &s)
        { accept(s); }
        
        void operator()(stream_type &s, boost::system::error_code &ec)
        { accept(s, ec); }
        
        acceptor_type acc_;
    };
}}

namespace fibio { namespace ssl {
    // Introduce some useful types
    using boost::asio::ssl::context;
    using boost::asio::ssl::rfc2818_verification;
    using boost::asio::ssl::verify_context;
    typedef boost::asio::ssl::stream_base::handshake_type handshake_type;
    
    typedef stream::fiberized_iostream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> tcp_stream;
    typedef stream::stream_acceptor<tcp_stream> tcp_stream_acceptor;
}}

#endif
