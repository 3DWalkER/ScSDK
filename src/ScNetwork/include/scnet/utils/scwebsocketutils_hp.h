#ifndef SCWEBSOCKETUTILS_HP_H
#define SCWEBSOCKETUTILS_HP_H

#include "scutils/scglobal.h"
#include <array>

SC_BEGIN_NAMESPACE

constexpr scuint8 kWsOpContinuation = 0x0;
constexpr scuint8 kWsOpText = 0x1;
constexpr scuint8 kWsOpBinary = 0x2;
constexpr scuint8 kWsOpClose = 0x8;
constexpr scuint8 kWsOpPing = 0x9;
constexpr scuint8 kWsOpPong = 0xA;

scuint32 leftRotate(scuint32 value, scuint32 bits) {
	return (value << bits) | (value >> (32U - bits));
}

ScString base64Encode(const scuint8* data, size_t length);

std::array<scuint8, 20> sha1Digest(const ScString& input);

bool isValidUtf8(const scuint8* data, size_t length);

ScString makeWebSocketAcceptValue(const ScString &value);

SC_END_NAMESPACE

#endif // SCWEBSOCKETUTILS_HP_H