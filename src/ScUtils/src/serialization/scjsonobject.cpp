#include "scutils/serialization/scjson_p.h"

ScJsonObject::ScJsonObject()
	: d(nullptr)
{
}

ScJsonObject::ScJsonObject(const ScJsonObject& other)
	: d(other.d)
{
	if (d)
		d->ref.ref();
}

ScJsonObject::~ScJsonObject()
{
	if (d && !d->ref.deref())
		delete d;
}

int ScJsonObject::size() const
{
	if (!d || !d->json)
		return 0;

	if (-1 == d->json->size)
		d->json->size = d->json->objectSize();
	return d->json->size;
}

void ScJsonObject::remove(const ScString& key)
{
	if (ScJsonData::detachObject(d))
	{
		if (d->json->removeObjectItem(key) && -1 != d->json->size)
			d->json->size -= 1;
	}
}

bool ScJsonObject::contains(const ScString& key) const
{
	if (!d || !d->json)
		return false;
	return d->json->containsObjectItem(key);
}

ScString ScJsonObject::keyAt(int i) const
{
	SC_ASSERT(d && d->json && i >= 0 && i < size());
	if (!d || !d->json || i < 0 || i >= size())
		return ScString();
	return d->json->objectKeyAt(i);
}

ScJsonValue ScJsonObject::valueAt(int i) const
{
	SC_ASSERT(d && d->json && i >= 0 && i < size());
	if (!d || !d->json || i < 0 || i >= size())
		return ScJsonValue();

	return ScJsonValue();
}

ScJsonValue ScJsonObject::value(const ScString& key) const
{
	if (!d || !d->json)
		return ScJsonValue(ScJsonValue::Invalid);
	return d->json->toValue(key);
}

ScJsonValueRef ScJsonObject::operator[](const ScString& key)
{
	return ScJsonValueRef(this, key);
}

ScJsonObject& ScJsonObject::operator=(const ScJsonObject& other)
{
	d = other.d;
	if (nullptr != d)
		d->ref.ref();
	return *this;
}