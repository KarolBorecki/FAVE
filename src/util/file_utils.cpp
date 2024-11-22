#include "util/file_utils.h"

namespace FAVE {
    std::string get_file_contents(const char *filename)
    {
        std::ifstream in(filename, std::ios::in | std::ios::binary);
        FAVE::log("2");
        if (in)
        {
            FAVE::log("3");
            std::string contents;
            in.seekg(0, std::ios::end);
            FAVE::log("4");
            contents.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            FAVE::log("5");
            in.read(&contents[0], contents.size());
            in.close();
            return (contents);
        }
        FAVE::throwError("Could not open file: %s", filename);
    }
}