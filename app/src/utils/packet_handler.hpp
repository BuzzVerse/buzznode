#pragma once

#include <pb_decode.h>
#include <pb_encode.h>

#include "protobufs/buzzverse/packet.pb.h"

class PacketHandler {
 public:
  static bool encode(const buzzverse_v1_Packet& packet, uint8_t* buffer, size_t& size) {
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buzzverse_v1_Packet_size);
    bool status = pb_encode(&stream, buzzverse_v1_Packet_fields, &packet);
    size = stream.bytes_written;
    return status;
  }

  static bool decode(const uint8_t* buffer, size_t size, buzzverse_v1_Packet& packet) {
    pb_istream_t stream = pb_istream_from_buffer(buffer, size);
    return pb_decode(&stream, buzzverse_v1_Packet_fields, &packet);
  }
};