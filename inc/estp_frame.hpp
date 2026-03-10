/**
 * @file estp.hpp
 * @brief C++ class definition for embedded module.
 *
 * This header defines the internal C++ class used by the module.
 * It is not exposed to C projects directly — only via the C facade.
 */

#pragma once
#include <cstdint>
#include <array>
#include <algorithm>
#include <functional>

namespace m_estp {
/**
 * @class EstpFrame
 * @brief Class representing a single ESTP frame.
 * This class encapsulates the frame ID, data payload, length, and checksum.
 */
template<uint16_t MAX_DATA_LENGTH = 255>
class EstpFrame {
public:
    static constexpr uint16_t kMaxDataLength {MAX_DATA_LENGTH};
    static constexpr uint8_t kHeaderSize {3}; // frameId (2 bytes) + dataLength
    static constexpr uint8_t kChecksumSize {2};
    static constexpr uint16_t kMaxFrameSize {kHeaderSize + kMaxDataLength + kChecksumSize};
    static constexpr uint8_t kMinRawFrameSize {2 + kHeaderSize + 0 + kChecksumSize}; // SOF + header + checksum + EOF
    static constexpr uint8_t kSecuritySize {2}; // 2 bytes for the Rolling Counter

    EstpFrame() noexcept;
    EstpFrame(const uint16_t frame_id, const uint8_t* data_p, const uint8_t data_length) noexcept;
    EstpFrame(const uint16_t frame_id, const uint8_t* data_p, const uint8_t data_length, const uint16_t checksum) noexcept;
    ~EstpFrame() = default;

    /**
     * @brief Get the length of the data payload.
     * @return Length of the data array.
     */
    uint16_t getLength() const noexcept;

    /**
     * @brief Get the frame ID.
     * @return Frame ID.
     */
    uint16_t getFrameId() const noexcept;

    /**
     * @brief Get pointer to the data payload.
     * @return Pointer to the data array.
     */
    const uint8_t* getData() const noexcept;

    /**
     * @brief Calculate and update the checksum of the frame.
     */
    void calculateChecksum() noexcept;

    /**
     * @brief Get the checksum of the frame.
     * @return The checksum value.
     */
    uint16_t getChecksum() const noexcept;

    /**
     * @brief Validate the checksum of the frame - raw data vs. calculated checksum.
     * @return true if checksum is valid, false otherwise.
     */
    bool validateChecksum() const noexcept;

    /**
     * @brief Get the total size of the frame including header, data, and checksum.
     * @return Total frame size in bytes.
     */
    uint8_t getFrameSize() const noexcept;

    void protect(uint16_t secret_key) noexcept;
    bool unprotect(uint16_t secret_key, uint16_t last_counter) noexcept;

    void setCounter(uint16_t c) { rolling_counter_ = c; }
    uint16_t getCounter() const { return rolling_counter_; }

    void setFrameId(const uint16_t frame_id) noexcept;
    void setData(const uint8_t* dataP, uint8_t dataLength) noexcept;

    static constexpr bool isCounterNew(uint16_t incoming, uint16_t last) noexcept
    {
        return (incoming > last) || (last - incoming > 32768);
    }

private:
    static uint16_t calculateChecksum_(const uint8_t* data, uint16_t length) noexcept;

