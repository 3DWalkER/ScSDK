#include "scutils/serialization/scjsondocument.h"
#include "scutils/serialization/scjsonarray.h"
#include "scutils/serialization/scjsonobject.h"

#include "scutils/text/scstring.h"


int main()
{
	do {
		ScJsonObject obj;
		//obj["Test"] = 2;
		ScJsonValue val = obj["Test"];
		auto type = val.type();
		auto value = val.toInt();
		int size = obj.size();

		ScJsonArray array;
		obj["carrs"] = array;
		ScJsonDocument doc(obj);

		const auto content = doc.toJson();

		obj.remove("Test");
		const auto result = doc.toJson();
	} while (1);
}