#include "scnet/utils/scwebsocketutils_hp.h"

#include "scutils/text/scstring.h"

#include <vector>

constexpr char kWebSocketGuid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
constexpr char kBase64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

SC_BEGIN_NAMESPACE

ScString base64Encode(const scuint8* data, size_t length)
{
	ScString result;
	result.reserve(((length + 2) / 3) * 4);

	for (std::size_t i = 0; i < length; i += 3)
	{
		const unsigned int value = (static_cast<unsigned int>(data[i]) << 16)
			| ((i + 1 < length ? static_cast<unsigned int>(data[i + 1]) : 0U) << 8)
			| (i + 2 < length ? static_cast<unsigned int>(data[i + 2]) : 0U);

		result.push_back(kBase64Table[(value >> 18) & 0x3F]);
		result.push_back(kBase64Table[(value >> 12) & 0x3F]);
		result.push_back(i + 1 < length ? kBase64Table[(value >> 6) & 0x3F] : '=');
		result.push_back(i + 2 < length ? kBase64Table[value & 0x3F] : '=');
	}

	return result;
}

std::array<scuint8, 20> sha1Digest(const ScString& input)
{
	std::vector<scuint8> data(input.begin(), input.end());
	const std::uint64_t bitLength = static_cast<std::uint64_t>(data.size()) * 8ULL;

	data.push_back(0x80);
	while ((data.size() % 64) != 56)
		data.push_back(0x00);

	for (int shift = 56; shift >= 0; shift -= 8)
		data.push_back(static_cast<scuint8>((bitLength >> shift) & 0xFF));

	std::uint32_t h0 = 0x67452301U;
	std::uint32_t h1 = 0xEFCDAB89U;
	std::uint32_t h2 = 0x98BADCFEU;
	std::uint32_t h3 = 0x10325476U;
	std::uint32_t h4 = 0xC3D2E1F0U;

	for (std::size_t offset = 0; offset < data.size(); offset += 64)
	{
		std::uint32_t words[80] = {};

		for (std::size_t i = 0; i < 16; ++i)
		{
			const std::size_t index = offset + i * 4;
			words[i] = (static_cast<std::uint32_t>(data[index]) << 24)
				| (static_cast<std::uint32_t>(data[index + 1]) << 16)
				| (static_cast<std::uint32_t>(data[index + 2]) << 8)
				| static_cast<std::uint32_t>(data[index + 3]);
		}

		for (std::size_t i = 16; i < 80; ++i)
			words[i] = leftRotate(words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16], 1);

		std::uint32_t a = h0;
		std::uint32_t b = h1;
		std::uint32_t c = h2;
		std::uint32_t d = h3;
		std::uint32_t e = h4;

		for (std::size_t i = 0; i < 80; ++i)
		{
			std::uint32_t f = 0;
			std::uint32_t k = 0;

			if (i < 20)
			{
				f = (b & c) | ((~b) & d);
				k = 0x5A827999U;
			}
			else if (i < 40)
			{
				f = b ^ c ^ d;
				k = 0x6ED9EBA1U;
			}
			else if (i < 60)
			{
				f = (b & c) | (b & d) | (c & d);
				k = 0x8F1BBCDCU;
			}
			else
			{
				f = b ^ c ^ d;
				k = 0xCA62C1D6U;
			}

			const std::uint32_t temp = leftRotate(a, 5) + f + e + k + words[i];
			e = d;
			d = c;
			c = leftRotate(b, 30);
			b = a;
			a = temp;
		}

		h0 += a;
		h1 += b;
		h2 += c;
		h3 += d;
		h4 += e;
	}

	std::array<scuint8, 20> digest{};
	const std::uint32_t hashParts[5] = { h0, h1, h2, h3, h4 };
	for (std::size_t i = 0; i < 5; ++i)
	{
		digest[i * 4] = static_cast<scuint8>((hashParts[i] >> 24) & 0xFF);
		digest[i * 4 + 1] = static_cast<scuint8>((hashParts[i] >> 16) & 0xFF);
		digest[i * 4 + 2] = static_cast<scuint8>((hashParts[i] >> 8) & 0xFF);
		digest[i * 4 + 3] = static_cast<scuint8>(hashParts[i] & 0xFF);
	}

	return digest;
}

bool isValidUtf8(const scuint8* data, size_t length)
{
	std::size_t index = 0;
	while (index < length)
	{
		const scuint8 lead = data[index];
		std::size_t width = 0;
		std::uint32_t codePoint = 0;

		if ((lead & 0x80U) == 0)
		{
			width = 1;
			codePoint = lead;
		}
		else if ((lead & 0xE0U) == 0xC0U)
		{
			width = 2;
			codePoint = lead & 0x1FU;
		}
		else if ((lead & 0xF0U) == 0xE0U)
		{
			width = 3;
			codePoint = lead & 0x0FU;
		}
		else if ((lead & 0xF8U) == 0xF0U)
		{
			width = 4;
			codePoint = lead & 0x07U;
		}
		else
			return false;

		if (index + width > length)
			return false;

		for (std::size_t i = 1; i < width; ++i)
		{
			const scuint8 continuation = data[index + i];
			if ((continuation & 0xC0U) != 0x80U)
				return false;
			codePoint = (codePoint << 6U) | (continuation & 0x3FU);
		}

		if ((width == 2 && codePoint < 0x80U)
			|| (width == 3 && codePoint < 0x800U)
			|| (width == 4 && codePoint < 0x10000U)
			|| codePoint > 0x10FFFFU
			|| (codePoint >= 0xD800U && codePoint <= 0xDFFFU))
			return false;

		index += width;
	}

	return true;
}

ScString makeWebSocketAcceptValue(const ScString& value)
{
	const ScString source = value + kWebSocketGuid;
	const std::array<scuint8, 20> hashBuffer = sha1Digest(source);
	return base64Encode(hashBuffer.data(), hashBuffer.size());
}

SC_END_NAMESPACE