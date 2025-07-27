#include <stdint.h>
#include "estf.h"
#include "estf_cfg.h"
#include "estf_cbk.h"

ESTF_ReceiverBuffer_t estfRxBuffer = {0};

ESTF_Packet_t ESTF_CreatePacket(uint8_t au8_frameId, uint8_t * au8_dataP, uint8_t au8_dataLength)
{
    ESTF_Packet_t packet;
    packet.frame.frameId = au8_frameId;
    if (au8_dataLength > ESTF_MAX_DATA_LEN)
    {
        au8_dataLength = ESTF_MAX_DATA_LEN; // Limit to max data length
    }
    packet.frame.dataLength = au8_dataLength;

    for (uint8_t i = 0; i < au8_dataLength; i++)
    {
        packet.frame.data[i] = au8_dataP[i];
    }

    // Calculate checksum (simple sum of data bytes)
    packet.frame.checksum = 0;
    for (uint8_t i = 0; i < au8_dataLength; i++)
    {
        packet.frame.checksum += packet.frame.data[i];
    }
    packet.raw[au8_dataLength+ESTF_HEADER_SIZE] = packet.frame.checksum;

    return packet;
}

void ESTF_ProcessByte(uint8_t byte)
{
    estfRxBuffer.buffer[estfRxBuffer.index++] = byte;

    if (estfRxBuffer.index == 2)
    {
        // Just received dataLength — check if it's within bounds
        uint8_t dataLen = estfRxBuffer.buffer[1];
        if (dataLen > ESTF_MAX_DATA_LEN)
        {
            estfRxBuffer.index = 0; // Invalid, reset
            return;
        }
    }

    if (estfRxBuffer.index == (2 + estfRxBuffer.buffer[1] + 1))
    {
        // Full packet received: 1 byte frameId + 1 byte dataLength + data + 1 byte checksum
        ESTF_Packet_t packet;
        for (uint8_t i = 0; i < estfRxBuffer.index; i++)
        {
            packet.raw[i] = estfRxBuffer.buffer[i];
        }
        packet.frame.checksum = estfRxBuffer.buffer[estfRxBuffer.index-1];

        // Validate checksum
        uint8_t sum = 0;
        for (uint8_t i = 0; i < packet.frame.dataLength; i++)
        {
            sum += packet.frame.data[i];
        }

        if (sum == packet.frame.checksum)
        {
            // Packet valid
            ESTF_HandlePacket_cbk(packet);
        }
        else
        {
            // Invalid checksum
        }

        estfRxBuffer.index = 0; // Ready for next packet
    }
}