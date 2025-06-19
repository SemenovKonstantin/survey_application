#ifndef SERVER_H
#define SERVER_H

#include <libwebsockets.h>
#include <string>
#include <vector>
#include <map>
#include "types.h"

class Server {
public:
    Server(int port);
    ~Server();
    void run();
    void broadcast_results(int poll_id);

private:
    static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
    static int callback_websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
    void handle_api_request(const std::string& path, const std::string& method, const std::string& body, std::string& response);
    
    struct lws_context *context;
    int port;
    std::map<std::string, lws *> clients; // WebSocket clients
};

#endif
