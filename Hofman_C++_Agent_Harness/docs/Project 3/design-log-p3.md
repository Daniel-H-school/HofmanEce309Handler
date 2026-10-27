# Project 3 Design Log
**Author:** <Daniel Hofman> · **P2 code used:** <instructor reference>
     While I had intended to use my own P2, there were some issues regarding the conversation class's data structure, especially when it came to the transcript playbacks. Upon receiving the feedback for project two, it's more clear what memory leaks may be causing this, but the feedback came in too late to fix project 2 for this one. However, I did notice that the sentinels were incorrectly being displayed in the terminal output when using the reference P2 code.
**Tools used:** <AI assistant (Gemini) was used to explain FNV-1a function better and to debug Hash table adding and re-sizing>

## 1. Hash table
     For my hash table, I used open addressing because I figured that it would be more memory efficient at the scale in which the tool registry would be used. Considering that the current project is only really using 4 or 5 tools, it seemed like it wouldn't take up too much time to linear probe around any collisions. My load-factor threshold before resizing the hash table was 0.75, which formed as a good baseline and performed well under the stress test. Each resize was to a hash table twice as large, starting at a hash table of size 8. This allows for the same ammortized cost argument from project two, where the costly O(N) resizings are juxtaposed by the O(1) adds without resize until the next one. Dividing this by N steps give an ammortized O(1) cost for adds. 

     For my linear probing, I decided to search both above and below the hash-function-given index. I belive I got this idea from class, but it certainly made for a more complicated code setup.

     The hash function for my table is the FNV-1a provided in the spec, and I found that it gave a good amount of "randomness" to avoid collisions somewhat naturally. Comparing against the weak hash Function at large values (N = 10,000) of similarly named tools, the bucket_count (16384) and load_factor (0.610352) remain the same due to open indexing. However, while the FNV-1a hashed function had a furthest probing distance of 16 (longest chain of 30), the Weak Hash had a furthest probing distance of 5001 (longest chain of 10,000). Including the entire comparison is unrealistic, but this is an indicative sample, where the hash functions displays the index of the added item:
                                        FNV-1a: 37        Weak Hash Function: 5029
                                        FNV-1a: 15116     Weak Hash Function: 11477
                                        FNV-1a: 15551     Weak Hash Function: 5030
                                        FNV-1a: 1342      Weak Hash Function: 11476
                                        FNV-1a: 1777      Weak Hash Function: 5031
                                        FNV-1a: 2009      Weak Hash Function: 11475
                                        FNV-1a: 1574      Weak Hash Function: 5032
                                        FNV-1a: 1139      Weak Hash Function: 11474
                                        FNV-1a: 704       Weak Hash Function: 5033
                                        FNV-1a: 3749      Weak Hash Function: 11473
                                        FNV-1a: 3314      Weak Hash Function: 5034
                                        FNV-1a: 2879      Weak Hash Function: 11472
                                        FNV-1a: 2444      Weak Hash Function: 5035
                                        FNV-1a: 5489      Weak Hash Function: 11471
                                        FNV-1a: 5054      Weak Hash Function: 5036
                                        FNV-1a: 2000      Weak Hash Function: 11470
                                        FNV-1a: 2435      Weak Hash Function: 5037
                                        FNV-1a: 2870      Weak Hash Function: 11469
                                        FNV-1a: 3305      Weak Hash Function: 5038
                                        FNV-1a: 3740      Weak Hash Function: 11468
                                        FNV-1a: 4175      Weak Hash Function: 5039
                                        FNV-1a: 4610      Weak Hash Function: 11467
                                        FNV-1a: 5045      Weak Hash Function: 5040
                                        FNV-1a: 5480      Weak Hash Function: 11466
                                        FNV-1a: 5915      Weak Hash Function: 5041
                                        FNV-1a: 15083     Weak Hash Function: 11465
                                        FNV-1a: 14648     Weak Hash Function: 5042
                                        FNV-1a: 15953     Weak Hash Function: 11464
                                        FNV-1a: 15518     Weak Hash Function: 5043
                                        FNV-1a: 439       Weak Hash Function: 11463
                                        FNV-1a: 4         Weak Hash Function: 5044
                                        FNV-1a: 1309      Weak Hash Function: 11462
                                        FNV-1a: 874       Weak Hash Function: 5045
                                        FNV-1a: 11603     Weak Hash Function: 11461
                                        FNV-1a: 11168     Weak Hash Function: 5046
