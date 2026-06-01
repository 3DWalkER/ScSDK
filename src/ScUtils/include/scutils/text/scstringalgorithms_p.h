#ifndef SCSTRINGALGORITHMS_P_H
#define SCSTRINGALGORITHMS_P_H

#include <ctype.h>

template <typename StringType>
struct ScStringAlgorithms
{
	typedef typename StringType::value_type Char;

	static void trimmed_helper_positions(const Char*& begin, const Char*& end);
	
	static StringType trimmed(StringType& str);
};

template<typename StringType>
inline void ScStringAlgorithms<StringType>::trimmed_helper_positions(const Char*& begin, const Char*& end)
{	
	while (begin < end && isspace(static_cast<unsigned char>(end[-1])))
		--end;

	while (begin < end && isspace(static_cast<unsigned char>(*begin)))
		begin++;
}

template<typename StringType>
inline StringType ScStringAlgorithms<StringType>::trimmed(StringType& str)
{
	const Char* begin = str.cbegin();
	const Char* end = str.cend();
	trimmed_helper_positions(begin, end);
	if (begin == str.cbegin() && end = str.cend())
		return str;
	return StringType(begin, end - begin);
}

#endif // SCSTRINGALGORITHMS_P_H
