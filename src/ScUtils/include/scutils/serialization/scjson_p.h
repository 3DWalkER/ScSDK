#ifndef SCJSON_P_H
#define SCJSON_P_H

#include "deps/cjson/cJSON.h"
#include "scutils/thread/scatomic.h"
#include "scjsondocument.h"
#include "scjsonarray.h"
#include "scjsonobject.h"

#include <memory>

class ScJsonErrorParser
{
public:
	ScJsonErrorParser(const char* json, int length, int offset);

	void parse(ScJsonParseError& error);

private:
	void eatBOM();
	bool eatSpace();
	bool eatEndSpace();
	char nextToken();

	bool isTokenVisible(char ch);
	ScJsonParseError::ParseError parseArrayError();
	ScJsonParseError::ParseError parseObjectError();

	const char* head;
	const char* json;
	const char* end;

	int length;
	int offset;
};

struct ScJson
{
	ScJson() = default;
	ScJson(cJSON* json, bool isDeletable);
	inline ScJson(ScJsonValue::Type type) { createJson(type); }
	~ScJson();

	bool isEmpty() const;
	bool isValid() const;
	bool isArray() const { return isArray(cJson); }
	bool isObject() const { return isObject(cJson); }

	int arraySize() const { return cJSON_GetArraySize(cJson); }
	int objectSize() const { return arraySize(); }

	ScJsonArray toArray() const { return toArray(cJson); }
	ScJsonObject toObject() const { return toObject(cJson); }
	ScJsonValue toValue(int index) const { return toValue(cJSON_GetArrayItem(cJson, index)); }
	ScJsonValue toValue(const ScString& key) const { return toValue(cJSON_GetObjectItem(cJson, key.data())); }
	ScString toJson(ScJsonDocument::JsonFormat format);

	cJSON* objectItem(const ScString& key) { return cJSON_GetObjectItem(cJson, key.data()); }
	bool containsObjectItem(const ScString& key) { return objectItem(key); }
	bool removeObjectItem(const ScString& key);
	ScString objectKeyAt(int i) const;
	ScJsonValue objectValueAt(int i) const;
	void addOrReplaceObjectItem(const ScString& name, const ScJsonValue& val);

	void replaceArrayItem(int index, const ScJsonValue &val);
	bool insertArrayItem(int index, const ScJsonValue& value) { return cJSON_InsertItemInArray(cJson, index, toValue(value)); }
	bool removeArrayItem(int index);

	std::shared_ptr<ScJson> clone();
	static std::shared_ptr<ScJson> parse(const ScString& json, ScJsonParseError* pError);

	cJSON* cJson{ };
	bool isDeletable{ };
	int size{ -1 };

private:
	void createJson(ScJsonValue::Type type);

	static bool isArray(cJSON* cJson) { return cJson && cJSON_IsArray(cJson); }
	static ScJsonArray toArray(cJSON* cJson);

	static bool isObject(cJSON* json) { return json && cJSON_IsObject(json); }
	static ScJsonObject toObject(cJSON* cJson);

	static ScJsonValue toValue(cJSON* cJson);
	static cJSON* toValue(const ScJsonValue& value);
};

inline bool ScJson::isEmpty() const
{
	return !cJson || !cJson->child;
}

inline bool ScJson::isValid() const {
	return cJson && !cJSON_IsInvalid(cJson);
}

inline bool ScJson::removeObjectItem(const ScString& key)
{
	cJSON* pJson = cJSON_DetachItemFromObject(cJson, key.data());
	if (!pJson)
		return false;
	cJSON_Delete(pJson);
	return true;
}

class ScJsonData
{
public:
	ScJsonData();
	explicit ScJsonData(std::shared_ptr<ScJson> json);
	ScJsonData(ScJsonValue::Type type);
	ScJsonData(const ScJsonData& o);
	ScJsonData(ScJsonData&& o) noexcept;
	~ScJsonData() = default;

	static bool detach(ScJsonData *&d, ScJsonValue::Type type);
	static bool detachArray(ScJsonData*& d) { return detach(d, ScJsonValue::Array); }
	static bool detachObject(ScJsonData*& d) { return detach(d, ScJsonValue::Object); }

	ScAtomicInt ref;
	std::shared_ptr<ScJson> json;
};

inline ScJsonData::ScJsonData(ScJsonData&& o) noexcept
	: json(o.json)
{
	o.json = nullptr;
	ref.ref();
} 

#endif //SCJSON_P_H