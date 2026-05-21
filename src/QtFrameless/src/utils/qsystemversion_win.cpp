#include "frameless/utils/qsystemversion_p.h"

#include "frameless/utils/qsysapiloader_p.h"

#include <array>
#include <QString>
#include <Windows.h>

struct QWinVersionNumber
{
	DWORD major{ };
	DWORD minor{ };
	DWORD micro{ };

	friend bool operator==(const QWinVersionNumber& lhs, const QWinVersionNumber& rhs) noexcept;
	friend bool operator!=(const QWinVersionNumber& lhs, const QWinVersionNumber& rhs) noexcept;
	friend bool operator>(const QWinVersionNumber& lhs, const QWinVersionNumber& rhs) noexcept;
	friend bool operator>=(const QWinVersionNumber& lhs, const QWinVersionNumber& rhs) noexcept;
};

typedef const std::array<QWinVersionNumber, static_cast<int>(QSystemVersion::WV_Latest) + 1> QWindowsVersions;
QWindowsVersions WindowsVersions =
{
	QWinVersionNumber{  5, 0,  2195 },  // Windows 2000
	QWinVersionNumber{  5, 1,  2600 },  // Windows XP
	QWinVersionNumber{  5, 2,  3790 },  // Windows XP x64 Edition or Windows Server 2003
	QWinVersionNumber{  6, 0,  6000 },  // Windows Vista
	QWinVersionNumber{  6, 0,  6001 },  // Windows Vista with Service Pack 1 or Windows Server 2008
	QWinVersionNumber{  6, 0,  6002 },  // Windows Vista with Service Pack 2
	QWinVersionNumber{  6, 1,  7600 },  // Windows 7 or Windows Server 2008 R2
	QWinVersionNumber{  6, 1,  7601 },  // Windows 7 with Service Pack 1 or Windows Server 2008 R2 with Service Pack 1
	QWinVersionNumber{  6, 2,  9200 },  // Windows 8 or Windows Server 2012
	QWinVersionNumber{  6, 3,  9200 },  // Windows 8.1 or Windows Server 2012 R2
	QWinVersionNumber{  6, 3,  9600 },  // Windows 8.1 with Update 1
	QWinVersionNumber{ 10, 0, 10240 },  // Windows 10 Version 1507 (TH1)
	QWinVersionNumber{ 10, 0, 10586 },  // Windows 10 Version 1511 (November Update) (TH2)
	QWinVersionNumber{ 10, 0, 14393 },  // Windows 10 Version 1607 (Anniversary Update) (RS1) or Windows Server 2016
	QWinVersionNumber{ 10, 0, 15063 },  // Windows 10 Version 1703 (Creators Update) (RS2)
	QWinVersionNumber{ 10, 0, 16299 },  // Windows 10 Version 1709 (Fall Creators Update) (RS3)
	QWinVersionNumber{ 10, 0, 17134 },  // Windows 10 Version 1803 (April 2018 Update) (RS4)
	QWinVersionNumber{ 10, 0, 17763 },  // Windows 10 Version 1809 (October 2018 Update) (RS5) or Windows Server 2019
	QWinVersionNumber{ 10, 0, 18362 },  // Windows 10 Version 1903 (May 2019 Update) (19H1)
	QWinVersionNumber{ 10, 0, 18363 },  // Windows 10 Version 1909 (November 2019 Update) (19H2)
	QWinVersionNumber{ 10, 0, 19041 },  // Windows 10 Version 2004 (May 2020 Update) (20H1)
	QWinVersionNumber{ 10, 0, 19042 },  // Windows 10 Version 20H2 (October 2020 Update) (20H2)
	QWinVersionNumber{ 10, 0, 19043 },  // Windows 10 Version 21H1 (May 2021 Update) (21H1)
	QWinVersionNumber{ 10, 0, 19044 },  // Windows 10 Version 21H2 (November 2021 Update) (21H2)
	QWinVersionNumber{ 10, 0, 19045 },  // Windows 10 Version 22H2 (October 2022 Update) (22H2)
	QWinVersionNumber{ 10, 0, 22000 },  // Windows 11 Version 21H2 (21H2)
	QWinVersionNumber{ 10, 0, 22621 }   // Windows 11 Version 22H2 (October 2022 Update) (22H2)
};

class QWinVersionHelper
{
public:
	explicit QWinVersionHelper();
	~QWinVersionHelper() = default;

	bool check(const QSystemVersion::WinVersion version) const;

private:
	void initialize();
	static bool isWindowsVersionOrGreater(QSystemVersion::WinVersion version);

	std::array<bool, static_cast<int>(QSystemVersion::WV_Latest) + 1> m_flags = {};
};
Q_GLOBAL_STATIC(QWinVersionHelper, g_winVerHelper)


QWinVersionHelper::QWinVersionHelper()
{
	initialize();
}

bool QWinVersionHelper::check(const QSystemVersion::WinVersion version) const
{
	return m_flags.at(static_cast<int>(version));
}

