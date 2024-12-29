#ifndef PACKET_H_
#define PACKET_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DATA_SIZE 59
#define META_DATA_SIZE 5
#define PACKET_SIZE (DATA_SIZE + META_DATA_SIZE)

#define PACKET_VERSION_IDX 0
#define PACKET_ID_IDX 1
#define PACKET_MSG_ID_IDX 2
#define PACKET_MSG_COUNT_IDX 3
#define PACKET_DATA_TYPE_IDX 4

typedef enum {
  BME280 = 1,
  BMA400 = 2,
  MQ2 = 3,
  GPS = 4,
  STATUS = 5,
  BMM150 = 6,
  SMS = 32
} DataType;

typedef struct {
  uint8_t version;         /**< 4 bits version + 4 bits reserved for later use */
  uint8_t id;              /**< 4 bits class + 4 bits device ID */
  uint8_t msgID;           /**< 1 byte message ID */
  uint8_t msgCount;        /**< 1 byte message count (optional) */
  DataType dataType;       /**< 1 byte data type (see DataType enum) */
  uint8_t data[DATA_SIZE]; /**< Payload data */
} packet_t;

#ifdef __cplusplus
}
#endif

#endif /* PACKET_H_ */