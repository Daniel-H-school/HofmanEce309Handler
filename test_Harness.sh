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