void QWinVersionHelper::initialize()
{
	const auto fill = [this](const int no) -> void {
		static const auto size = int(std::size(m_flags));
		if ((no <= 0) || (no >= size)) {
			return;
		}
		for (int i = 0; i != size; ++i) {
			m_flags.at(i) = (i <= no);
		}
		};

#define ELIF(Version) \
    else if (isWindowsVersionOrGreater(QSystemVersion::WV_##Version)) { \
    fill(static_cast<int>(QSystemVersion::WV_##Version)); \
}

	if (false) { /* Dummy */ }
	ELIF(11_22H2)
	ELIF(11_21H2)
	ELIF(10_22H2)
	ELIF(10_21H2)
	ELIF(10_21H1)
	ELIF(10_20H2)
	ELIF(10_2004)
	ELIF(10_1909)
	ELIF(10_1903)
	ELIF(10_1809)
	ELIF(10_1803)
	ELIF(10_1709)
	ELIF(10_1703)
	ELIF(10_1607)
	ELIF(10_1511)
	ELIF(10_1507)
	ELIF(8_1_Update1)
	ELIF(8_1)
	ELIF(8)
	ELIF(7_SP1)
	ELIF(7)
	ELIF(Vista_SP2)
	ELIF(Vista_SP1)
	ELIF(Vista)
	ELIF(XP_64)
	ELIF(XP)
	ELIF(2000)
	else { /* Dummy */ }

#undef ELIF
}

bool QWinVersionHelper::isWindowsVersionOrGreater(QSystemVersion::WinVersion version)
{
	static const auto currOsVer = []() -> QWinVersionNumber {
		if (!QSysApiLoader::isAvailable("ntdll", "RtlGetVersion"))
			return { };

		using RtlGetVersionPtr = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
		const auto pRtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(QSysApiLoader::get("ntdll", "RtlGetVersion"));

		RTL_OSVERSIONINFOEXW osvi;
		SecureZeroMemory(&osvi, sizeof(osvi));
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		if (pRtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&osvi)) == ERROR_SUCCESS)
			return QWinVersionNumber{ osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber };
		return { };
		}();

	QWinVersionNumber targetOsVer = WindowsVersions.at(version);
	if (currOsVer != QWinVersionNumber())
		return currOsVer >= targetOsVer;

	OSVERSIONINFOEXW osvi;
	SecureZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwMajorVersion = targetOsVer.major;
	osvi.dwMinorVersion = targetOsVer.minor;
	osvi.dwBuildNumber = targetOsVer.micro;
	DWORDLONG dwlConditionMask = 0;
	static constexpr const auto op = VER_GREATER_EQUAL;
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
	VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, op);
	return (::VerifyVersionInfoW(&osvi, (VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER), dwlConditionMask) != FALSE);
}

bool operator>(const QWinVersionNumber& lhs, const QWinVersionNumber& rhs) noexcept
{
	if (lhs == rhs)
		return false;

	if (lhs.major > rhs.major)
		return true;

	if (lhs.major < rhs.major)
		return false;

	if (lhs.minor > rhs.minor)
		return true;

	if (lhs.minor < rhs.minor)
		return false;

	if (lhs.micro < rhs.micro)
		return false;

	return lhs.micro > rhs.micro;
}

bool operator==(const QWinVersionNumber& lhs, const QWinVersionNumber& rhs) noexcept
{
	return (lhs.major == rhs.major) && (lhs.minor == rhs.minor) && (lhs.micro == rhs.micro);
}

bool operator>=(const QWinVersionNumber& lhs, const QWinVersionNumber& rhs) noexcept
{
	return operator==(lhs, rhs) || operator>(lhs, rhs);
}

bool operator!=(const QWinVersionNumber& lhs, const QWinVersionNumber& rhs) noexcept
{
	return !operator==(lhs, rhs);
}

namespace QSystemVersion
{
#define IMPL(Name, Version) \
    bool isWin##Name##OrGreater() \
    { \
        static const bool result = g_winVerHelper()->check(QSystemVersion::WV_##Version); \
        return result; \
    }

	IMPL(2K, 2000)
    IMPL(XP, XP)
    IMPL(XP64, XP_64)
    IMPL(Vista, Vista)
    IMPL(VistaSP1, Vista_SP1)
    IMPL(VistaSP2, Vista_SP2)
    IMPL(7, 7)
    IMPL(7SP1, 7_SP1)
    IMPL(8, 8)
    IMPL(8Point1, 8_1)
    IMPL(8Point1Update1, 8_1_Update1)
    IMPL(10, 10)
    IMPL(10TH1, 10_1507)
    IMPL(10TH2, 10_1511)
    IMPL(10RS1, 10_1607)
    IMPL(10RS2, 10_1703)
    IMPL(10RS3, 10_1709)
    IMPL(10RS4, 10_1803)
    IMPL(10RS5, 10_1809)
    IMPL(1019H1, 10_1903)
    IMPL(1019H2, 10_1909)
    IMPL(1020H1, 10_2004)
    IMPL(1020H2, 10_20H2)
    IMPL(1021H1, 10_21H1)
    IMPL(1021H2, 10_21H2)
    IMPL(1022H2, 10_22H2)
    IMPL(11, 11)
    IMPL(1121H2, 11_21H2)
    IMPL(1122H2, 11_22H2)

#undef IMPL
}
