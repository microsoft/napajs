// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

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

    /// <summary> Split a string range into a list with a set of delimiters. </summary>
    inline void Split(
        std::string::const_iterator begin,
        std::string::const_iterator end,
        std::vector<std::string>& outputList,
        const std::vector<char>& delimiters,
        bool compress = false) {

        outputList.clear();
        auto start = begin;

        for (auto it = begin; it != end; ++it) {
            for (auto j = delimiters.begin(); j < delimiters.end(); ++j) {
                if (*it == *j) {
                    if (it != start || !compress) {
                        outputList.push_back(std::string(start, it));
                    }
                    start = it + 1;
                    break;
                }
            }
        }
        if (start != end || !compress) {
            outputList.push_back(std::string(start, end));
        }
    }

    /// <summary> Split a string range into a list with a set of delimiters passed in as a string. </summary>
    inline void Split(
        std::string::const_iterator begin,
        std::string::const_iterator end,
        std::vector<std::string>& outputList,
        const std::string& anyCharAsDelimiter,
        bool compress = false) {

        std::vector<char> delimiters(anyCharAsDelimiter.begin(), anyCharAsDelimiter.end());
        Split(begin, end, outputList, delimiters, compress);
    }

    /// <summary> Split a string into a list with a set of delimiters. </summary>
    inline void Split(
        const std::string& str,
        std::vector<std::string>& outputList,
        const std::string& delimiters,
        bool compress = false) {

        return Split(str.begin(), str.end(), outputList, delimiters, compress);
    }

    /// <summary> Trim the provided string. </summary>
    inline void Trim(std::string &str) {
        // Left trim
        str.erase(0, str.find_first_not_of(" \n\r\t"));

        // Right trim
        str.erase(str.find_last_not_of(" \n\r\t") + 1);
    }

    /// <summary> Return a trimmed copy of the string. </summary>
    inline std::string TrimCopy(const std::string &str) {
        std::string copy = str;
        Trim(copy);
        return copy;
    }

    /// <summary> In-place convert a string to lower case. </summary>
    inline void ToLower(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), (int (*)(int))std::tolower);
    }

    /// <summary> Return a copy of a string of lower case. </summary>
    inline std::string ToLowerCopy(const std::string& str) {
        std::string copy(str);
        ToLower(copy);
        return copy;
    }

    /// <summary> In-place convert a string to upper case. </summary>
    inline void ToUpper(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), (int (*)(int))std::toupper);
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
        return 0;
    }

    /// <summary> Case insensitive equals. </summary>
    inline bool CaseInsensitiveEquals(const std::string& left, const std::string& right) {
        return CaseInsensitiveCompare(left, right) == 0;
    }
}
}
}