#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>

struct PollOption {
    int id;
    std::string text;
    int vote_count;
};

struct Poll {
    int id;
    std::string question;
    std::vector<PollOption> options;
};

#endif
