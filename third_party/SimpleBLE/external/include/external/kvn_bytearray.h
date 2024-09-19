#ifndef KVN_BYTEARRAY_H
#define KVN_BYTEARRAY_H

#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace kvn {

/**
 * @class bytearray
 * @brief A class to handle byte arrays and their conversion from/to hex strings.
 */
class bytearray {
  public:
    /**
     * @brief Default constructor.
     */
    bytearray() = default;

    /**
     * @brief Constructs byte array from a vector of uint8_t.
     * @param vec A vector of uint8_t.
     */
    bytearray(const std::vector<uint8_t>& vec) : data_(vec) {}

    /**
     * @brief Constructs byte array from a raw pointer and size.
     * @param ptr A pointer to uint8_t data.
     * @param size The size of the data.
     */
    bytearray(const uint8_t* ptr, size_t size) : data_(ptr, ptr + size) {}

    /**
     * @brief Constructs byte array from a std::string.
     * @param byteArr A string containing byte data.
     */
    bytearray(const std::string& byteArr) : data_(byteArr.begin(), byteArr.end()) {}

    /**
     * @brief Constructs byte array from a C-style string and size.
     * @param byteArr A C-style string.
     * @param size The size of the string.
     */
    bytearray(const char* byteArr, size_t size) : bytearray(std::string(byteArr, size)) {}

    /**
     * @brief Constructs byte array from a C-style string.
     * @param byteArr A C-style string.
     */
    bytearray(const char* byteArr) : bytearray(std::string(byteArr)) {}

    /**
     * @brief Creates a ByteArray from a hex string.
     *
     * Case is ignored and the string may have a '0x' hex prefix or not.
     *
     * @param hexStr A string containing hex data.
     * @return A ByteArray object.
     * @throws std::invalid_argument If the hex string contains non-hexadecimal characters.
     * @throws std::length_error If the hex string length is not even.
     */
    static bytearray fromHex(const std::string& hexStr) {
        std::string cleanString(hexStr);

        // Check and skip the '0x' prefix if present
        if (cleanString.size() >= 2 && cleanString.substr(0, 2) == "0x") {
            cleanString = cleanString.substr(2);
        }

        size_t size = cleanString.size();
        if (size % 2 != 0) {
            throw std::length_error("Hex string length must be even.");
        }

        bytearray byteArray;
        byteArray.data_.reserve(size / 2);

        for (size_t i = 0; i < size; i += 2) {
            uint8_t byte = static_cast<uint8_t>(std::stoi(cleanString.substr(i, 2), nullptr, 16));
            byteArray.data_.push_back(byte);
        }

        return byteArray;
    }

    /**
     * @overload
     */
    static bytearray fromHex(const char* byteArr) { return fromHex(std::string(byteArr)); }

    /**
     * @overload
     */
    static bytearray fromHex(const char* byteArr, size_t size) { return fromHex(std::string(byteArr, size)); }

    /**
     * @brief Converts the byte array to a lowercase hex string without '0x' prefix.
     * @param spacing Whether to include spaces between bytes.
     *
     * @return A hex string representation of the byte array.
     */
    std::string toHex(bool spacing = false) const {
        std::ostringstream oss;
        for (auto byte : data_) {
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
            if (spacing) {
                oss << " ";
            }
        }
        return oss.str();
    }

    /**
     * @brief Slices  the byte array from a specified start index to an end index.
     *
     * This method creates a new byte array containing bytes from the specified range.
     * The start index is inclusive, while the end index is exclusive.
     *
     * @param start The starting index from which to begin slicing.
     * @param end The ending index up to which to slice (exclusive).
     * @return byte array A new byte array containing the sliced segment.
     * @throws std::out_of_range If the start index is greater than the end index or if the end index is out of bounds.
     */
    bytearray slice(size_t start, size_t end) const {
        if (start > end || end > data_.size()) {
            throw std::out_of_range("Invalid slice range");
        }
        return bytearray(std::vector<uint8_t>(data_.begin() + start, data_.begin() + end));
    }

    /**
     * @brief Slices the byte array from a specified start index to the end of the array.
     *
     * This method creates a new byte array containing all bytes from the specified start index to the end of the
     * byte array.
     *
     * @param start The starting index from which to begin slicing.
     * @return byte array A new byte array containing the sliced segment from the start index to the end.
     * @throws std::out_of_range If the start index is out of the bounds of the byte array.
     */
    bytearray slice_from(size_t start) const { return slice(start, data_.size()); }

    /**
     * @brief Slices the byte array from the beginning to a specified end index.
     *
     * This method creates a new byte array containing all bytes from the beginning of the byte array to the specified
     * end index.
     *
     * @param end The ending index up to which to slice (exclusive).
     * @return byte array A new byte array containing the sliced segment from the beginning to the end index.
     * @throws std::out_of_range If the end index is out of the bounds of the byte array.
     */
    bytearray slice_to(size_t end) const { return slice(0, end); }

    /**
     * @brief Overloaded stream insertion operator for byte array.
     * @param os The output stream.
     * @param byteArray The byte array object.
     * @return The output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const bytearray& byteArray) {
        os << byteArray.toHex(true);
        return os;
    }

    /**
     * @brief Conversion operator to convert byte array to std::string.
     *
     * @note This is provided to support code that relies on byte array
     *       being representd as a string.
     * @return String containing the raw bytes of the byte array
     */
    operator std::string() const { return std::string(data_.begin(), data_.end()); }

    //! @cond Doxygen_Suppress
    // Expose vector-like functionality
    size_t size() const { return data_.size(); }
    const uint8_t* data() const { return data_.data(); }
    bool empty() const { return data_.empty(); }
    void clear() { data_.clear(); }
    uint8_t& operator[](size_t index) { return data_[index]; }
    const uint8_t& operator[](size_t index) const { return data_[index]; }
    void push_back(uint8_t byte) { data_.push_back(byte); }
    auto begin() const { return data_.begin(); }
    auto end() const { return data_.end(); }
    //! @endcond

  private:
    std::vector<uint8_t> data_;
};

}  // namespace kvn

#endif  // KVN_BYTEARRAY_H