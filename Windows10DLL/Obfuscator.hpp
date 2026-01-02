#pragma once
#include <string>
#include <vector>

class Obfuscator {
public:
    static std::string Deobfuscate(const std::vector<unsigned char>& data, unsigned char key) {
        std::string output = "";
        for (unsigned char c : data) {
            output += (c ^ key);
        }
        return output;
    }
};