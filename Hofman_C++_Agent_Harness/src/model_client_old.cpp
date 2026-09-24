#include <string>           //Uses strings as a data type
#include "core/conversation.h"   //Uses conversation and message classes

// Struct for stops in code for errors
struct StopReason {
    enum class Kind { Sentinel, TurnLimit, UserExit, ClientError } kind;    //Which kind of stoppage was it
    std::string detail;         //more details/explanation behind the stop 
};

/*
Class that is used for tracking token use
*/
class TokenSink {
public:
    virtual ~TokenSink() = default;
    virtual void on_chunk(std::string_view chunk) = 0;
    virtual void on_complete() = 0;
};

/*
    Class that acts as an interface to model the ai's direct interactions with client.
*/
class ModelClient {
public:
    virtual ~ModelClient() = default;

    //Gets tokens from the sink, collects them, and appends them to the conversation
    virtual void generate(const Conversation& conv, TokenSink& sink){
        
    }
    Message generate(const Conversation& conv){
        // TokenSink userSink = new TokenSink();
        // this->generate(conv, userSink);
        // return Message(Role::Assistant, userSink.)
    };
};