/**
 * @file harness.h
 * @brief Simple Mock LLM Agent Harness Header File
 * 
 * ============================================================================
 *                         SYSTEM ARCHITECTURE OVERVIEW
 * ============================================================================
 * 
 *    +-------------------------------------------------------------------+
 *    |                            main()                                 |
 *    |  - Runs the Interactive Loop (fgets)                              |
 *    |  - Handles clean exit ('exit')                                    |
 *    +---------------------------------+---------------------------------+
 *                                      |
 *                                      v
 *    +-------------------------------------------------------------------+
 *    |                       mock_llm_handler()                          |
 *    |  - Intent Classification / Pattern Matcher                        |
 *    |  - Formats responses                                              |
 *    +-----------------+-------------------------------+-----------------+
 *                      |                               |
 *                      v                               v
 *    +-----------------------------------+   +---------------------------+
 *    |       tool_add_numbers()          |   |    add_to_history()       |
 *    |  - Parses math parameters         |   |  - Dynamic heap allocation|
 *    |  - Performs calculation           |   |  - Circular buffer logic  |
 *    +-----------------------------------+   +---------------------------+
 * 
 * ============================================================================
 */

#ifndef HARNESS_H
#define HARNESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Maximum allowed size for input strings
#define MAX_INPUT 256

// Capacity of the conversation ring buffer
#define MAX_HISTORY 5

/**
 * @brief Represents a single prompt-response turn in conversation memory.
 */
typedef struct {
    char *prompt;   /**< Dynamically allocated string for user input */
    char *response; /**< Dynamically allocated string for agent reply */
} Turn;

/**
 * @brief Circular buffer managing conversation history state.
 */
typedef struct {
    Turn turns[MAX_HISTORY]; /**< Fixed array storing up to MAX_HISTORY turns */
    int count;               /**< Total active turns stored (0 to MAX_HISTORY) */
    int head;                /**< Write index for the next incoming turn */
} ConversationMemory;

/* -------------------------------------------------------------------
 * TOOL IMPLEMENTATION PROTOTYPES
 * ------------------------------------------------------------------- */

/**
 * @brief Tool function that parses integers from text and prints addition results.
 * 
 * Parses prompts matching "add X Y" or "calculate X + Y".
 * 
 * @param input The user input string containing math operands.
 */
void tool_add_numbers(const char *input);

/* -------------------------------------------------------------------
 * MEMORY MANAGEMENT PROTOTYPES
 * ------------------------------------------------------------------- */

/**
 * @brief Saves a prompt/response pair into heap memory using a ring buffer.
 * 
 * Frees previous memory at the head index if overwritten, allocates new memory
 * for both strings, and advances the head pointer circularly using modulo arithmetic.
 * 
 * @param mem Pointer to the active ConversationMemory structure.
 * @param prompt User input string to copy into heap memory.
 * @param response AI response string to copy into heap memory.
 */
void add_to_history(ConversationMemory *mem, const char *prompt, const char *response);

/**
 * @brief Frees all dynamically allocated memory stored in the conversation buffer.
 * 
 * Iterates through all turns, calls free() on non-NULL prompt and response pointers,
 * and sets pointers back to NULL to prevent dangling references.
 * 
 * @param mem Pointer to the active ConversationMemory structure.
 */
void free_history(ConversationMemory *mem);

/**
 * @brief Displays all stored prompt-response pairs in chronological order.
 * 
 * @param mem Const pointer to the active ConversationMemory structure.
 */
void print_history(const ConversationMemory *mem);

/* -------------------------------------------------------------------
 * MOCK LLM & AGENT HANDLER PROTOTYPES
 * ------------------------------------------------------------------- */

/**
 * @brief Mimics an LLM router by classifying intents and selecting actions.
 * 
 * Evaluates lowercase input to either trigger function calling (tool execution),
 * hardcoded keyword replies (greetings), history printing, or default echoing.
 * 
 * @param input User prompt string.
 * @param mem Pointer to conversation memory for history-dependent commands.
 * @return char* Dynamically allocated string containing the response. Must be freed by caller.
 */
char* mock_llm_handler(const char *input, ConversationMemory *mem);

#endif /* HARNESS_H */