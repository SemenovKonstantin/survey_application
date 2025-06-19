#ifndef DATABASE_H
#define DATABASE_H

#include <pqxx/pqxx>
#include <string>
#include <vector>
#include "types.h"

class Database {
public:
    Database(const std::string& conn_str);
    int create_poll(const std::string& question, const std::vector<std::string>& options);
    bool add_vote(int poll_id, int option_id, const std::string& user_ip);
    std::vector<Poll> get_polls();
    Poll get_poll_results(int poll_id);

private:
    pqxx::connection conn;
};

#endif
