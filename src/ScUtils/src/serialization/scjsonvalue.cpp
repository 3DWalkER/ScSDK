#include "scutils/serialization/scjson_p.h"

class ScJsonValuePrivate
{
	SC_DECLARE_PUBLIC(ScJsonValue)
public:
	ScJsonValuePrivate(ScJsonValue* q, ScJsonValue::Type);
	~ScJsonValuePrivate();

	ScJsonValue* q_ptr;
	ScJsonValue::Type type;

	union
	{
		bool b;
		double d;
		ScString s;
		ScJsonArray a;
		ScJsonObject o;
	};
};

ScJsonValuePrivate::ScJsonValuePrivate(ScJsonValue* q, ScJsonValue::Type type)
	: q_ptr(q)
	, type(type)
	, b(false)
{
}

ScJsonValuePrivate::~ScJsonValuePrivate()
{
	if (ScJsonValue::String == type)
		s.~ScString();
	else if (ScJsonValue::Object == type)
		o.~ScJsonObject();
	else if (ScJsonValue::Array == type)
		a.~ScJsonArray();
}

ScJsonValue::ScJsonValue(Type type)
	: d_ptr(new ScJsonValuePrivate(this, type))
{
}

ScJsonValue::ScJsonValue(bool value)
	: ScJsonValue(Bool)
{
	d_ptr->b = value;
}

ScJsonValue::ScJsonValue(int value)
	: ScJsonValue(Number)
{
	d_ptr->d = value;
}

ScJsonValue::ScJsonValue(scuint64 value)
	: ScJsonValue(Number)
{
	d_ptr->d = value;
}

ScJsonValue::ScJsonValue(scint64 value)
	: ScJsonValue(Number)
{
	d_ptr->d = value;
}

ScJsonValue::ScJsonValue(double value)
	: ScJsonValue(Number)
{
	d_ptr->d = value;
}

ScJsonValue::ScJsonValue(const ScString& value)
	: ScJsonValue(String)
{
	new (&d_ptr->s) ScString();
	d_ptr->s = value;
}

ScJsonValue::ScJsonValue(const char* value)
	: ScJsonValue(ScString(value))
{
}

ScJsonValue::ScJsonValue(const ScJsonObject& value)
	: ScJsonValue(Object)
{
	new (&d_ptr->o) ScJsonObject();
	d_ptr->o = value;
}

ScJsonValue::ScJsonValue(const ScJsonArray& value)
	: ScJsonValue(Array)
{
	new (&d_ptr->a) ScJsonArray();
	d_ptr->a = value;
}

ScJsonValue::ScJsonValue(const ScJsonValue& other)
	: d_ptr(new ScJsonValuePrivate(this, Null))
{
	SC_D(ScJsonValue);
	if (!other.d_ptr)
		return;

	d->type = other.type();
	switch (d->type)
	{
	case ScJsonValue::Bool:
		d->b = other.d_func()->b;
		break;
	case ScJsonValue::Number:
		d->d = other.d_func()->d;
		break;
	case ScJsonValue::Object:
		d->o = other.d_func()->o;
		break;
	case ScJsonValue::Array:
		d->a = other.d_func()->a;
		break;
	case ScJsonValue::String:
		d->s = other.d_func()->s;
		break;
	default:
		break;
	}
}

ScJsonValue::~ScJsonValue()
{
	SC_SAVE_DELETE(d_ptr);
}

ScJsonValue::Type ScJsonValue::type() const
{
	SC_D(const ScJsonValue);
	return !d ? Invalid : d->type;
}

bool ScJsonValue::toBool(bool defaultValue) const
{
	SC_D(const ScJsonValue);
	if (!d || Bool != d->type)
		return defaultValue;
	return d->b;
}

int ScJsonValue::toInt(int defaultValue) const
{
	SC_D(const ScJsonValue);
	if (!d || Number != d->type || static_cast<int>(d->d) != d->d)
		return defaultValue;
	return d->d;
}

double ScJsonValue::toDouble(double defaultValue) const
{
	SC_D(const ScJsonValue);
	if (!d || Number != d->type)
		return defaultValue;
	return d->d;
}

ScString ScJsonValue::toString(const ScString& defaultValue) const
{
	SC_D(const ScJsonValue);
	if (!d || String != d->type)
		return defaultValue;
	return d->s;
}

ScJsonArray ScJsonValue::toArray() const
{
	return toArray(ScJsonArray());
}

ScJsonArray ScJsonValue::toArray(const ScJsonArray& defaultValue) const
{
	SC_D(const ScJsonValue);
	if (!d || Array != d->type)
		return defaultValue;
	return d->a;
}

ScJsonObject ScJsonValue::toObject() const
{
	return toObject(ScJsonObject());
}

ScJsonObject ScJsonValue::toObject(const ScJsonObject& defaultValue) const
{
	SC_D(const ScJsonValue);
	if (!d || Object != d->type)
		return defaultValue;
	return d->o;
}

