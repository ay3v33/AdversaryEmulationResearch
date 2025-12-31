#include <vector>
#include <string>

class RollingSecureBuffer {
private:
    std::vector<char> buffer;
    // A longer, multi-byte key is much harder to guess
    const std::string secretKey = "D1sc0rd_St3alth_2026!";

public:
    void addChar(char c) {
        // Use the modulus operator (%) to "roll" through the key
        char keyByte = secretKey[buffer.size() % secretKey.length()];
        buffer.push_back(c ^ keyByte);
    }

    std::string getDecryptedData() {
        std::string decrypted = "";
        for (size_t i = 0; i < buffer.size(); ++i) {
            char keyByte = secretKey[i % secretKey.length()];
            decrypted += (buffer[i] ^ keyByte);
        }
        return decrypted;
    }

    int size() {
        return buffer.size();
    }

    void clear() {
        if (!buffer.empty()) {
            std::fill(buffer.begin(), buffer.end(), 0);
            buffer.clear();
        }
    }
};