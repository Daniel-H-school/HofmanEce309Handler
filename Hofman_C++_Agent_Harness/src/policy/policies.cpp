// src/policy/policies.cpp — YOU IMPLEMENT. Spec §5.2.

#include "policy/policies.h"

#include <utility>

AllowlistPolicy::AllowlistPolicy(const std::vector<std::string>& programs) {
    // TODO: copy the names into programs_ (an std::unordered_set).
    // Clue: sets use insert(x), not push_back(x). One-liner: in the member
    // initializer list,  programs_(programs.begin(), programs.end()).
    for (auto program : programs){      //For eaach program
        programs_.insert(program);      //Insert a copy of it into the programs
    }
}

Decision AllowlistPolicy::evaluate(const CommandRequest& req) {
    // TODO
    // Clue: test membership with  programs_.count(x) > 0  or
    // programs_.find(x) != programs_.end().  (contains() is C++20; this
    // project is C++17.)  Build results with Decision::allow(reason, name())
    // and Decision::deny(reason, name()). Any non-empty reason is fine;
    // naming the program ("'rm' is not on the allowlist") helps the user.
    if (programs_.find(req.program) != programs_.end()){    //Checks if the requested program is within the allowlist
        return Decision::allow("'rm' is on allowlist",name());  //If allowed, return so
    } else {
        return Decision::deny("'rm' is not on allowlist",name());   //If disallowed, return so
    }
}

ArgumentPatternPolicy::ArgumentPatternPolicy(std::vector<std::string> patterns)
    : patterns_(std::move(patterns)) {}

Decision ArgumentPatternPolicy::evaluate(const CommandRequest& req) {
    // TODO: search req.command_line() for each pattern.
    // Clue: std::string::find returns std::string::npos when it finds
    // nothing, NOT 0 or false. "if (line.find(p))" is true for almost
    // everything. Compare with npos.
    // Clue: skip empty patterns: every string "contains" the empty string.
    
    for (std::string pattern : patterns_){  //For each pattern initialized into the policy
        if (pattern.empty()) continue;  //Skip empty "patterns"
        if (req.command_line().find(pattern) != std::string::npos){ //Checks if each pattern is found within the string
            return Decision::deny("command line contains blacklisted pattern", name());
        } 
    }

    return Decision::allow("Command line free of blacklisted patterns", name());
}

ConfirmPolicy::ConfirmPolicy(const std::vector<std::string>& programs) {
    for (auto program : programs){  //For each program
        programs_.insert(program);  //Inserts a copy of it into stored programs
    }
}

Decision ConfirmPolicy::evaluate(const CommandRequest& req) {
    // TODO: Decision::ask(...) for listed programs, Decision::allow(...) otherwise.
    if (programs_.find(req.program) != programs_.end()){    //If the program is listed
        return Decision::ask("'rm' needs confirmation to run", name());
    }
    return Decision::allow("'rm' allowed to run", name());
}

RateLimitPolicy::RateLimitPolicy(std::size_t max_commands, double window_seconds, const Clock& clock)
    : max_(max_commands), window_(window_seconds), clock_(clock) {}

Decision RateLimitPolicy::evaluate(const CommandRequest& req) {
    // TODO: drop timestamps that left the window, then decide.
    // Clues, in order:
    //  1. now = clock_.now_seconds().
    //  2. While the queue is NOT empty and its front timestamp has left the
    //     window, pop it. A timestamp s is still INSIDE when now - s < window_,
    //     so it has left when now - s >= window_ (exactly window_ seconds old
    //     counts as gone). Calling front() on an empty queue is undefined
    //     behavior (UBSan says "misaligned address"): check empty() first.
    //  3. If the queue already holds max_ timestamps: Deny, and do NOT push.
    //  4. Otherwise push(now) and Allow. Only allowed requests are recorded.
    if (max_ == 0) return Decision::deny(req.command_line() + " denied due max_ denying all requests", name());
    double now = clock_.now_seconds();    //Gets current time
    while (!times_.empty()){    //While there are still times to check in the queue
        if (now - times_.front() >= window_){   //If the front time is not within window
            times_.pop();   //Pop it
        } else {    //Else if it is in window
            break;  //break out. Further requests in queue will also be in windo
        }
    }
    if (times_.size() >= max_){ //If the held times within this timeframe has reached max already
        return Decision::deny(req.command_line() + " denied due to rate limitation", name());
    }
    times_.push(now);   //Else, push current time request
    return Decision::allow(req.command_line() + " not rate limited. Allowed", name());
}

std::size_t RateLimitPolicy::tracked() const noexcept { return times_.size(); }
