# M_ESTF
ERMA Simple Transfer Protocol

## Frame

| Field | Size | Content |
|---|---|---|
| `SOF` | 1 | `0x7E` |
| `FRAME_ID` | 2 | Frame ID, high byte first |
| `LEN` | 1 | Data length, 0-255 |
| `DATA` | `LEN` | Data |
| `CRC` | 2 | CRC-16/CCITT-FALSE (polynomial `0x1021`, start `0xFFFF`), high byte first |
| `EOF` | 1 | `0x7F` |

The CRC covers `FRAME_ID`, `LEN`, and `DATA` in this order (the bytes before stuffing), so a
corrupted frame ID or length is detected. Inside the frame, `0x7D`, `0x7E`, and `0x7F` are sent as
`0x7D` followed by the byte XOR `0x20`; `SOF` and `EOF` are not stuffed.

Until 2026-10-10 the CRC covered `DATA` only. Both ends of a link must use the same version.

Example: frame ID `0x2001`, data `00 16 03 00 00 00`: CRC over `20 01 06 00 16 03 00 00 00` is
`0x0B7B`, the frame is `7E 20 01 06 00 16 03 00 00 00 0B 7B 7F`. More examples are the test vectors
of `M_LedBoardProtocol` (`test_vectors.json`).
