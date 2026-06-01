#include "scutils/serialization/scjsondocument.h"

#include "scutils/serialization/scjson_p.h"

ScJsonDocument::ScJsonDocument()
	: d(nullptr)
{

}

ScJsonDocument::ScJsonDocument(const ScJsonArray& array)
	: d(nullptr)
{
	setArray(array);
}

ScJsonDocument::ScJsonDocument(const ScJsonObject& object)
	: d(nullptr)
{
	setObject(object);
}

ScJsonDocument::ScJsonDocument(const ScJsonDocument& other)
	: d(other.d)
{

}

ScJsonDocument::~ScJsonDocument()
{
	if (d && !d->ref.deref())
		delete d;
}

bool ScJsonDocument::isNull() const
{
	return !d || !d->json;
}

bool ScJsonDocument::isEmpty() const
{
	return isNull() || !d->json->isEmpty();
}

bool ScJsonDocument::isValid() const
{
	return !isNull() && d->json->isValid();
}

bool ScJsonDocument::isArray() const
{
	return !isNull() && d->json->isArray();
}

bool ScJsonDocument::isObject() const
{
	return !isNull() && d->json->isObject();
}

ScJsonArray ScJsonDocument::array() const
{
	if (!isNull())
		return d->json->toArray();
	return ScJsonArray();
}

ScJsonObject ScJsonDocument::object() const
{
	if (!isNull())
		return d->json->toObject();
	return ScJsonObject();
}

void ScJsonDocument::setArray(const ScJsonArray& array)
{
	if (d && !d->ref.deref())
		delete d;

	d = array.d;
	if (!d)
		d = new ScJsonData(ScJsonValue::Array);
	else
		d->ref.ref();
}

void ScJsonDocument::setObject(const ScJsonObject& object)
{
	if (d && !d->ref.deref())
		delete d;

	d = object.d;
	if (!d)
		d = new ScJsonData(ScJsonValue::Object);
	else
		d->ref.ref();
}

ScJsonDocument& ScJsonDocument::operator=(const ScJsonDocument& other)
{
	d = other.d;
	if (d)
		d->ref.ref();
	return *this;
}

ScString ScJsonDocument::toJson(JsonFormat format) const
{
	if (isNull())
		return ScString();
	return d->json->toJson(format);
}

ScJsonDocument ScJsonDocument::fromJson(const ScString& value, ScJsonParseError* error)
{
	ScJsonDocument document;
	auto pJson = ScJson::parse(value, error);
	if (pJson)
		document.d = new ScJsonData(pJson);
	return document;
}
