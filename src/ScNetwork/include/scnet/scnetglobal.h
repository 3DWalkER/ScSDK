#ifndef SCNETGLOBAL_H
#define SCNETGLOBAL_H

namespace ScWebSocketProtocol
{
	enum Version
	{
		VersionUnknown = -1,
		Version0 = 0,
		Version4 = 4,
		Version5 = 5,
		Version6 = 6,
		Version7 = 7,
		Version8 = 8,
		Version13 = 13,
		VersionLatest = Version13
	};
} // End namespace ScWebSocketProtocol

#endif // SCNETGLOBAL_H