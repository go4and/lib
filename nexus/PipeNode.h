/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
BOOST_PP_REPEAT_FROM_TO(
        1, BOOST_PP_INC(NEXUS_PACKET_PACKER_MAX_ARITY),
        NEXUS_PIPE_NODE_SEND_DEF, ~)

    void start(const Listener & listener);
    void listen(const std::wstring & name);
    void connect(const std::wstring & name);
    
    bool connected();    
private:
    void finish();
    void shutdown();
    boost::asio::windows::stream_handle & stream();
    void processPackets(nexus::PacketReader & reader);

    void doListen(const std::wstring & name);
    void doConnect(const std::wstring & name);
    void handleConnected(const boost::system::error_code & ec, const std::wstring & name);
    void handleExpired(const boost::system::error_code & ec, const boost::function<void()> & action);

    void connectDone();

    boost::asio::io_service ioService_;
    boost::scoped_ptr<boost::asio::windows::stream_handle> pipe_;
    boost::thread thread_;
    boost::asio::deadline_timer timer_;
    Listener listener_;
    
    friend class Connection<PipeNode>;
};

}
