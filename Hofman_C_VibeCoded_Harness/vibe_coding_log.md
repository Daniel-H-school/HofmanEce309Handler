// This file contains the prompts, responses, and iterations that I used while vibe coding for the
//first part of this project.
// I used Google Gemini, specifically the version provided to NCSU students as my agent for all subsequent prompts.

//Prompts are surrounded with ??
//Responses are surrounded with  !!

9/4/2026
??
I am a beginning C programmer, and I am trying to understand how AI Agent handlers function. To do this, I need you to write a simple command-line C program which imitates a harness for a mock LLM that I can learn from. Do not use external libraries, only standard C libraries like <stdio.h> and <string.h>. Here are the specifications for the program:

1. It should run a master while loop that asks for user input using fgets.
2. It should take that input and pass it into a function that mimics an LLM in doing the following things:
   a. If the user types 'exit', the loop should break and the program should end.
   b. If the user types a sentence containing the word 'hello', the program should print a hardcoded greeting.
   c. If the user types anything else, it should echo their input back to them.
3. Do not actually call an LLM API.
4. It should be able to allocate and free memory to store a minimal conversation history. For example, being able to "remember" the last 5 prompts and responses.
5. The harness should have the capability to call a tool to execute functions like mathematical calculations when an input calls for it.
6. Please add clear, line-by-line comments explaining what the code is doing.
??

!!
This C program acts as a lightweight AI agent harness. It demonstrates the core patterns used in real AI agents: a main execution loop, memory management (circular buffer for history), intent classification, and function calling (tool use).

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 256
#define MAX_HISTORY 5

// Structure to store a single conversation turn (User Prompt + Model Response)
typedef struct {
    char *prompt;
    char *response;
} Turn;

// Structure to hold our conversation memory (a fixed-size ring/circular buffer)
typedef struct {
    Turn turns[MAX_HISTORY]; // Array to store up to 5 turns
    int count;               // Total number of turns saved so far
    int head;                // Index where the next new turn will be written
} ConversationMemory;

// -------------------------------------------------------------------
// TOOL IMPLEMENTATION
// -------------------------------------------------------------------

// A simple tool function that handles basic addition when requested by the model handler
void tool_add_numbers(const char *input) {
    int a, b;
    // Attempt to parse two integers from the user's input string
    if (sscanf(input, "add %d %d", &a, &b) == 2 || sscanf(input, "calculate %d + %d", &a, &b) == 2) {
        printf("[TOOL EXECUTION] Calculator output: %d + %d = %d\n", a, b, a + b);
    } else {
        printf("[TOOL EXECUTION] Error: Could not parse numbers. Use format: 'add X Y'\n");
    }
}

// -------------------------------------------------------------------
// MEMORY MANAGEMENT FUNCTIONS
// -------------------------------------------------------------------

// Function to store a prompt/response pair into heap memory
void add_to_history(ConversationMemory *mem, const char *prompt, const char *response) {
    int index = mem->head;

    // If memory spot is currently occupied by an old turn, free that dynamic memory first
    if (mem->turns[index].prompt != NULL) {
        free(mem->turns[index].prompt);
        free(mem->turns[index].response);
    }

    // Allocate dynamic heap memory for the new prompt string (+1 for null terminator)
    mem->turns[index].prompt = (char *)malloc(strlen(prompt) + 1);
    strcpy(mem->turns[index].prompt, prompt);

    // Allocate dynamic heap memory for the new response string
    mem->turns[index].response = (char *)malloc(strlen(response) + 1);
    strcpy(mem->turns[index].response, response);

    // Advance head index using modulo arithmetic to wrap around (0 -> 1 -> 2 -> 3 -> 4 -> 0)
    mem->head = (mem->head + 1) % MAX_HISTORY;

    // Track total items stored up to our MAX_HISTORY limit
    if (mem->count < MAX_HISTORY) {
        mem->count++;
    }
}

// Helper function to free all dynamically allocated memory upon exit
void free_history(ConversationMemory *mem) {
    for (int i = 0; i < MAX_HISTORY; i++) {
        if (mem->turns[i].prompt != NULL) {
            free(mem->turns[i].prompt); // Free prompt buffer
            mem->turns[i].prompt = NULL;
        }
        if (mem->turns[i].response != NULL) {
            free(mem->turns[i].response); // Free response buffer
            mem->turns[i].response = NULL;
        }
    }
}

// Optional helper to display currently stored turns
void print_history(const ConversationMemory *mem) {
    printf("\n--- CONVERSATION HISTORY (Last %d) ---\n", mem->count);
    for (int i = 0; i < mem->count; i++) {
        printf("[%d] User: %s | AI: %s\n", i + 1, mem->turns[i].prompt, mem->turns[i].response);
    }
    printf("--------------------------------------\n\n");
}

// -------------------------------------------------------------------
// MOCK LLM & AGENT HANDLER
// -------------------------------------------------------------------

