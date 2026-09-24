/*
    Conversation class.
    dynamically allocates and stores recent messages to
    form a simple conversational memory.
*/
#pragma once
#include "message.h"    //Uses the message class as a data type
#include <string>       //Uses strings for message data

class Conversation {
private:
    Message*    data_ = nullptr;    //pointer to beginning of conversation array
    std::size_t size_ = 0;          //Size of conversation to this point
    std::size_t capacity_ = 0;      //Size of capacity for array holding conversation

public:
    // Empty conversation: size() == 0, no allocation yet.
    Conversation();

    // Releases all owned Message storage. No effect if already empty
    // (e.g. moved-from).
    ~Conversation();

    // Deep copy: allocates its own buffer and copies every Message.
    // this->begin() must differ from other.begin() afterward.
    Conversation(const Conversation& other);
    Conversation& operator=(const Conversation& other);

    // Steals other's buffer — no per-element copying. Afterward, other
    // must be left valid and empty (safe to destroy or reassign).
    Conversation(Conversation&& other) noexcept;
    Conversation& operator=(Conversation&& other) noexcept;

    // Appends m, growing the backing array if needed. Amortized O(1) —
    // document and justify your growth strategy in the design log
    // (see Appendix C if you want a refresher first).
    void append(Message m);

    // Number of messages currently stored.
    std::size_t size() const noexcept;

    // Bounds-checked access. Decide what happens on i >= size() (throw,
    // assert, whatever you pick) and test that behavior explicitly.
    const Message& at(std::size_t i) const;

    // Range-for iteration, oldest message first. begin() == end() when
    // size() == 0.
    const Message* begin() const noexcept;
    const Message* end()   const noexcept;


};