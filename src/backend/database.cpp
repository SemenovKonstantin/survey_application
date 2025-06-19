#include "database.h"
#include <iostream>

Database::Database(const std::string& conn_str) : conn(conn_str) {
    if (!conn.is_open()) {
        std::cerr << "Failed to connect to database" << std::endl;
        exit(1);
    }
}

int Database::create_poll(const std::string& question, const std::vector<std::string>& options) {
    try {
        pqxx::work txn(conn);
        auto result = txn.exec_params(
            "INSERT INTO polls (question, created_at) VALUES ($1, NOW()) RETURNING id",
            question
        );
        int poll_id = result[0][0].as<int>();

        for (const auto& option : options) {
            txn.exec_params(
                "INSERT INTO poll_options (poll_id, option_text) VALUES ($1, $2)",
                poll_id, option
            );
        }

        txn.commit();
        return poll_id;
    } catch (const std::exception& e) {
        std::cerr << "Error creating poll: " << e.what() << std::endl;
        return -1;
    }
}

bool Database::add_vote(int poll_id, int option_id, const std::string& user_ip) {
    try {
        pqxx::work txn(conn);
        auto result = txn.exec_params(
            "SELECT 1 FROM votes WHERE poll_id = $1 AND user_ip = $2",
            poll_id, user_ip
        );
        if (!result.empty()) {
            return false; // Already voted
        }

        result = txn.exec_params(
            "SELECT 1 FROM poll_options WHERE id = $1 AND poll_id = $2",
            option_id, poll_id
        );
        if (result.empty()) {
            return false; // Invalid option
        }

        txn.exec_params(
            "INSERT INTO votes (poll_id, option_id, user_ip, created_at) VALUES ($1, $2, $3, NOW())",
            poll_id, option_id, user_ip
        );
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error adding vote: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Poll> Database::get_polls() {
    std::vector<Poll> polls;
    try {
        pqxx::nontransaction txn(conn);
        auto result = txn.exec("SELECT id, question FROM polls");
        for (const auto& row : result) {
            Poll poll;
            poll.id = row[0].as<int>();
            poll.question = row[1].as<std::string>();
            polls.push_back(poll);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting polls: " << e.what() << std::endl;
    }
    return polls;
}

Poll Database::get_poll_results(int poll_id) {
    Poll poll;
    try {
        pqxx::nontransaction txn(conn);
        auto result = txn.exec_params("SELECT id, question FROM polls WHERE id = $1", poll_id);
        if (!result.empty()) {
            poll.id = result[0][0].as<int>();
            poll.question = result[0][1].as<std::string>();
        }

        result = txn.exec_params(
            "SELECT po.id, po.option_text, COUNT(v.option_id) as vote_count "
            "FROM poll_options po LEFT JOIN votes v ON po.id = v.option_id "
            "WHERE po.poll_id = $1 GROUP BY po.id, po.option_text",
            poll_id
        );
        for (const auto& row : result) {
            PollOption option;
            option.id = row[0].as<int>();
            option.text = row[1].as<std::string>();
            option.vote_count = row[2].as<int>();
            poll.options.push_back(option);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting poll results: " << e.what() << std::endl;
    }
    return poll;
}
