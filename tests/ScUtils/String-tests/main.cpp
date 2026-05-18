#include "scutils/text/scstring.h"
#include "scutils/utils/scconfig.h"
#include "scutils/text/scstringlist.h"

#include <string>

int main()
{
	do {
		char linkType[13]{ }, net_Type[16]{ };

		ScString str(net_Type);
		ScStringList list;
		list.append(net_Type);
		auto size = list[0].size();
	} while (1);
}