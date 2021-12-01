#include <stdlib.h>
#include <string.h>
#include "mqtt.h"
#include "pack.h"

static size_t unpack_mqtt_connect(const unsigned char *, union mqtt_header *, union mqtt_packet *);
static size_t unpack_mqtt_publish(const unsigned char *, union mqtt_header *, union mqtt_packet *);
static size_t unpack_mqtt_subscribe(const unsigned char *, union mqtt_header *, union mqtt_packet *);
static size_t unpack_mqtt_unsubscribe(const unsigned char *, union mqtt_header *, union mqtt_packet *);
static size_t unpack_mqtt_ack(const unsigned char *, union mqtt_header *, union mqtt_packet *);

static unsigned char *pack_mqtt_header(const union mqtt_header *);
static unsigned char *pack_mqtt_ack(const union mqtt_header *);
static unsigned char *pack_mqtt_connack(const union mqtt_header *);
static unsigned char *pack_mqtt_suback(const union mqtt_header *);
static unsigned char *pack_mqtt_publish(const union mqtt_header *);

static unsigned char *pack_mqtt_(const union mqtt_header *);

static const int MAX_LEN_BYTES = 4;

int mqtt_encode_lenght(unsigned char *buf, size_t len) {
    int bytes = 0;
    do {
        if (bytes + 1 > MAX_LEN_BYTES) 
            return bytes;
        short d = len % 128;
        len /= 128;
        if (len > 0)
            d |= 128;
        buf[bytes++] = d;
    } while (len > 0);
    return bytes;
}