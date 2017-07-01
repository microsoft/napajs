#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include <vector>


namespace napa {
namespace utils {
namespace string {

    /// <summary> Replace all matches in a string. </summary>
    inline void ReplaceAll(std::string& str, const std::string& match, const std::string& replacement) {
        for (size_t pos = str.find(match); pos != std::string::npos; pos = str.find(match, pos)) {
            str.replace(pos, match.size(), replacement);
            pos += replacement.size();
        }
    }

    /// <summary> Return a copy of string with all matches replaced. </summary>
    inline std::string ReplaceAllCopy(const std::string& str, const std::string& match, const std::string& replacement) {
        std::string copy(str);
        ReplaceAll(copy, match, replacement);
        return copy;
    }

    /// <summary> Split a string into a list with a set of delimiters. </summary>
    inline void Split(const std::string& str, std::vector<std::string>& list, const std::vector<char>& delimiters, bool compress = false) {
        list.clear();
        size_t start = 0;

        for (size_t i = 0; i < str.size(); ++i) {
            for (size_t j = 0; j < delimiters.size(); ++j) {
                if (str[i] == delimiters[j]) {
                    auto len = i - start;
                    if (len != 0 || !compress) {
                        list.push_back(str.substr(start, len));
                    }
                    start = i;
                    break;
                }
            }
        }
        auto len = str.size() - start;
        if (len != 0 || !compress) {
            list.push_back(str.substr(start, len));
        }
    }

    /// <summary> Split a string into a list with a delimiter. </summary>
    inline void Split(const std::string& str, std::vector<std::string>& list, const std::string& anyCharAsDelimiter, bool compress = false) {
        std::vector<char> delimiters;
        for (auto it = anyCharAsDelimiter.begin(); it != anyCharAsDelimiter.end(); ++it) {
            delimiters.push_back(*it);
        }
        Split(str, list, delimiters, compress);
    }

    /// <summary> In-place convert a string to lower case. </summary>
    inline void ToLower(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), std::tolower);
    }

    /// <summary> Return a copy of a string of lower case. </summary>
    inline std::string ToLowerCopy(const std::string& str) {
        std::string copy(str);
        ToLower(copy);
        return copy;
    }

    /// <summary> In-place convert a string to upper case. </summary>
    inline void ToUpper(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), std::toupper);
    }

    /// <summary> Return a copy of a string of upper case. </summary>
    inline std::string ToUpperCopy(const std::string& str) {
        std::string copy(str);
        ToUpper(copy);
        return copy;
    }

    /// <summary> Case insensitive compare. </summary>
    inline int CaseInsensitiveCompare(const std::string& left, const std::string& right) {
        for (size_t i = 0; i < left.size(); ++i) {
            if (i == right.size()) {
                return 1;
            }
            auto l = std::tolower(left[i]);
            auto r = std::tolower(right[i]);

            if (l != r) {
                return l - r;
            }
        }
        return -1;
    }

    /// <summary> Case insensitive equals. </summary>
    inline bool CaseInsensitiveEquals(const std::string& left, const std::string& right) {
        return CaseInsensitiveCompare(left, right) == 0;
    }
}
}
}