ScJsonValue& ScJsonValue::operator=(const ScJsonValue& other)
{
	ScJsonValue nascent(other);
	std::swap(d_ptr, nascent.d_ptr);
	return *this;
}

bool ScJsonValue::operator==(const ScJsonValue& other) const
{
	SC_D(const ScJsonValue);
	if (d->type != other.type())
		return false;

	const auto& od = other.d_func();
	switch (d->type)
	{
	case Bool:
		return d->b == od->b;
	case Number:
		return d->d == od->d;
	default:
		return true;
	}
}

class ScJsonValueRefPrivate
{
	SC_DECLARE_PUBLIC(ScJsonValueRef)
public:
	ScJsonValueRefPrivate(ScJsonValueRef* q, ScJsonArray* arr, int index);
	ScJsonValueRefPrivate(ScJsonValueRef* q, ScJsonObject* obj, const ScString& key);
	~ScJsonValueRefPrivate();

	ScJsonValue toValue() const;

	ScJsonValueRef* q_ptr;
	bool isObject;

	union {
		ScJsonArray* a;
		ScJsonObject* o;
	};

	union {
		int index;
		ScString key;
	};
};

ScJsonValueRefPrivate::ScJsonValueRefPrivate(ScJsonValueRef* q, ScJsonArray* arr, int index)
	: q_ptr(q)
	, isObject(false)
	, a(arr)
	, index(index)
{

}

ScJsonValueRefPrivate::ScJsonValueRefPrivate(ScJsonValueRef* q, ScJsonObject* obj, const ScString& key)
	: q_ptr(q)
	, isObject(true)
	, o(obj)
	, key()
{
	new (&this->key) ScString();
	this->key = key;
}

ScJsonValueRefPrivate::~ScJsonValueRefPrivate()
{
	if (isObject)
		key.~ScString();
}

ScJsonValue ScJsonValueRefPrivate::toValue() const
{
	if (isObject)
		return o ? o->value(key) : ScJsonValue(ScJsonValue::Invalid);
	return a ? a->at(index) : ScJsonValue(ScJsonValue::Invalid);
}

ScJsonValueRef::ScJsonValueRef(ScJsonArray* array, int index)
	: d_ptr(new ScJsonValueRefPrivate(this, array, index))
{
}

ScJsonValueRef::ScJsonValueRef(ScJsonObject* object, const ScString& key)
	: d_ptr(new ScJsonValueRefPrivate(this, object, key))
{
}

ScJsonValueRef::~ScJsonValueRef()
{
	delete d_ptr;
}

ScJsonValue::Type ScJsonValueRef::type() const
{
	SC_D(const ScJsonValueRef);
	return d->toValue().type();
}
bool ScJsonValueRef::toBool(bool defaultValue) const
{
	SC_D(const ScJsonValueRef);
	return d->toValue().toBool(defaultValue);
}

int ScJsonValueRef::toInt(int defaultValue) const
{
	SC_D(const ScJsonValueRef);
	return d->toValue().toInt(defaultValue);
}

double ScJsonValueRef::toDouble(double defaultValue) const
{
	SC_D(const ScJsonValueRef);
	return d->toValue().toDouble(defaultValue);
}

ScString ScJsonValueRef::toString(const ScString& defaultValue) const
{
	SC_D(const ScJsonValueRef);
	return d->toValue().toString(defaultValue);
}

ScJsonArray ScJsonValueRef::toArray() const
{
	SC_D(const ScJsonValueRef);
	return d->toValue().toArray();
}

ScJsonArray ScJsonValueRef::toArray(const ScJsonArray& defaultValue) const
{
	SC_D(const ScJsonValueRef);
	return d->toValue().toArray(defaultValue);
}

ScJsonObject ScJsonValueRef::toObject() const
{
	SC_D(const ScJsonValueRef);
	return d->toValue().toObject();
}

ScJsonObject ScJsonValueRef::toObject(const ScJsonObject& defaultValue) const
{
	SC_D(const ScJsonValueRef);
	return d->toValue().toObject(defaultValue);
}

ScJsonValueRef::operator ScJsonValue() const
{
	SC_D(const ScJsonValueRef);
	return d->toValue();
}

ScJsonValueRef& ScJsonValueRef::operator=(const ScJsonValue& val)
{
	SC_D(ScJsonValueRef);
	if (d->isObject)
	{
		if (d->o)
		{
			auto& od = d->o->d;
			if (ScJsonData::detachObject(od))
				od->json->addOrReplaceObjectItem(d->key, val);
		}
	}
	else
	{
		if (d->a)
			d->a->replace(d->index, val);
	}
	return *this;
}

bool ScJsonValueRef::operator==(const ScJsonValue& other) const
{
	SC_D(const ScJsonValueRef);
	return d->toValue() == other;
}
