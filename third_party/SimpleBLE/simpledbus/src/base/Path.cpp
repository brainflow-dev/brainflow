#include "simpledbus/base/Path.h"

#include <algorithm>
#include <regex>
#include <stdexcept>

namespace SimpleDBus {

Path::Path(const std::string& path) : _path(path) { PathUtils::validate(_path); }

// Member functions delegating to PathUtils
size_t Path::count_elements() const { return PathUtils::count_elements(_path); }
std::string Path::fetch_elements(size_t count) const { return PathUtils::fetch_elements(_path, count); }
std::vector<std::string> Path::split_elements() const { return PathUtils::split_elements(_path); }
bool Path::is_descendant(const Path& base) const { return PathUtils::is_descendant(base._path, _path); }
bool Path::is_ascendant(const Path& base) const { return PathUtils::is_ascendant(base._path, _path); }
bool Path::is_child(const Path& base) const { return PathUtils::is_child(base._path, _path); }
bool Path::is_parent(const Path& base) const { return PathUtils::is_parent(base._path, _path); }
std::string Path::next_child(const Path& base) const { return PathUtils::next_child(base._path, _path); }
std::string Path::next_child_strip(const Path& base) const { return PathUtils::next_child_strip(base._path, _path); }

size_t PathUtils::count_elements(const std::string& path) {
    if (path.empty() || path == "/") {
        return 0;
    }

    return std::count(path.begin(), path.end(), '/');
}

std::vector<std::string> PathUtils::split_elements(const std::string& path) {
    std::vector<std::string> elements;

    if (path.empty() || path == "/") {
        return elements;
    }

    // Note: Skip the first element, which is the root
    std::string::const_iterator it = path.begin() + 1;
    std::string::const_iterator end = path.end();

    while (it != end) {
        std::string::const_iterator next = std::find(it, end, '/');
        elements.push_back(std::string(it, next));
        it = next;
        if (it != end) {
            ++it;
        }
    }

    return elements;
}

std::string PathUtils::fetch_elements(const std::string& path, size_t count) {
    if (count == 0) {
        return "/";
    }

    if (count > count_elements(path)) {
        // TODO: Should we throw an exception?
        return path;
    }

    std::vector<std::string> elements = split_elements(path);
    std::string result;

    for (size_t i = 0; i < count; i++) {
        result += "/" + elements[i];
    }

    return result;
}

bool PathUtils::is_descendant(const std::string& base, const std::string& path) {
    if (base.empty() || path.empty()) {
        return false;
    }

    if (base == path) {
        return false;
    }

    if (base == "/") {
        return true;
    }

    return path.rfind(base) == 0;
}

bool PathUtils::is_ascendant(const std::string& base, const std::string& path) {
    if (base.empty() || path.empty()) {
        return false;
    }

    if (base == path) {
        return false;
    }

    return !is_descendant(base, path);
}

bool PathUtils::is_child(const std::string& base, const std::string& path) {
    if (base.empty() || path.empty()) {
        return false;
    }

    if (base == path) {
        return false;
    }

    if (!is_descendant(base, path)) {
        return false;
    }

    return count_elements(base) + 1 == count_elements(path);
}

bool PathUtils::is_parent(const std::string& base, const std::string& path) {
    if (base.empty() || path.empty()) {
        return false;
    }

    if (base == path) {
        return false;
    }

    if (!is_ascendant(base, path)) {
        return false;
    }

    return count_elements(base) - 1 == count_elements(path);
}

std::string PathUtils::next_child(const std::string& base, const std::string& path) {
    return fetch_elements(path, count_elements(base) + 1);
}

std::string PathUtils::next_child_strip(const std::string& base, const std::string& path) {
    const std::string child = next_child(base, path);
    return child.substr(base.length() + 1);
}

void PathUtils::validate(const std::string& path) {
    if (path.empty()) {
        throw std::invalid_argument("Path cannot be empty");
    }
    if (path != "/" && path[0] != '/') {
        throw std::invalid_argument("Path must start with '/'");
    }
    // Allow alphanumeric, underscores, and slashes only
    std::regex valid_path_regex("^/([a-zA-Z0-9_]+/)*[a-zA-Z0-9_]*$");
    if (!std::regex_match(path, valid_path_regex)) {
        throw std::invalid_argument("Path contains invalid characters");
    }
}

}  // namespace SimpleDBus
