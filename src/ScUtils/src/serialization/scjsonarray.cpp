#include "scutils/serialization/scjson_p.h"

ScJsonArray::ScJsonArray()
	: d(nullptr)
{
}

ScJsonArray::ScJsonArray(const ScJsonArray& other)
	: d(other.d)
{
	if (d)
		d->ref.ref();
}

ScJsonArray::~ScJsonArray()
{
	if (d && !d->ref.deref())
		delete d;
}

int ScJsonArray::size() const
{
	if (!d || !d->json)
		return 0;

	if (-1 == d->json->size)
		d->json->size = d->json->arraySize();
	return d->json->size;
}

ScJsonValue ScJsonArray::at(int i) const
{
	SC_ASSERT(d && i >= 0 && i < size());
	if (!d || !d->json || i < 0 || i >= size())
		return ScJsonValue(ScJsonValue::Invalid);
	return d->json->toValue(i);;
}

ScJsonValue ScJsonArray::first() const
{
	if (isEmpty())
		return ScJsonValue(ScJsonValue::Invalid);
	return at(0);
}

ScJsonValue ScJsonArray::last() const
{
	if (isEmpty())
		return ScJsonValue(ScJsonValue::Invalid);
	return at(size() - 1);
}

void ScJsonArray::prepend(const ScJsonValue& value)
{
	insert(0, value);
}

void ScJsonArray::append(const ScJsonValue& value)
{
	insert(size(), value);
}

void ScJsonArray::insert(int i, const ScJsonValue& value)
{
	const int size = this->size();
	const int max = size < 0 ? 0 : size;
	SC_ASSERT(i >= 0 && i <= max);
	if (i < 0 || i > max)
		return;

	if (!ScJsonData::detachArray(d))
		return;

	if (d->json->insertArrayItem(i, value) && -1 != d->json->size)
		d->json->size += 1;
}

void ScJsonArray::replace(int i, const ScJsonValue& value)
{
	const int size = this->size();
	const int max = size < 0 ? 0 : size;
	SC_ASSERT(i >= 0 && i <= max);
	if (!d || !d->json || i < 0 || i > max)
		return;

	if (ScJsonData::detachArray(d))
		d->json->replaceArrayItem(i, value);
}

void ScJsonArray::removeAt(int i)
{
	const int size = this->size();
	const int max = size < 0 ? 0 : size;
	SC_ASSERT(i >= 0 && i <= max);
	if (!d || !d->json || i < 0 || i > max)
		return;

	if (!ScJsonData::detachArray(d))
		return;

	if (d->json->removeArrayItem(i) && -1 != d->json->size)
		d->json->size -= 1;
}

ScJsonValue ScJsonArray::takeAt(int i)
{
	const int size = this->size();
	const int max = size < 0 ? 0 : size;
	SC_ASSERT(i >= 0 && i <= max);
	if (!d || !d->json || i < 0 || i > max)
		return ScJsonValue::Invalid;

	ScJsonValue value = at(i);
	removeAt(i);
	return value;
}

ScJsonArray& ScJsonArray::operator=(const ScJsonArray& other)
{
	d = other.d;
	if (nullptr != d)
		d->ref.ref();
	return *this;
}