## 2. Pointer stability
     A Tool* returned by find() is able to survive a resize in my design because the resize uses the std::move() function to move around std::unique_ptr Tool objects. Thus, the actual memory containg the object is never moved between resizes, only the method of accessing it through the tool registry hash function.

## 3. Calculator trace
     Here is a trace of what the calculator is doing internally through a two-stack values and operations system when evaluating the tokens 2 * (3 + 4) - 5
                         2 * (3 + 4) - 5
     Values    Ops       Tokens left         -At start, both values and ops stacks are empty

       2                 * (3 + 4) - 5
     Values    Ops       Tokens left         -Every time a number is encountered, it is added to the values stack

       2        *        (3 + 4) - 5
     Values    Ops       Tokens left         -The first operator is pushed onto the ops stack

                (
       2        *        3 + 4) - 5
     Values    Ops       Tokens left         -The left parenthesis is always pushed onto the ops stack

       3        (
       2        *        + 4) - 5
     Values    Ops       Tokens left         -Every number encountered is added to values stack

                +
       3        (
       2        *        4) - 5
     Values    Ops       Tokens left         -Since the top of the stack is a left parenthesis and not a higher-precedence operation, the addition gets pushed onto ops stack

       4        +
       3        (
       2        *        ) - 5
     Values    Ops       Tokens left         -Every number encountered is added to values stack

                
       4        +                            
       3        (
       2        *        - 5 
     Values    Ops       Tokens left        -The current operation is the ) paren. In this case, the operations on the Ops stack are performed in order until the left paren is found
                                            -First, the addition is encountered. 4 and 3 are popped from the values stack and stored as the right and left arguments, respectively.
                                            -The operation 3 + 4 is performed, and the result is pushed onto the values stack
                                            -The addition operation is popped from the operation stack
       7                                    -Because the next operation is left parenthesis, the left parenthesis are also popped from the operation stack
       2        *        5
     Values    Ops       Tokens left        -The current operation is subtraction. The top of the ops stack is multiplication, which has a higher precedence than subtraction.
                                            -As such, the two values are popped in the same manner as before to perform 2 * 7. The result of 14 is pushed onto the values stack
       5                                    -The multiplication is popped from the ops stack, and the current operation of subtraction is pushed onto it
       14       -
     Values    Ops       Tokens left        -There are no longer any tokens left in the equation. From here, the operations left in the ops stack are performed top-to-bottom.
                                            -In this example, subtraction becomes the active operation and the two values are popped.
                                            -The operation 14 - 5 is performed and the result is pushed to values stack
       9
     Values    Ops       Tokens left        -Since there are no more operations to perform, the remaining value is popped and returned as the answer to the equation.


## 4. Errors as values
     Tools, resolve(), and calc_evaluate() return errors instead of throwing exceptions because it keeps the program running smoothly, while an exception is bulkier and more final. An exception is a good choice for processes which would result in catastrophic errors or dangerous memory accessing/modifying. In these cases, it is a good idea to stop everything and give a warning of the behavior causing itself so that the programmer can fix the error. In our program, the agent handler is the thing that's supposed to deal with these problems at runtime, not the user. Thus it makes sense to return internal errors that the handler can evaluate instead of throwing broad exceptions with a bunch of try and catch blocks everywhere.

## 5. What I would design differently
     If I had started this project again from scratch, I would likely design the linear probing to just go in one direction. Doing it bidirectionally may be slightly more efficient, but the added complexity and likelihood to have a dead spot which is never indexed through improper bounds checking were annoying to deal with.