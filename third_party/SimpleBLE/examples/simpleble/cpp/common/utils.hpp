#ifndef UTILS_HPP
#define UTILS_HPP

#include <optional>
#include "simpleble/SimpleBLE.h"

namespace Utils {
/**
 * @brief Function to retrieve the adapter easily
 *
 * @return the adapter or an empty optional
 */
std::optional<SimpleBLE::Adapter> getAdapter();

/**
 * @brief Function to get a user input as size_t, it's used in all examples to select an index
 *
 * @param line the text asking for something to the user
 * @param max the maximum value that can be written
 *
 * @return the value or empty optional
 */
std::optional<std::size_t> getUserInputInt(const std::string& line, std::size_t max);

/**
 * @brief Pretty print a ByteArray
 */
void print_byte_array(const SimpleBLE::ByteArray& bytes);
}  // namespace Utils

#endif