    uint16_t frame_id_;
    uint16_t rolling_counter_;
    uint16_t data_length_;
    uint16_t checksum_;
    std::array<uint8_t, MAX_DATA_LENGTH> data_;
};

template<uint16_t MAX_DATA_LENGTH>
EstpFrame<MAX_DATA_LENGTH>::EstpFrame() noexcept
        : frame_id_(0),
            rolling_counter_(0),
            data_length_(0),
            checksum_(0)
{
        data_.fill(0);
}

template<uint16_t MAX_DATA_LENGTH>
EstpFrame<MAX_DATA_LENGTH>::EstpFrame(const uint16_t frame_id, const uint8_t* data_p, const uint8_t data_length) noexcept
    : frame_id_(frame_id),
            rolling_counter_(0),
      data_length_(data_length),
      checksum_(0)
{
    if (data_length_ > kMaxDataLength) {
        data_length_ = kMaxDataLength;
    }
    std::copy(data_p, data_p + data_length_, data_.begin());
    calculateChecksum();
}

template<uint16_t MAX_DATA_LENGTH>
EstpFrame<MAX_DATA_LENGTH>::EstpFrame(const uint16_t frame_id, const uint8_t* data_p, const uint8_t data_length, const uint16_t checksum) noexcept
    : frame_id_(frame_id),
    rolling_counter_(0),
      data_length_(data_length),
      checksum_(checksum)
{
    if (data_length_ > kMaxDataLength) {
        data_length_ = kMaxDataLength;
    }
    std::copy(data_p, data_p + data_length_, data_.begin());
}

template<uint16_t MAX_DATA_LENGTH>
uint16_t EstpFrame<MAX_DATA_LENGTH>::getLength() const noexcept
{
    return data_length_;
}

template<uint16_t MAX_DATA_LENGTH>
uint8_t EstpFrame<MAX_DATA_LENGTH>::getFrameSize() const noexcept
{
    return static_cast<uint8_t>(kHeaderSize + data_length_ + kChecksumSize);
}

template<uint16_t MAX_DATA_LENGTH>
uint16_t EstpFrame<MAX_DATA_LENGTH>::getFrameId() const noexcept
{
    return frame_id_;
}

template<uint16_t MAX_DATA_LENGTH>
const uint8_t* EstpFrame<MAX_DATA_LENGTH>::getData() const noexcept
{
    return data_.data();
}

template<uint16_t MAX_DATA_LENGTH>
void EstpFrame<MAX_DATA_LENGTH>::calculateChecksum() noexcept
{
    checksum_ = calculateChecksum_(data_.data(), data_length_);
}

template<uint16_t MAX_DATA_LENGTH>
uint16_t EstpFrame<MAX_DATA_LENGTH>::getChecksum() const noexcept
{
    return checksum_;
}

/**
 * @brief Validate the checksum of the frame.
 * @return true if checksum is valid, false otherwise.
 */
template<uint16_t MAX_DATA_LENGTH>
bool EstpFrame<MAX_DATA_LENGTH>::validateChecksum() const noexcept
{
    uint16_t calculated_checksum {0};
    calculated_checksum = calculateChecksum_(data_.data(), data_length_);
    return (calculated_checksum == checksum_);
}

template<uint16_t MAX_DATA_LENGTH>
void EstpFrame<MAX_DATA_LENGTH>::setFrameId(const uint16_t frame_id) noexcept
{
    frame_id_ = frame_id;
}

/**
 * @brief Set data for the frame.
 * @param dataP Pointer to the data array.
 * @param dataLength Length of the data array.
 * @note If dataLength exceeds kMaxDataLength, it will be truncated. Checksum is recalculated.
 */
template<uint16_t MAX_DATA_LENGTH>
void EstpFrame<MAX_DATA_LENGTH>::setData(const uint8_t* dataP, uint8_t dataLength) noexcept
{
    if (dataLength > kMaxDataLength) {
        dataLength = kMaxDataLength;
    }
    data_length_ = dataLength;
    std::copy(dataP, dataP + dataLength, data_.begin());
    calculateChecksum();
}

template<uint16_t MAX_DATA_LENGTH>
void EstpFrame<MAX_DATA_LENGTH>::protect(uint16_t secret_key) noexcept
{
    // We use the counter to mutate the key so the cipher text 
    // changes even if the data remains the same.
    uint16_t dynamic_key = secret_key ^ rolling_counter_;
    uint8_t key_low = static_cast<uint8_t>(dynamic_key & 0xFF);
    uint8_t key_high = static_cast<uint8_t>(dynamic_key >> 8);

    for (uint16_t i = 0; i < data_length_; ++i) {
        // Alternating byte XOR for a bit more entropy
        data_[i] ^= (i % 2 == 0) ? key_low : key_high;
    }
    
    // Recalculate checksum AFTER encryption
    calculateChecksum(); 
}

template<uint16_t MAX_DATA_LENGTH>
bool EstpFrame<MAX_DATA_LENGTH>::unprotect(uint16_t secret_key, uint16_t last_counter) noexcept
{
    // STEP 1: Integrity Check
    // Always validate the checksum BEFORE touching the data
    if (!validateChecksum()) {
        return false; 
    }

    // STEP 2: Freshness Check (Anti-Replay)
    // If the counter isn't newer than the last one we saw, it's a replay attack
    if (isCounterNew(rolling_counter_, last_counter) == false)
    {
        return false;
    }

    // STEP 3: Decryption (Symmetric XOR)
    uint16_t dynamic_key = secret_key ^ rolling_counter_;
    uint8_t key_low = static_cast<uint8_t>(dynamic_key & 0xFF);
    uint8_t key_high = static_cast<uint8_t>(dynamic_key >> 8);

    for (uint16_t i = 0; i < data_length_; ++i) {
        data_[i] ^= (i % 2 == 0) ? key_low : key_high;
    }

    return true; // Data is now "clean" and safe to use
}

template<uint16_t MAX_DATA_LENGTH>
uint16_t EstpFrame<MAX_DATA_LENGTH>::calculateChecksum_(const uint8_t* data, uint16_t length) noexcept
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; ++i)
    {
        crc ^= static_cast<uint16_t>(data[i]) << 8;
        for (uint8_t bit = 0; bit < 8; ++bit)
        {
            if ((crc & 0x8000U) != 0U)
            {
                crc = static_cast<uint16_t>((crc << 1) ^ 0x1021U);
            }
            else
            {
                crc = static_cast<uint16_t>(crc << 1);
            }
        }
    }

    return crc;
}

} // namespace m_estp