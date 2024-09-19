#include "simpledbus/base/Path.h"

#include <algorithm>

namespace SimpleDBus {

// TODO: Add path validation and throw exception if invalid

size_t Path::count_elements(const std::string& path) {
    if (path.empty() || path == "/") {
        return 0;
    }

    return std::count(path.begin(), path.end(), '/');
}

std::vector<std::string> Path::split_elements(const std::string& path) {
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

std::string Path::fetch_elements(const std::string& path, size_t count) {
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

bool Path::is_descendant(const std::string& base, const std::string& path) {
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

bool Path::is_ascendant(const std::string& base, const std::string& path) {
    if (base.empty() || path.empty()) {
        return false;
    }

    if (base == path) {
        return false;
    }

    return !is_descendant(base, path);
}

bool Path::is_child(const std::string& base, const std::string& path) {
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

bool Path::is_parent(const std::string& base, const std::string& path) {
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

std::string Path::next_child(const std::string& base, const std::string& path) {
    return fetch_elements(path, count_elements(base) + 1);
}

std::string Path::next_child_strip(const std::string& base, const std::string& path) {
    const std::string child = next_child(base, path);
    return child.substr(base.length() + 1);
}

}  // namespace SimpleDBus
