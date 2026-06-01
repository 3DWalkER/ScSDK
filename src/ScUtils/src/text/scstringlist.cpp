#include "scutils/text/scstringlist.h"

#include "scutils/utils/scnumeric.h"
#include <stdexcept>

ScStringListData ScStringListData::shared_null = { SC_REFCOUNT_INITIALIZE_STATIC , {} };

static void detach_grow(ScStringListData *& d, size_t index, size_t num);

ScStringList::ScStringList()
	: d(&ScStringListData::shared_null)
{
	
}

ScStringList::ScStringList(const ScStringList& other)
	: d(other.d)
{
	if (d->ref.isSharable())
		d->ref.ref();
}

ScStringList::ScStringList(std::initializer_list<ScString> args)
	: d(new ScStringListData())
{
	d->list = std::move(std::vector<ScString>(args));
	d->ref.setSharable(true);
}

ScStringList::~ScStringList()
{
	if (!d->ref.deref())
		delete d;
}

ScStringList& ScStringList::operator+=(const ScStringList& list)
{
	if (!list.isEmpty())
	{
		if (&ScStringListData::shared_null != d)
		{
			const auto& l = list.d->list;
			if (d->ref.isShared())
				::detach_grow(d, 0, list.size());
			d->list.insert(d->list.end(), l.begin(), l.end());
		}
		else
			*this = list;
	}
	return *this;
}

ScStringList& ScStringList::operator+=(const ScString& s)
{
	if (&ScStringListData::shared_null != d)
	{
		if (d->ref.isShared())
			::detach_grow(d, 0, 1);
	}
	else
	{
		d = new ScStringListData();
		d->ref.setSharable(true);
	}
	d->list.push_back(s);
	return *this;
}

void ScStringList::detach()
{
	if (!d->ref.isShared())
		return;

	ScStringListData* newData = new ScStringListData();
	newData->list = d->list;
	newData->ref.setSharable(true);

	d->ref.deref();
	d = newData;
}

void ScStringList::prepend(const ScString& s)
{
	if (&ScStringListData::shared_null != d)
	{
		if (d->ref.isShared())
			::detach_grow(d, 1, 1);
	}
	else
	{
		d = new ScStringListData();
		d->ref.setSharable(true);
	}
	d->list.insert(d->list.begin(), s);
}

void ScStringList::prepend(const ScStringList& list)
{
	const int size = list.size();
	if (0 == size)
		return;

	if (&ScStringListData::shared_null != d)
	{
		const auto& l = list.d->list;
		if (d->ref.isShared())
			::detach_grow(d, size, size);
		d->list.insert(d->list.begin(), l.begin(), l.end());
	}
	else
		*this = list;
}

void detach_grow(ScStringListData*& d, size_t index, size_t num)
{
	size_t newlen = 0;
	if (Sc::add_overflow(d->list.size(), num, &newlen))
		throw std::length_error("");

	ScStringListData* newData = new ScStringListData();
	newData->list.reserve(newlen);
	newData->list.insert(newData->list.begin() + index, d->list.begin(), d->list.end());
	newData->ref.setSharable(true);

	d->ref.deref();
	d = newData;
}