/**
 * @file estp.cpp
 * @brief C++ class implementation
 */

#include "estp.hpp"   // C++ class definition
namespace m_estp {
/********************************************************************** */
/**
 * ESTF handler class implementation
 */

void Estp::processByte(const uint8_t byte) noexcept
{
    if (byte == SOF)
    {
        /* Start of Frame detected */
        rx_status_ = Estp::Status::kProcesing;
        rx_buffer_.fill(0);
        rx_index_ = 0;
    }
    else if (byte == EOFRM && rx_status_ == Estp::Status::kProcesing)
    {
        /* End of Frame detected */
        rx_status_ = Estp::Status::kEof;
        /* Invoke callback if set */
        if (onFrameReceived_ != nullptr)
        {
            /* Deserialize the received buffer into a frame and invoke the callback */
            EstpFrame<> frame = deserialize(rx_buffer_.data(), rx_index_);
            onFrameReceived_(frame);
        }
        rx_status_ = Estp::Status::kIdle;
        /* RX is done */
    }
    else if (rx_status_ == Estp::Status::kProcesing)
    {
        /* Store byte in buffer */
        if (rx_index_ < kMaxTransmissionSize)
        {
            rx_buffer_[rx_index_++] = byte;
        }
        else
        {
            /* Buffer overflow, set error status */
            rx_status_ = Estp::Status::kError;
        }
    }
    else
    {
        /* Ignore byte */
    }
}

/**
 * @brief Deserialize a byte buffer into an ESTP frame.
 * @param in_buffer Pointer to the input byte buffer.
 * @param length Length of the input byte buffer.
 * @return Deserialized EstpFrame object.
 */
EstpFrame<> Estp::deserialize(const uint8_t* in_buffer, const uint8_t length) noexcept
{
    if (length < EstpFrame<>::kHeaderSize + EstpFrame<>::kChecksumSize)
    {
        return EstpFrame<>(); // Return empty frame on error
    }
    uint8_t index {0};
    // Extract frame ID, data length, data, and checksum, handling byte stuffing
    uint16_t frame_id {0};
    uint8_t data_length {0};
    std::array<uint8_t, EstpFrame<>::kMaxDataLength> data {};
    uint16_t checksum {0};
    // Frame ID
    // first byte handling - can be either ID or SOF, depends on implementation
    uint8_t byte = in_buffer[index++];
    if (byte == SOF)
    {
        // ignore SOF character
        byte = in_buffer[index++];
    }
    if (byte == ESC)
    {
        byte = in_buffer[index++] ^ XOR_MASK;
    }
    frame_id = static_cast<uint16_t>(byte << 8);
    byte = in_buffer[index++];
    if (byte == ESC)
    {
        byte = in_buffer[index++] ^ XOR_MASK;
    }
    frame_id |= static_cast<uint16_t>(byte);
    // Data Length
    byte = in_buffer[index++];
    if (byte == ESC)
    {
        byte = in_buffer[index++] ^ XOR_MASK;
    }
    data_length = byte;
    // Data
    for (uint8_t i {0}; i < data_length; i++)
    {
        byte = in_buffer[index++];
        if (byte == ESC)
        {
            byte = in_buffer[index++] ^ XOR_MASK;
        }
        data[i] = byte;
    }
    // Checksum
    byte = in_buffer[index++];
    if (byte == ESC)
    {
        byte = in_buffer[index++] ^ XOR_MASK;
    }
    checksum = static_cast<uint16_t>(byte << 8);
    byte = in_buffer[index++];
    if (byte == ESC)
    {
        byte = in_buffer[index++] ^ XOR_MASK;
    }
    checksum |= static_cast<uint16_t>(byte);
    return EstpFrame<>(frame_id, data.data(), data_length, checksum);
}

/**
 * @brief Serialize an ESTP frame into a byte buffer.
 * @param frame Reference to the EstpFrame to serialize.
 * @param out_buffer Pointer to the output byte buffer.
 * @return The size of the serialized frame in bytes.
 */
uint8_t Estp::serialize(const EstpFrame<>& frame, uint8_t* out_buffer) noexcept
{
    uint8_t index {0};
    out_buffer[index++] = SOF;

    // Serialize frame ID
    uint16_t frame_id {frame.getFrameId()};
    uint8_t frame_id_high {static_cast<uint8_t>((frame_id >> 8) & 0xFF)};
    uint8_t frame_id_low {static_cast<uint8_t>(frame_id & 0xFF)};
    if (frame_id_high == SOF || frame_id_high == ESC || frame_id_high == EOFRM)
    {
        out_buffer[index++] = ESC;
        out_buffer[index++] = frame_id_high ^ XOR_MASK;
    }
    else
    {
        out_buffer[index++] = frame_id_high;
    }
    if (frame_id_low == SOF || frame_id_low == ESC || frame_id_low == EOFRM)
    {
        out_buffer[index++] = ESC;
        out_buffer[index++] = frame_id_low ^ XOR_MASK;
    }
    else
    {
        out_buffer[index++] = frame_id_low;
    }

    // Serialize data length
    uint8_t data_length {static_cast<uint8_t>(frame.getLength())};
    if (data_length == SOF || data_length == ESC || data_length == EOFRM)
    {
        out_buffer[index++] = ESC;
        out_buffer[index++] = data_length ^ XOR_MASK;
    }
    else
    {
        out_buffer[index++] = data_length;
    }

    // Serialize data
    const uint8_t* data {frame.getData()};
    for (uint8_t i {0}; i < data_length; i++)
    {
        if (data[i] == SOF || data[i] == ESC || data[i] == EOFRM)
        {
            out_buffer[index++] = ESC;
            out_buffer[index++] = data[i] ^ XOR_MASK;
        }
        else
        {
            out_buffer[index++] = data[i];
        }
    }

    // Serialize checksum
    uint16_t checksum { frame.getChecksum() };
    uint8_t checksum_high {static_cast<uint8_t>((checksum >> 8) & 0xFF)};
    uint8_t checksum_low {static_cast<uint8_t>(checksum & 0xFF)};

    if (checksum_high == SOF || checksum_high == ESC || checksum_high == EOFRM)
    {
        out_buffer[index++] = ESC;
        out_buffer[index++] = checksum_high ^ XOR_MASK;
    }
    else
    {
        out_buffer[index++] = checksum_high;
    }

    if (checksum_low == SOF || checksum_low == ESC || checksum_low == EOFRM)
    {
        out_buffer[index++] = ESC;
        out_buffer[index++] = checksum_low ^ XOR_MASK;
    }
    else
    {
        out_buffer[index++] = checksum_low;
    }
    out_buffer[index++] = EOFRM;

    return index; // Return the total length of the serialized frame
}

/**
 * @brief Calculate checksum for given data.
 * @param data Pointer to the data array.
 * @param length Length of the data array.
 * @return Calculated checksum.
 */
uint16_t Estp::calculateChecksum(const uint8_t* data, const uint8_t length) noexcept
{
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < length; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

} // namespace m_estp