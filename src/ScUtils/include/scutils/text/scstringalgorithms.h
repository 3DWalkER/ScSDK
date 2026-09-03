#ifndef SCSTRINGALGORITHMS_H
#define SCSTRINGALGORITHMS_H

#include "scutils/utils/scnamespace.h"
#include <algorithm>
#include <string.h>

class ScStringView;

SC_BEGIN_NAMESPACE

SC_BEGIN_DETAIL_NAMESPACE

template <class Pod>
inline void podCopy(const Pod* b, const Pod* e, Pod* d)
{
	SC_ASSERT(b != nullptr);
	SC_ASSERT(e != nullptr);
	SC_ASSERT(d != nullptr);
	SC_ASSERT(e >= b);
	SC_ASSERT(d >= e || d + (e - b) <= b);
	memcpy(d, b, (e - b) * sizeof(Pod));
}

template <class Pod>
inline void podMove(const Pod* b, const Pod* e, Pod* d)
{
	SC_ASSERT(b != nullptr);
	SC_ASSERT(e != nullptr);
	SC_ASSERT(d != nullptr);
	SC_ASSERT(e >= b);
	memmove(d, b, (e - b) * sizeof(*b));
}

inline size_t strlen_s(const char* str) {
	return str ? strlen(str) : 0;
}

inline size_t strlen_s(const unsigned char* str) {
	return SC_DETAIL::strlen_s(reinterpret_cast<const char*>(str));
}

template <typename Number>
SC_DECL_CONSTEXPR int sclencmp(Number lhs, Number rhs) noexcept
{
	return lhs == rhs ? 0 : lhs > rhs ? 1 : -1;
}

template <size_t N>
inline void strcpy_s(char(&dst)[N], const char* data, size_t length) {
	size_t copylen = length > N - 1 ? N - 1 : length;
	std::copy(data, data + copylen, dst);
	dst[copylen] = '\0';
}

template <size_t N>
inline void strcpy_s(char(&dst)[N], const ScString& src) {
	SC_DETAIL::strcpy_s(dst, src.data(), src.size());
}

template <typename Char>
struct IsCompatibleCharTypeHelper
	: std::integral_constant<bool,
	std::is_same<Char, char>::value ||
	std::is_same<Char, unsigned char>::value> {
};

template <typename Char>
struct IsCompatibleCharType
	: IsCompatibleCharTypeHelper<typename std::remove_cv<typename std::remove_reference<Char>::type>::type> {
};

template <typename Pointer>
struct IsCompatiblePointerHelper : std::false_type {};
template <typename Char>
struct IsCompatiblePointerHelper<Char*> : IsCompatibleCharType<Char> { };
template <typename Pointer>
struct IsCompatiblePointer
	: IsCompatiblePointerHelper<typename std::remove_cv<typename std::remove_reference<Pointer>::type>::type> {
};

SC_API_EXPORT int compareStrings(ScStringView lhs, ScStringView rhs, Sc::CaseSensitivity cs = Sc::CaseSensitive);

SC_API_EXPORT size_t indexOf(ScStringView haystack, size_t from, ScStringView needle, Sc::CaseSensitivity cs = Sc::CaseSensitive);

SC_API_EXPORT size_t lastIndexOf(ScStringView haystack, size_t from, ScStringView needle, Sc::CaseSensitivity cs = Sc::CaseSensitive);

SC_API_EXPORT bool startsWith(ScStringView haystack, ScStringView needle, Sc::CaseSensitivity cs = Sc::CaseSensitive);
SC_API_EXPORT bool startsWith(ScStringView haystack, char c, Sc::CaseSensitivity cs = Sc::CaseSensitive);

SC_API_EXPORT bool  endsWith(ScStringView haystack, ScStringView needle, Sc::CaseSensitivity cs = Sc::CaseSensitive);
SC_API_EXPORT bool  endsWith(ScStringView haystack, char c, Sc::CaseSensitivity cs = Sc::CaseSensitive);

SC_API_EXPORT ScStringView trimmed(ScStringView haystack);

SC_END_DETAIL_NAMESPACE

SC_END_NAMESPACE

#endif // SCSTRINGALGORITHMS_H