// src/tools/workspace.cpp — YOU IMPLEMENT (spec §4.4).
//
// Workspace::resolve: a purely lexical check, steps 1–6 of the spec.
// Do not touch the filesystem here (no exists(), canonical(), ...).
// The constructor, root() and check_symlinks() are provided in
// workspace_provided.cpp.

#include "tools/workspace.h"
#include <iostream>

namespace fs = std::filesystem;

bool Workspace::resolve(std::string_view user_path, fs::path* out,
                        std::string* reason) const {
    if (user_path.empty()){     //If fed an empty string
        *reason = "empty path";
        return false;
    }

    if (user_path.find('\0') != -1){  //If fed a string with incorrect formatting (nul char which would mess up future steps; should be optimized out by operating system under normal use)
        *reason = "unexpected null character in path";
        return false;
    }

    std::filesystem::path path{std::string(user_path)};;  //Initializes a filesystem path based on the argument path
   
    if (path.is_absolute() || path.has_root_path()){
        *reason = "only workspace-relative paths allowed";
        return false;
    } 

    std::filesystem::path simplified_path = path.lexically_normal();  //Creates a simplified path without unnecessary components like // or /../

    if (simplified_path.string().substr(0,2) == ".."){    //If this simplified path starts out by reaching beyond its workspace's scope
        *reason = "path reaches beyond scope with ../";
        return false;
    }

    if (simplified_path.compare(".") == 0){   //If the simplified path is just "." (i.e. the current workspace)
        *out = root();  //Returns this workspace
    } else {    //Else if n is a regular path
        *out = root() / simplified_path;    //Returns this workspace + the path that the user specified
    }

    *reason = "Worked as expected";  
    return true;
}
