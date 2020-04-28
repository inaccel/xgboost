#ifndef PACKET_H
#define PACKET_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _packet *packet;

packet packet_create(char *header, char *content, size_t content_length);

void packet_free(packet p);

#ifdef __cplusplus
}
#endif

#endif    // PACKET_H