#pragma once
#include "hackforge/schema.hpp"
#include <string>

namespace hackforge {
class Minimizer {
public:
    static void minimize(const std::string& target, const Schema& schema, const std::string& input_file, const std::string& output_file);
};
}