/*
    Enumerates a simple system to keep track of the "Roles" in the
    conversation between user, agent, and harness.
*/
#pragma once
#include <string>   //Messages use strings as data types

enum class Role { System, User, Assistant};
/*
    Message object. Each object carries the role of the sender
    as well as its message. Will be used in arrays and memory allocation
    to build larger conversations
*/
class Message {
private:
    Role        role_;      //Holds this message's role
    std::string content_;   //Holds this message's content
public:
    Message();  //Empty constructor for use in initializing empty array slots
    Message(Role role, std::string content);    //Full constructor to be constructed with role of messager as well as a message
                                                //The role is a Role object and the message is a string object
    Role               role()    const noexcept; // Accesses role
    const std::string& content() const noexcept; // Accesses content


};