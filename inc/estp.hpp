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
#include "estp_frame.hpp"

extern "C" {
#include "estp.h"
}

namespace m_estp {
/**
 * @class Estp
 * @brief Class for handling ESTF frame serialization and deserialization.
 * This class provides methods to process incoming bytes, serialize frames,
 * and calculate checksums according to the ESTP protocol.
 */
class Estp {
    enum class Status : uint8_t {
        kIdle = 0,
        kProcesing = 1,
        kEof = 7,
        kError = 10
    };

    using Frame = EstpFrame<>;
    using FrameCallback = std::function<void(const Frame&)>; /* Callback for processed frames */

public:
    static constexpr uint8_t SOF {0x7E}; /* Start of Frame */
    static constexpr uint8_t ESC {0x7D}; /* Escape character */
    static constexpr uint8_t EOFRM {0x7F}; /* End of Frame */
    static constexpr uint8_t XOR_MASK {0x20}; /* Mask for byte stuffing */

    /* double size for stuffing, SOF and EOF are skipped during byte processing*/
    static constexpr uint16_t kMaxTransmissionSize {(Frame::kMaxFrameSize * 2U)};

    /* Constructor and Destructor */
    Estp() noexcept = default;
    ~Estp() = default;

    /**
     * @brief Process a single byte received from the ESTP stream.
     * @param byte The byte to process.
     */
    void processByte(const uint8_t byte) noexcept;

    /**
     * @brief Set the callback function to be called when a complete frame is received.
     * @param callback User-defined function to handle received frames.
     */
    void setFrameCallback(FrameCallback callback) noexcept
    {
        onFrameReceived_ = callback;
    }

    /**
     * @brief Deserialize a byte buffer into an ESTP frame.
     * @param in_buffer Pointer to the input byte buffer.
     * @param length Length of the input byte buffer.
     * @return Deserialized EstpFrame object.
     */
    static Frame deserialize(const uint8_t* in_buffer, const uint8_t length) noexcept;

    /**
     * @brief Serialize an ESTP frame into a byte buffer.
     * @param frame Reference to the EstpFrame to serialize.
     * @param out_buffer Pointer to the output byte buffer.
     * @return The size of the serialized frame in bytes.
     */
    static uint8_t serialize(const Frame& frame, uint8_t* out_buffer) noexcept;

    /**
     * @brief Calculate checksum for given data.
     * @param data Pointer to the data array.
     * @param length Length of the data array.
     * @return Calculated checksum.
     */
    static uint16_t calculateChecksum(const uint8_t* data, const uint8_t length) noexcept;

private:
    FrameCallback onFrameReceived_ = nullptr; /* User-defined callback for received frames */
    Status rx_status_ {Status::kIdle};
    uint8_t rx_index_ {0};
    std::array<uint8_t, (kMaxTransmissionSize)> rx_buffer_;
};

} // namespace m_estp