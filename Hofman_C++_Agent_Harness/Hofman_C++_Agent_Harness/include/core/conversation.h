/*
    Conversation class.
    dynamically allocates and stores recent messages to
    form a simple conversational memory.
*/
#include "message.h"    //Uses the message class as a data type
#include <string>       //Uses strings for message data

class Conversation {
public:
    //Rule of five functions, as this class dynamically allocates and thus
    //deletes memory.
    ~Conversation();                                        //Destructor
    Conversation(const Conversation& other);                //Copy Constructor
    Conversation& operator=(const Conversation& other);     //Assignment Operator
    Conversation(Conversation&& other) noexcept;            //Movement Operator
    Conversation& operator=(Conversation&& other) noexcept; //Move assign operatior

    void append(Message m);     //Adds this message to the end of message history

    std::size_t     size() const noexcept;  //Returns the size of message history?
    const Message& at(std::size_t i) const; //Returns value at indexed spot in conversation?
    const Message* begin() const noexcept;  //Returns index of first message?
    const Message* end()   const noexcept;  //Returns index of latest message?

private:
    Message*    data_ = nullptr;    //?
    std::size_t size_ = 0;          //Size of conversation
    std::size_t capacity_ = 0;      //Number of messages left to be added to conversation
};