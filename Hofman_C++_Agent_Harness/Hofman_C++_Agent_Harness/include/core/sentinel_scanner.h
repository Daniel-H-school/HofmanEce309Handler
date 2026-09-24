#include <string>   //Uses strings to store data
/*
    Consumes chunks, emits text not part of the sentinal.
    Reports when the sentinel is found
*/

class SentinelScanner {
public:
    explicit SentinelScanner(std::string sentinel); //Constructor

    struct Out { std::string safe_text; bool sentinel_found; }; //Struct to hold outputs. Outputs comprised of the safe text output and whether or not the sentinel was found
    Out feed(std::string_view chunk);   //Feed chunk to be scanned
    Out flush();                        //Flush out feed

private:
    std::string sentinel_;  //The sentinel being searched for
    std::string pending_;   //Whether or not it has yet been found?
};