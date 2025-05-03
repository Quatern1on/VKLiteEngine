#include "pch.h"
#include "Utils.h"

std::vector<const char*> Utils::toCStrVector(const std::vector<std::string>& strings) {
    std::vector<const char*> cStrings;
    cStrings.reserve(strings.size());

    for (const auto& string : strings) {
        cStrings.push_back(string.c_str());
    }

    return cStrings;
}
