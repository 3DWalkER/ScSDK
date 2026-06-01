#include "scutils/serialization/scjson_p.h"

ScJsonErrorParser::ScJsonErrorParser(const char* json, int length, int offset)
	: head(json), json(json), end(json + length)
	, length(length), offset(offset)
{
}

enum class ScJsonToken : scint8
{
	Space = 0x20,
	Tab = 0x09,
	LineFeed = 0x0a,
	Return = 0x0d,
	BeginArray = 0x5b,
	BeginObject = 0x7b,
	EndArray = 0x5d,
	EndObject = 0x7d,
	NameSeparator = 0x3a,
	ValueSeparator = 0x2c,
	Quote = 0x22
};

SC_DECL_CONSTEXPR char operator+(ScJsonToken token) {
	return static_cast<char>(token);
}

void ScJsonErrorParser::eatBOM()
{
	scuint8 utf8Bom[3] = { 0xef, 0xbb, 0xbf };
	if (end - json > 3 &&
		scuint8(json[0] == utf8Bom[0]) &&
		scuint8(json[1] == utf8Bom[1]) &&
		scuint8(json[2] == utf8Bom[2]))
		json += 3;
}

bool ScJsonErrorParser::eatSpace()
{
	while (json < end)
	{
		if (*json > +ScJsonToken::Space)
			break;

		if (isTokenVisible(*json))
			break;
		++json;
	}
	return json < end;
}

bool ScJsonErrorParser::eatEndSpace()
{
	while (json < end)
	{
		if (*(end - 1) > +ScJsonToken::Space)
			break;

		if (isTokenVisible(*(end - 1)))
			break;
		--end;
	}
	return json < end;
}

char ScJsonErrorParser::nextToken()
{
	if (!eatSpace())
		return 0;

	char token = *json++;
	switch (token)
	{
	case +ScJsonToken::BeginArray:
	case +ScJsonToken::BeginObject:
	case +ScJsonToken::NameSeparator:
	case +ScJsonToken::ValueSeparator:
	case +ScJsonToken::EndArray:
	case +ScJsonToken::EndObject:
	case +ScJsonToken::Quote:
		break;
	default:
		token = 0;
		break;
	}
	return token;
}

bool ScJsonErrorParser::isTokenVisible(char ch)
{
	if (ch != +ScJsonToken::Space &&
		ch != +ScJsonToken::Tab &&
		ch != +ScJsonToken::LineFeed &&
		ch != +ScJsonToken::Return)
		return true;
	return false;
}

ScJsonParseError::ParseError ScJsonErrorParser::parseArrayError()
{
	if (offset < 0 && *(end - 1) != +ScJsonToken::EndArray)
	{
		offset = (end - 1) - head;
		return ScJsonParseError::GarbageAtEnd;
	}

	if (length == offset)
		return ScJsonParseError::UnterminatedArray;
	return ScJsonParseError::NoError;
}

ScJsonParseError::ParseError ScJsonErrorParser::parseObjectError()
{
	if (offset < 0 && *(end - 1) != +ScJsonToken::EndObject)
	{
		offset = (end - 1) - head;
		return ScJsonParseError::GarbageAtEnd;
	}

	if (length == offset)
		return ScJsonParseError::UnterminatedObject;
	return ScJsonParseError::NoError;
}

void ScJsonErrorParser::parse(ScJsonParseError& error)
{
	eatBOM();
	eatEndSpace();
	char token = nextToken();
	if (token == +ScJsonToken::BeginArray)
		error.error = parseArrayError();
	else if (token == +ScJsonToken::BeginObject)
		error.error = parseObjectError();
	else
	{
		error.error = ScJsonParseError::IllegalValue;
		offset = json - head;
	}
	error.offset = offset;
}

ScJson::ScJson(cJSON* json, bool isDeletable)
	: cJson(json)
	, isDeletable(isDeletable)
{
}

ScJson::~ScJson()
{
	if (isDeletable && cJson)
		cJSON_Delete(cJson);
}

ScJsonData::ScJsonData()
{
	ref.ref();
}

ScJsonData::ScJsonData(std::shared_ptr<ScJson> json)
	: json(json)
{
	ref.ref();
}

ScJsonData::ScJsonData(ScJsonValue::Type type)
	: json(std::make_shared<ScJson>(type))
{
	ref.ref();
}

ScJsonData::ScJsonData(const ScJsonData& o)
	: json(o.json)
{
	ref.ref();
}

void ScJson::addOrReplaceObjectItem(const ScString& name, const ScJsonValue& val)
{
	if (!cJson)
		return;

	cJSON* pNewJson = toValue(val);
	if (!pNewJson)
		return;

	if (cJSON_ReplaceItemInObject(cJson, name.data(), pNewJson))
		return;

	if (cJSON_AddItemToObject(cJson, name.data(), pNewJson))
	{
		if (-1 != size)
			size += 1;
	}
}

void ScJson::replaceArrayItem(int index, const ScJsonValue& val)
{
	if (!cJson)
		return;

	cJSON* pNewJson = toValue(val);
	if (pNewJson)
		cJSON_ReplaceItemInArray(cJson, index, pNewJson);
}

