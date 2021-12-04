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
static unsigned char *pack_mqtt_ack(const union mqtt_packet *);
static unsigned char *pack_mqtt_connack(const union mqtt_packet *);
static unsigned char *pack_mqtt_suback(const union mqtt_packet *);
static unsigned char *pack_mqtt_publish(const union mqtt_packet *);

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

unsigned long long mqtt_decode_length(const unsigned char **buf) {
    char c;
    int multiplier = 1;
    unsigned long long value = 0LL;
    do {
        c = **buf;
        value += (c & 127) * multiplier;
        (*buf)++;
    } while ((c &128) != 0);
    return value;
}

static size_t unpack_mqtt_connect(const unsigned char *buf, union mqtt_header *hdr, union mqtt_packet *pkt) {
    struct mqtt_connect connect = { .header = *hdr };
    pkt->connect = connect;
    const unsigned char *init = buf;

    size_t len = mqtt_decode_length(&buf);

    buf = init + 8;
    pkt->connect.byte = unpack_u8((const uint8_t **) &buf);
    pkt->connect.payload.keepalive = unpack_u16((const uint8_t **) &buf);
    uint16_t cid_len = unpack_u16((const uint8_t **) &buf);

    if (cid_len > 0) {
        pkt->connect.payload.client_id = malloc(cid_len + 1);
        unpack_bytes((const uint8_t **) &buf, cid_len, pkt->connect.payload.client_id);
    }

    if (pkt->connect.bits.will == 1) {
        unpack_string16(&buf, &pkt->connect.payload.will_topic);
        unpack_string16(&buf, &pkt->connect.payload.will_message);
    }

    if (pkt->connect.bits.username == 1)
        unpack_string16(&buf, &pkt->connect.payload.username);

    if (pkt->connect.bits.password == 1)
        unpack_string16(&buf, &pkt->connect.payload.password);
    return len;
}

static size_t unpack_mqtt_publish(const unsigned char *buf,
                                  union mqtt_header *hdr,
                                  union mqtt_packet *pkt) {
    struct mqtt_publish publish = { .header = *hdr };
    pkt->publish = publish;
    /*
     * Second byte of the fixed header, contains the length of remaining bytes
     * of the connect packet
     */
    size_t len = mqtt_decode_length(&buf);
    /* Read topic length and topic of the soon-to-be-published message */
    pkt->publish.topiclen = unpack_string16(&buf, &pkt->publish.topic);
    uint16_t message_len = len;
    /* Read packet id */
    if (publish.header.bits.qos > AT_MOST_ONCE) {
        pkt->publish.pkt_id = unpack_u16((const uint8_t **) &buf);
        message_len -= sizeof(uint16_t);
    }
    /*
     * Message len is calculated subtracting the length of the variable header
     * from the Remaining Length field that is in the Fixed Header
     */
    message_len -= (sizeof(uint16_t) + topiclen);
    pkt->publish.payloadlen = message_len;
    pkt->publish.payload = malloc(message_len + 1);
    unpack_bytes((const uint8_t **) &buf, message_len, pkt->publish.payload);
    return len;
}

static size_t unpack_mqtt_subscribe(const unsigned char *buf,
                                    union mqtt_header *hdr,
                                    union mqtt_packet *pkt) {
    struct mqtt_subscribe subscribe = { .header = *hdr };
    size_t len = mqtt_decode_length(&buf);
    size_t remaining_bytes = len;
    subscribe.pkt_id = unpack_u16((const uint8_t **) &buf);
    remaining_bytes -= sizeof(uint16_t);
    int i = 0;
    while (remaining_bytes > 0) {
        remaining_bytes -= sizeof(uint16_t);
        subscribe.tuples = realloc(subscribe.tuples, (i+1) * sizeof(*subscribe.tuples));
        subscribe.tuples[i].topic_len = unpack_string16(&buf, &subscribe.tuples[i].topic);
        remaining_bytes -= subscribe.tuples[i].topic_len;
        subscribe.tuples[i].qos = unpack_u8((const uint8_t **) &buf);
        len -= sizeof(uint8_t);
        i++;
    }
    subscribe.tuples_len = i;
    pkt->subscribe = subscribe;
    return len;
}

static size_t unpack_mqtt_unsubscribe(const unsigned char *buf, union mqtt_header *hdr, union mqtt_packet *pkt) {
    struct mqtt_unsubscribe unsubscribe = { .header = *hdr };
    size_t len = mqtt_decode_length(&buf);
    size_t remaining_bytes = len;
    unsubscribe.pkt_id = unpack_u16((const uint8_t **) &buf);
    remaining_bytes -= sizeof(uint16_t);

    int i = 0;
    while (remaining_bytes > 0) {
        remaining_bytes -= sizeof(uint16_t);
        unsubscribe.tuples = realloc(unsubscribe.tuples, (i+1) * sizeof(*unsubscribe.tuples));
        unsubscribe.tuples[i].topic_len = unpack_string16(&buf, &unsubscribe.tuples[i].topic);
        remaining_bytes -= unsubscribe.tuples[i].topic_len;
        i++;
    }

    unsubscribe.tuples_len = i;
    pkt->unsubscribe = unsubscribe;
    return len;
}

static size_t unpack_mqtt_ack(const unsigned char *buf,
                              union mqtt_header *hdr,
                              union mqtt_packet *pkt) {
    struct mqtt_ack ack = { .header = *hdr };
    /*
     * Second byte of the fixed header, contains the length of remaining bytes
     * of the connect packet
     */
    size_t len = mqtt_decode_length(&buf);
    ack.pkt_id = unpack_u16((const uint8_t **) &buf);
    pkt->ack = ack;
    return len;
}

typedef size_t mqtt_unpack_handler(const unsigned char *,
                                   union mqtt_header *,
                                   union mqtt_packet *);

static mqtt_unpack_handler *unpack_handlers[11] = {
    NULL,
    unpack_mqtt_connect,
    NULL,
    unpack_mqtt_publish,
    unpack_mqtt_ack,
    unpack_mqtt_ack,
    unpack_mqtt_ack,
    unpack_mqtt_ack,
    unpack_mqtt_subscribe,
    NULL,
    unpack_mqtt_unsubscribe
};

int unpack_mqtt_packet(const unsigned char *buf, union mqtt_packet *pkt) {
    int rc = 0;
    /* Read first byte of the fixed header */
    unsigned char type = *buf;
    union mqtt_header header = {
        .byte = type
    };
    if (header.bits.type == DISCONNECT
        || header.bits.type == PINGREQ
        || header.bits.type == PINGRESP)
        pkt->header = header;
    else
        /* Call the appropriate unpack handler based on the message type */
        rc = unpack_handlers[header.bits.type](++buf, &header, pkt);
    return rc;
}

