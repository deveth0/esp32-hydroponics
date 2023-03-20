#ifndef HYDROPONICS_IMPROV_H
#define HYDROPONICS_IMPROV_H


void handleImprovPacket();
void sendImprovStateResponse(uint8_t state, bool error = false);
void sendImprovInfoResponse();
void sendImprovRPCResponse(uint8_t commandId);

#endif