bool ScJson::removeArrayItem(int index)
{
	cJSON *pJson = cJSON_DetachItemFromArray(cJson, index);
	if (!pJson)
		return false;

	cJSON_Delete(pJson);
	return true;
}

ScString ScJson::objectKeyAt(int i) const
{
	if (!cJson)
		return ScString();

	cJSON* pChild = cJson->child;
	while (i--)
		pChild = pChild->next;
	return pChild->string;
}

ScJsonValue ScJson::objectValueAt(int i) const
{
	if (!cJson)
		return ScJsonValue();

	cJSON* pChild = cJson->child;
	while (i--)
		pChild = pChild->next;
	return toValue(pChild);;
}

ScString ScJson::toJson(ScJsonDocument::JsonFormat format)
{
	if (!cJson)
		return ScString();

	char* pContent = nullptr;
	if (ScJsonDocument::Compact == format)
		pContent = cJSON_PrintUnformatted(cJson);
	else
		pContent = cJSON_Print(cJson);

	ScString result;
	if (pContent)
	{
		result = pContent;
		cJSON_free(pContent);
	}
	return result;
}

ScJsonArray ScJson::toArray(cJSON* cJson)
{
	ScJsonArray array;
	if (!cJson || !isArray(cJson))
		return array;

	cJSON* child = cJson->child;
	if (!child)
		return array;

	auto pJson = std::make_shared<ScJson>(child, false);
	array.d = new ScJsonData(pJson);
	return array;
}

ScJsonObject ScJson::toObject(cJSON* cJson)
{
	ScJsonObject object;
	if (!cJson || !isObject(cJson))
		return object;

	cJSON* child = cJson->child;
	if (!child)
		return object;

	auto pJson = std::make_shared<ScJson>(child, false);
	object.d = new ScJsonData(pJson);
	return object;
}

ScJsonValue ScJson::toValue(cJSON* cJson)
{
	ScJsonValue value(ScJsonValue::Invalid);
	if (!cJson)
		return value;

	int type = cJson->type & 0xFF;
	switch (type)
	{
	case cJSON_False:
		return ScJsonValue(false);
	case cJSON_True:
		return ScJsonValue(true);
	case cJSON_Number:
		return ScJsonValue(cJson->valuedouble);
	case cJSON_String:
		return ScJsonValue(cJson->valuestring);
	case cJSON_Array:
		return ScJsonValue(toArray(cJson));
	case cJSON_Object:
		return ScJsonValue(toObject(cJson));
	default:
		SC_UNREACHABLE();
		break;
	}
	return value;
}

cJSON* ScJson::toValue(const ScJsonValue& value)
{
	auto type = value.type();
	switch (value.type())
	{
	case ScJsonValue::Bool:
		return cJSON_CreateBool(value.toBool());
	case ScJsonValue::Number:
		return cJSON_CreateNumber(value.toDouble());
	case ScJsonValue::String:
		return cJSON_CreateString(value.toString().data());
	case ScJsonValue::Array:
		return cJSON_CreateArray();
	case ScJsonValue::Object:
		return cJSON_CreateObject();
	default:
		SC_UNREACHABLE();
		return nullptr;
	}
}

std::shared_ptr<ScJson> ScJson::clone()
{
	auto pJson = std::make_shared<ScJson>();
	if (pJson)
	{
		pJson->size = size;
		pJson->isDeletable = true;
		pJson->cJson = cJSON_Duplicate(cJson, true);
	}
	return pJson;
}

std::shared_ptr<ScJson> ScJson::parse(const ScString& json, ScJsonParseError* pError)
{
	std::shared_ptr<ScJson> pJson = nullptr;
	ScJsonParseError error{ 0, ScJsonParseError::NoError };
	do {
		if (json.isEmpty())
		{
			error.offset = 0;
			error.error = ScJsonParseError::NoError;
			break;
		}

		int offset = -1;
		const char* pJsonData = json.data();
		cJSON* pCJson = cJSON_Parse(pJsonData);
		if (!pCJson)
		{
			if (pError)
			{
				const char* pErr = cJSON_GetErrorPtr();
				offset = static_cast<int>(pErr - pJsonData);
			}
		}

		ScJsonErrorParser errorParser(pJsonData, json.length(), offset);
		errorParser.parse(error);
		if (ScJsonParseError::NoError != error.error)
			break;

		pJson = std::make_shared<ScJson>(pCJson, true);
	} while (0);

	if (pError)
		*pError = error;
	return pJson;
}

void ScJson::createJson(ScJsonValue::Type type)
{
	size = -1;
	isDeletable = true;
	switch (type)
	{
	case ScJsonValue::Object:
		cJson = cJSON_CreateObject();
		break;
	case ScJsonValue::Array:
		cJson = cJSON_CreateArray();
		break;
	default:
		cJson = nullptr;
		size = 0;
		isDeletable = false;
		break;
	}
}

bool ScJsonData::detach(ScJsonData*& d, ScJsonValue::Type type)
{
	if (!d)
	{
		d = new ScJsonData(type);
		return true;
	}

	if (1 == d->ref.loadRelaxed())
		return true;

	auto pJson = d->json->clone();
	if (!pJson)
		return false;

	d->ref.deref();
	d = new ScJsonData(pJson);
	return true;
}
