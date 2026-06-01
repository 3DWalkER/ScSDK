#ifndef SCSTRINGVIEW_H
#define SCSTRINGVIEW_H

#include "scstringalgorithms.h"

class SC_API_EXPORT ScStringView
{
	template <typename Char>
	using if_compatible_char = typename std::enable_if<SC_DETAIL::IsCompatibleCharType<Char>::value, bool>::type;

	template <typename T>
	using if_compatible_scstring_like = typename std::enable_if<std::is_same<T, ScString>::value || std::is_same<T, std::string>::value, bool>::type;

	template <typename Pointer>
	using if_compatible_pointer = typename std::enable_if<SC_DETAIL::IsCompatiblePointer<Pointer>::value, bool>::type;

	template <typename Char>
	static const char* castHelper(const Char* str) noexcept {
		return reinterpret_cast<const char*>(str);
	}

	static SC_DECL_CONSTEXPR const char* castHelper(const char* str) noexcept {
		return str;
	}
public:
	SC_DECL_CONSTEXPR ScStringView() noexcept :m_size(0), m_data(nullptr) {}
	SC_DECL_CONSTEXPR ScStringView(std::nullptr_t) : ScStringView() {}

	template <typename Char, if_compatible_char<Char> = true>
	SC_DECL_CONSTEXPR ScStringView(const Char* str, size_t len)
		: m_size((SC_ASSERT(len >= 0), SC_ASSERT(str || !len), len))
		, m_data(castHelper(str)) {
	}

	template <typename Char, if_compatible_char<Char> = true>
	SC_DECL_CONSTEXPR ScStringView(const Char* begin, const Char* end)
		: ScStringView(begin, end - begin) {
	}

	template <typename String, if_compatible_scstring_like<String> = true>
	ScStringView(const String& str)
		: ScStringView(str.data(), str.length()){
	}

	template <typename Pointer, if_compatible_pointer<Pointer> = true>
	SC_DECL_CONSTEXPR ScStringView(const Pointer& str) noexcept
		: ScStringView(str, str ? SC_DETAIL::strlen_s(str) : 0) {
	}

	SC_REQUIRED_RESULT SC_DECL_CONSTEXPR bool isNull() const noexcept { return !m_data; }
	SC_REQUIRED_RESULT SC_DECL_CONSTEXPR size_t size() const noexcept { return m_size; }
	SC_REQUIRED_RESULT SC_DECL_CONSTEXPR size_t length() const noexcept { return size(); }
	SC_REQUIRED_RESULT SC_DECL_CONSTEXPR bool isEmpty() const noexcept { return 0 == size(); }

	typedef const char value_type;
	typedef value_type& reference;
	typedef value_type& const_reference;
	typedef value_type* pointer;
	typedef value_type* const_pointer;

	typedef pointer iterator;
	typedef const_pointer const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	SC_REQUIRED_RESULT const_pointer data() const noexcept { return reinterpret_cast<const_pointer>(m_data); }
	SC_REQUIRED_RESULT SC_DECL_CONSTEXPR char front() const { return SC_ASSERT(!isEmpty()), m_data[0]; }
	SC_REQUIRED_RESULT SC_DECL_CONSTEXPR char back()  const { return SC_ASSERT(!isEmpty()), m_data[m_size - 1]; }
	SC_REQUIRED_RESULT SC_DECL_CONSTEXPR char first() const { return front(); }
	SC_REQUIRED_RESULT SC_DECL_CONSTEXPR char last()  const { return back(); }

	SC_REQUIRED_RESULT int compare(ScStringView other, Sc::CaseSensitivity cs = Sc::CaseSensitive) const noexcept {
		return SC_DETAIL::compareStrings(*this, other, cs);
	}

	SC_REQUIRED_RESULT size_t indexOf(ScStringView other, size_t pos, Sc::CaseSensitivity cs = Sc::CaseSensitive) const {
		return SC_DETAIL::indexOf(*this, pos, other, cs);
	}

	SC_REQUIRED_RESULT ScStringView trimmed() const noexcept { return SC_DETAIL::trimmed(*this); }

	SC_REQUIRED_RESULT const_iterator begin()   const noexcept { return data(); }
	SC_REQUIRED_RESULT const_iterator end()     const noexcept { return data() + size(); }
	SC_REQUIRED_RESULT const_iterator cbegin()  const noexcept { return begin(); }
	SC_REQUIRED_RESULT const_iterator cend()    const noexcept { return end(); }
	SC_REQUIRED_RESULT const_reverse_iterator rbegin()  const noexcept { return const_reverse_iterator(end()); }
	SC_REQUIRED_RESULT const_reverse_iterator rend()    const noexcept { return const_reverse_iterator(begin()); }
	SC_REQUIRED_RESULT const_reverse_iterator crbegin() const noexcept { return rbegin(); }
	SC_REQUIRED_RESULT const_reverse_iterator crend()   const noexcept { return rend(); }

private:
	size_t m_size;
	const char* m_data;
};

#endif // SCSTRINGVIEW_H