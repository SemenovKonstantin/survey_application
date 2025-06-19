#include "server.h"
#include "database.h"
#include <jsoncpp/json/json.h>
#include <sstream>
#include <iostream>

static Database db("dbname=polling_app user=postgres password=postgres host=localhost port=5432");
static Server* server_instance = nullptr;

Server::Server(int port) : port(port), context(nullptr) {
    server_instance = this;
    struct lws_protocols protocols[] = {
        {
            "http",
            callback_http,
            0,
            0,
        },
        {
            "poll-protocol",
            callback_websocket,
            0,
            128,
        },
        { NULL, NULL, 0, 0 }
    };

    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = port;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    context = lws_create_context(&info);
    if (!context) {
        std::cerr << "Failed to create LWS context" << std::endl;
        exit(1);
    }
}

Server::~Server() {
    if (context) {
        lws_context_destroy(context);
    }
    server_instance = nullptr;
}

void Server::run() {
    while (true) {
        lws_service(context, 50);
    }
}

void Server::broadcast_results(int poll_id) {
    Poll poll = db.get_poll_results(poll_id);
    Json::Value json;
    json["type"] = "poll_update";
    json["poll_id"] = poll.id;
    json["question"] = poll.question;
    for (const auto& option : poll.options) {
        Json::Value opt;
        opt["id"] = option.id;
        opt["text"] = option.text;
        opt["votes"] = option.vote_count;
        json["options"].append(opt);
    }

    std::string message = Json::writeString(Json::StreamWriterBuilder(), json);
    for (auto& client : clients) {
        lws_write(client.second, (unsigned char*)message.c_str(), message.size(), LWS_WRITE_TEXT);
    }
}

int Server::callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    if (reason == LWS_CALLBACK_HTTP) {
        std::string path = std::string((char*)in);
        std::string method = lws_hdr_total_length(wsi, WSI_TOKEN_GET_URI) ? "GET" : "POST";
        std::string body; // Note: Simplified, needs LWS_CALLBACK_HTTP_BODY for real body handling
        std::string response;
        
        server_instance->handle_api_request(path, method, body, response);
        
        unsigned char buf[LWS_PRE + 2048];
        unsigned char *p = buf + LWS_PRE;
        memcpy(p, response.c_str(), response.size());
        lws_write(wsi, p, response.size(), LWS_WRITE_HTTP);
        lws_finalize_write(wsi, LWS_WRITE_HTTP);
        return 0;
    }
    return lws_callback_http_dummy(wsi, reason, user, in, len);
}

int Server::callback_websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            server_instance->clients[std::string((char*)in)] = wsi;
            break;
        case LWS_CALLBACK_CLOSED:
            server_instance->clients.erase(std::string((char*)in));
            break;
        default:
            break;
    }
    return 0;
}

void Server::handle_api_request(const std::string& path, const std::string& method, const std::string& body, std::string& response) {
    Json::Value json_response;
    if (path == "/api/polls" && method == "POST") {
        Json::Value json_body;
        Json::Reader reader;
        if (reader.parse(body, json_body)) {
            std::string question = json_body["question"].asString();
            std::vector<std::string> options;
            for (const auto& opt : json_body["options"]) {
                options.push_back(opt.asString());
            }
            int poll_id = db.create_poll(question, options);
            json_response["status"] = "success";
            json_response["poll_id"] = poll_id;
        } else {
            json_response["status"] = "error";
            json_response["message"] = "Invalid JSON";
        }
    } else if (path == "/api/polls" && method == "GET") {
        std::vector<Poll> polls = db.get_polls();
        json_response["status"] = "success";
        for (const auto& poll : polls) {
            Json::Value p;
            p["id"] = poll.id;
            p["question"] = poll.question;
            json_response["polls"].append(p);
        }
    } else if (path == "/api/vote" && method == "POST") {
        Json::Value json_body;
        Json::Reader reader;
        if (reader.parse(body, json_body)) {
            int poll_id = json_body["poll_id"].asInt();
            int option_id = json_body["option_id"].asInt();
            std::string user_ip = json_body["user_ip"].asString();
            if (db.add_vote(poll_id, option_id, user_ip)) {
                json_response["status"] = "success";
                server_instance->broadcast_results(poll_id);
            } else {
                json_response["status"] = "error";
                json_response["message"] = "Already voted or invalid option";
            }
        } else {
            json_response["status"] = "error";
            json_response["message"] = "Invalid JSON";
        }
    } else if (path == "/api/results" && method == "GET") {
        int poll_id = std::stoi(path.substr(path.find_last_of("=") + 1)); // Simplified query parsing
        Poll poll = db.get_poll_results(poll_id);
        json_response["status"] = "success";
        json_response["poll_id"] = poll.id;
        json_response["question"] = poll.question;
        for (const auto& option : poll.options) {
            Json::Value opt;
            opt["id"] = option.id;
            opt["text"] = option.text;
            opt["votes"] = option.vote_count;
            json_response["options"].append(opt);
        }
    } else {
        json_response["status"] = "error";
        json_response["message"] = "Invalid endpoint";
    }
    response = Json::writeString(Json::StreamWriterBuilder(), json_response);
}
