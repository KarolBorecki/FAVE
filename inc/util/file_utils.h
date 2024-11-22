#ifndef FAVE_FILE_UTILS_H
#define FAVE_FILE_UTILS_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>

#include "logging/logger.h"
#include "logging/exceptions.h"

namespace FAVE {
    std::string get_file_contents(const char *filename);
}

#endif // FAVE_FILE_UTILS_H