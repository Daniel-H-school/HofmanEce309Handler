#include <string>

class SentinelScanner {
private:
    std::string sentinel_;  //The sentinel being scanned for
    std::string pending_;   //The chunk of text currently being processed
public:

    explicit SentinelScanner(std::string sentinel){
        sentinel_ = sentinel;
    }

    struct Out {std::string safe_text; bool sentinel_found;};

    //Keeps track of two chunks at a time in case a chunk border cuts off
    //  the sentinel
    Out feed(std::string_view chunk){
        int i = 0;
        while (i < chunk.length()){
            pending_.append(chunk.substr());
            i++;
        }
    }
};