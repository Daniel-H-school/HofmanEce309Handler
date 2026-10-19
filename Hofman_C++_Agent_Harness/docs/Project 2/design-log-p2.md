# Design Log — Project 2

(500–800 words total. See spec §5 for what each section must cover.)

## Growth factor and amortized cost
The time complexity of the append() function in conversation is an amortized O(1). In my append(), once the dynamic array has run out of capacity, it gets replaced by a newly allocated one that is twice as long. (i.e. growth factor = 2). First, new memory must be allocated with type Message. Because Message uses a constructor, this adds N * 2 time steps to initialize the space to empty messages, where N is the size of capacity_: O(N). Next, the existing data must be copied over to this space one-by-one. This requires N steps, where N is the size of capacity_: O(N). Third, the previous data needs to be de-allocated and deleted, which adds N time steps. Finally, the new message is placed into the next slot: O(1). Adding everything up together we get O(2N + N + N + 1), which is equal to O(N) steps are required every time the dynamic memory expands. 

However, because our growth factor is 2, each append() operation does not need to expand the dynamic array. Whenever the old capacity is reached, the new capacity doubles, so it takes another old capacity times to require another expansion. Let's say that a reallocation at size n has a complexity of O(N), as discussed earlier. For the next n appends, it only takes constant time steps to move a message into its spot and increment a few variables: O(1). So, the total copying across these n appends is only O(N). By amortizing per append, we see O(N/N), or an amortized time complexity of O(1) per append.

I chose the growth factor of two to better demonstrate this amortized growth. While I considered a smaller growth factor due to conversations at this stage of the project not being very long, I found that they grew too slowly at the early stages. This would then call for new dynamic arrays nearly every append() early on, which seemed inefficient.

## Rule of Five evidence
In the conversation class, I had to implement the Rule of Five because of its dynamic memory array data storage for message objects. The pointer to this data is stored in data_
In the destructor, I have a simple delete[] data_ which safely deallocates the memory whenever an object falls out of scope.
In the copy constructor, I first allocate space for the new data to go into, then I have a simple for loop interate through the other Conversation's data_ array and manually copy over each message into this Conversation's data_ array. Then I copy over the other Conversation's size_ and capacity_ values.
In the copy assignment operator, I first double check that there isn't a situation like conversationA = conversationA happening (as that may end up deleting its own data before it copies itself over). Then, I delete[] this conversation's data to prevent it from staying in heap memory without ownership. Next, I manually copy over each message from the other Conversation's data_ array.
In the Move Constructor, I copy over size_ and capacity_ data as usual. Then I delete this Conversation's data_ as a double check, although its data_ should be a nullptr after just being initialized. Next, I assign data_ pointer to point to the other's data_. Then I make other's data_ into a nullptr to make sure that it doesn't get deleted when the other Conversation is destructed.
The move assignment operator is written identically to the move Constructor, except it returns *this.

## Sentinel scanner: bounded pending_ proof
My SentinelScanner feed() function is split up into two parts to ensure that pending_ never exceeds sentinel_.size() - 1 as it scans through the chunks. Otherwise, concatenating all the chunks together and searching from the beginning would have O(N^2).
To begin my feed function, I fill up pending_ with however many chunks it takes to get to sentinel_.size() - 1. On whichever chunk may bring pending_.size() over that, the first output of safe text can be displayed. Then pending_ is set to be whatever was not just outputted, and the program returns.
For the rest of the feed function, this same process is kept, where the next pending_ is set to be the last sentinel_.size() - 1 characters of the previous pending_ + current chunk. This hard caps pending_'s size at sentinel_.size() - 1.
Additionally, a test verifies that pending_.size() is less than sentinel_.size() - 1.

## What I would design differently
If I had more time and a better base understanding of the chunking and sentinel system going into project 2, then I would have tried to implement the Knuth-Morris-Pratt algorithm or find some other way to optimize the sentinel scanner. Even with keeping my pending_.size() low, there are still too many concatenations for my liking, and there must be a more efficient way, even without implementing that specific algorithm.