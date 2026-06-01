#include "scutils/text/scstring.h"

#include "scutils/text/scstringalgorithms_p.h"

ScString::ScString(const char* c, size_t size)
	: d(c, size)
{

}

ScString::ScString(char c)
	: ScString(&c, 1)
{
}

void ScString::resize(size_t size)
{
	const size_t s = this->size();
	if (size > s)
		d.expand(size - s);
	else
		d.shrink(s - size);
	SC_ASSERT(this->size() == size);
}

void ScString::resize(size_t size, char fillChar)
{
	const size_t s = this->size();
	if (size > s)
	{
		const auto delta = size - s;
		auto pData = d.expand(delta);
		std::fill_n(pData, delta, fillChar);
	}
	else
		d.shrink(s - size);
	SC_ASSERT(this->size() == size);
}

ScString& ScString::assign(const char* s, const size_t n)
{
	if (0 == n)
	{
		resize(0);
	}
	else if (size() >= n)
	{
		SC_DETAIL::podMove(s, s + n, d.mutableData());
		d.shrink(size() - n);
	}
	else
	{
		resize(0);
		SC_DETAIL::podCopy(s, s + n, d.expand(n));
	}

	SC_ASSERT(size() == n);
	return *this;
}

ScString& ScString::append(const char* s, const size_t n)
{
	if (SC_UNLIKELY(!n))
		return *this;

	const auto oldSize = size();
	const auto oldData = data();
	auto pData = d.expand(n, true);

	std::less_equal<const char*> le;
	if (SC_UNLIKELY(le(oldData, s)) && !le(oldData + oldSize, s))
	{
		SC_ASSERT(le(s + n, oldData + oldSize));
		s = data() + (s - oldData);
		SC_DETAIL::podMove(s, s + n, pData);
	}
	else
		SC_DETAIL::podCopy(s, s + n, pData);
	return *this;
}

ScString& ScString::remove(size_t pos, size_t nsize)
{
	const int size = this->size();
	if (npos == pos)
		pos = size - 1;

	if (pos > size)
	{
	}
	else if (nsize >= size - pos)
		resize(pos);
	else if (nsize > 0)
	{
		if (d.isShared())
		{
			const int newSize = d.size() - nsize;
			ScStringData newData(d.data(), newSize);
			SC_DETAIL::podCopy(d.data() + pos + nsize, d.data() + newSize, newData.data() + pos);
			d.swap(newData);
		}
		else
		{
			std::copy(begin() + pos + nsize, end(), begin() + pos);
			resize(length() - nsize);
		}
	}
	return *this;
}

int ScString::toInt(bool* ok, int base)
{
	if (isEmpty())
	{
		if (ok)
			*ok = false;
		return 0;
	}

	SC_UNUSED(base);
	return std::atoi(data());
}

ScString ScString::trimmed() const&&
{
	auto begin = cbegin(), end = cend();
	ScStringAlgorithms<ScString>::trimmed_helper_positions(begin, end);
	return ScString(begin, end - begin);
}