#include <string>   //Uses strings as data types
#include "core/message.h"

// enum class Role {
//     System,
//     User,
//     Assistant
// };

//class Message {
//private:
    Role        role_;
    std::string content_;

//public:
    Message::Message(){
        role_ = Role::System;   //Defaults to an empty system message if initialized without parameters
        content_ = "";
    }

    //Proper constructor
    Message::Message(Role role, std::string content){
        role_ = role;   //Assigns local variables
        content_ = content;
    }

    Role Message::role() const noexcept{ //Returns this message's role
        return role_;   
    }

    const std::string& Message::content() const noexcept{ //Returns this message's conent
        return content_;
    }
//};