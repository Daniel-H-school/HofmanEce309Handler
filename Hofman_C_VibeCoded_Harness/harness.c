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