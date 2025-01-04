#pragma once

#include <string>
#include <vector>

namespace SimpleDBus {

class Path {
  public:
    static size_t count_elements(const std::string& path);
    static std::string fetch_elements(const std::string& path, size_t count);
    static std::vector<std::string> split_elements(const std::string& path);

    static bool is_descendant(const std::string& base, const std::string& path);
    static bool is_ascendant(const std::string& base, const std::string& path);

    static bool is_child(const std::string& base, const std::string& path);
    static bool is_parent(const std::string& base, const std::string& path);

    static std::string next_child(const std::string& base, const std::string& path);
    static std::string next_child_strip(const std::string& base, const std::string& path);
};

}  // namespace SimpleDBus
