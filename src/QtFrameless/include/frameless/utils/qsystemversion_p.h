#ifndef QSYSTEMVERSION_H
#define QSYSTEMVERSION_H

namespace QSystemVersion
{
/**
 * @brief The Version enum 系统版本
 */
enum WinVersion
{
    WV_2000,
    WV_XP,
    WV_XP_64,
    WV_Vista,
    WV_Vista_SP1,
    WV_Vista_SP2,
    WV_7,
    WV_7_SP1,
    WV_8,
    WV_8_1,
    WV_8_1_Update1,
    WV_10_1507,
    WV_10_1511,
    WV_10_1607,
    WV_10_1703,
    WV_10_1709,
    WV_10_1803,
    WV_10_1809,
    WV_10_1903,
    WV_10_1909,
    WV_10_2004,
    WV_10_20H2,
    WV_10_21H1,
    WV_10_21H2,
    WV_10_22H2,
    WV_11_21H2,
    WV_11_22H2,

    WV_WS_03 = WV_XP_64, // Windows Server 2003
    WV_10 = WV_10_1507,
    WV_11 = WV_11_21H2,

    WV_Latest = WV_11_22H2
};

bool isWinVistaOrGreater();
bool isWin8Point1OrGreater();
bool isWin10OrGreater();
bool isWin11OrGreater();
bool isWin10RS2OrGreater();
bool isWin10RS5OrGreater();
bool isWin1019H1OrGreater();
}

#endif // QSYSTEMVERSION_H