// Mock LLM function that processes user input, handles tool triggers, and returns a response
char* mock_llm_handler(const char *input, ConversationMemory *mem) {
    // Allocate space on heap for the AI's response buffer
    char *response = (char *)malloc(512);
    if (!response) return NULL;

    // Convert input copy to lowercase for easier string matching
    char lower_input[MAX_INPUT];
    strncpy(lower_input, input, MAX_INPUT);
    for (int i = 0; lower_input[i]; i++) {
        lower_input[i] = (char)tolower((unsigned char)lower_input[i]);
    }

    // AGENT DECISION TREE (Intent Routing)

    // Check 1: Tool invocation trigger ("add" or "calculate")
    if (strstr(lower_input, "add") != NULL || strstr(lower_input, "calculate") != NULL) {
        // Execute external function / tool
        tool_add_numbers(lower_input);
        snprintf(response, 512, "I recognized a math request and executed the addition tool for you.");
    }
    // Check 2: Greeting keyword check ("hello")
    else if (strstr(lower_input, "hello") != NULL) {
        snprintf(response, 512, "Hello! How can I assist you with your C code today?");
    }
    // Check 3: Display history debug command
    else if (strcmp(lower_input, "history") == 0) {
        print_history(mem);
        snprintf(response, 512, "Displayed conversation history above.");
    }
    // Default Behavior: Standard echo
    else {
        snprintf(response, 512, "Echo: %s", input);
    }

    return response;
}

// -------------------------------------------------------------------
// MAIN ENGINE LOOP
// -------------------------------------------------------------------

int main(void) {
    char input_buffer[MAX_INPUT];
    ConversationMemory memory = {0}; // Initialize all memory pointers to NULL and counts to 0

    printf("===================================================\n");
    printf(" Mock LLM Agent Harness Initialized\n");
    printf(" Commands: 'hello', 'add X Y', 'history', or 'exit'\n");
    printf("===================================================\n\n");

    while (1) {
        printf("User > ");
        
        // Read input safely using fgets
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
            break; // Handle EOF or input error
        }

        // Strip trailing newline character added by fgets
        input_buffer[strcspn(input_buffer, "\r\n")] = '\0';

        // Check 1: Exit condition check
        if (strcmp(input_buffer, "exit") == 0) {
            printf("Exiting agent loop...\n");
            break;
        }

        // Skip empty inputs
        if (strlen(input_buffer) == 0) {
            continue;
        }

        // Pass input into the Agent/LLM handler
        char *ai_response = mock_llm_handler(input_buffer, &memory);

        // Print AI's reply
        printf("Agent > %s\n\n", ai_response);

        // Store the prompt and generated response into our history memory
        add_to_history(&memory, input_buffer, ai_response);

        // Free local response string buffer returned by mock_llm_handler
        free(ai_response);
    }

    // Clean up allocated heap memory prior to exiting program
    free_history(&memory);
    printf("Memory freed successfully. Goodbye!\n");

    return 0;
}

```

### Key C Concepts Used

* **Heap Memory (`malloc`/`free`):** In C, strings that need to outlive local function scopes must be dynamically allocated using `malloc()` and cleaned up with `free()` to prevent memory leaks.
* **Circular Buffer (`% MAX_HISTORY`):** The `add_to_history` function overwrites older items once it exceeds 5 records without needing to continuously reallocate array sizes.
* **String Parsing (`strstr`, `sscanf`):** Functions from `<string.h>` act as primitive "intent classification" detectors to route user prompts into either standard replies or function tools.

!!

//After some troubleshooting with getting the WSL ubuntu setup and running the right file, I ran this code and it met the eye test

??
The code you provided appears to work! Could you create a simple bash script to input into the program a bunch of predefined text to verify its outputs work deterministically?
??

!!
Here is a simple Bash script using a **Here Document** (`<< 'EOF'`) to feed a sequence of predefined commands directly into your `./harness` binary and verify its output deterministically.

### `test_harness.sh`

```bash
#!/bin/bash

# Compile the C program first to ensure we have the latest binary
echo "Compiling harness.c..."
gcc harness.c -o harness

# Check if compilation succeeded
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

echo -e "\n--- STARTING AUTOMATED TEST SUITE ---\n"

# Pass a sequence of inputs directly into the program using stdin redirection
./harness << 'EOF'
hello
What is the capital of North Carolina?
add 15 27
calculate 100 + 50
history
exit
EOF

echo -e "\n--- TEST SUITE COMPLETE ---"

```

---

### How to Run It

1. **Create the script file:**
```bash
nano test_harness.sh

```


2. **Paste the code above and save.**
3. **Make the script executable:**
```bash
chmod +x test_harness.sh

```


4. **Execute the script:**
```bash
./test_harness.sh

```



---

### Expected Test Coverage

| Input Command | Expected Harness Behavior |
| --- | --- |
| `hello` | Matches keyword, outputs greeting prompt response. |
| `What is the capital...` | Default route, echoes input back. |
| `add 15 27` | Invokes `tool_add_numbers()` and prints `15 + 27 = 42`. |
| `calculate 100 + 50` | Invokes `tool_add_numbers()` and prints `100 + 50 = 150`. |
| `history` | Displays all 4 stored turns from dynamic heap memory. |
| `exit` | Breaks main loop, frees memory, and terminates cleanly. |
!!

//After executing the bash script, it produces the expected outputs for the predetermined inputs
//Finally, I asked Gemini to generate a header file for harness.c

??
Could you create a simple header file for harness.c? It needs to give text descriptions for all functions as well as a basic breakdown of the system architecture
??

!!
Here is a clean header file (`harness.h`) that provides full function prototypes, documentation comments, and a high-level visual summary of the harness architecture.

### `harness.h`

```c
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

```

---

### How to update `harness.c` to use it

1. Save the file above as `harness.h` in the same directory as `harness.c`.
2. At the very top of `harness.c`, replace your `#include` directives with:
```c
#include "harness.h"

```


3. Compile normally:
```bash
gcc harness.c -o harness

```
!!
