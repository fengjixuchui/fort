#pragma once

#include <QSignalSpy>

#include <googletest.h>

#include <conf/addressgroup.h>
#include <conf/appgroup.h>
#include <conf/firewallconf.h>
#include <fortcommon.h>
#include <util/conf/confappswalker.h>
#include <util/conf/confutil.h>
#include <util/envmanager.h>
#include <util/fileutil.h>
#include <util/net/netutil.h>

class ConfUtilTest : public Test
{
    // Test interface
protected:
    void SetUp();
    void TearDown();
};

void ConfUtilTest::SetUp() { }

void ConfUtilTest::TearDown() { }

TEST_F(ConfUtilTest, ConfWriteRead)
{
    EnvManager envManager;
    FirewallConf conf;

    AddressGroup *inetGroup = conf.inetAddressGroup();

    inetGroup->setIncludeAll(true);
    inetGroup->setExcludeAll(false);

    inetGroup->setIncludeText(QString());
    inetGroup->setExcludeText(NetUtil::localIpv4Networks().join('\n'));

    conf.setAppBlockAll(true);
    conf.setAppAllowAll(false);

    AppGroup *appGroup1 = new AppGroup();
    appGroup1->setName("Base");
    appGroup1->setEnabled(true);
    appGroup1->setPeriodEnabled(true);
    appGroup1->setPeriodFrom("00:00");
    appGroup1->setPeriodTo("12:00");
    appGroup1->setBlockText("System");
    appGroup1->setAllowText("C:\\Program Files\\Skype\\Phone\\Skype.exe\n"
                            "?:\\Utils\\Dev\\Git\\**\n"
                            "D:\\**\\Programs\\**\n");

    AppGroup *appGroup2 = new AppGroup();
    appGroup2->setName("Browser");
    appGroup2->setEnabled(false);
    appGroup2->setAllowText("C:\\Utils\\Firefox\\Bin\\firefox.exe");
    appGroup2->setLimitInEnabled(true);
    appGroup2->setSpeedLimitIn(1024);

    conf.addAppGroup(appGroup1);
    conf.addAppGroup(appGroup2);

    ConfUtil confUtil;

    QByteArray buf;
    const int confIoSize = confUtil.write(conf, nullptr, envManager, buf);
    ASSERT_NE(confIoSize, 0);

    // Check the buffer
    const char *data = buf.constData() + FortCommon::confIoConfOff();

    ASSERT_FALSE(FortCommon::confIpInRange(data, 0, true));
    ASSERT_FALSE(FortCommon::confIpInRange(data, NetUtil::textToIp4("9.255.255.255")));
    ASSERT_FALSE(FortCommon::confIpInRange(data, NetUtil::textToIp4("11.0.0.0")));
    ASSERT_TRUE(FortCommon::confIpInRange(data, NetUtil::textToIp4("10.0.0.0")));
    ASSERT_TRUE(FortCommon::confIpInRange(data, NetUtil::textToIp4("169.254.100.100")));
    ASSERT_TRUE(FortCommon::confIpInRange(data, NetUtil::textToIp4("192.168.255.255")));
    ASSERT_FALSE(FortCommon::confIpInRange(data, NetUtil::textToIp4("193.0.0.0")));
    ASSERT_TRUE(FortCommon::confIpInRange(data, NetUtil::textToIp4("239.255.255.250")));

    ASSERT_TRUE(FortCommon::confAppBlocked(data, FortCommon::confAppFind(data, "System")));
    ASSERT_FALSE(FortCommon::confAppBlocked(data,
            FortCommon::confAppFind(data,
                    FileUtil::pathToKernelPath("C:\\Program Files\\Skype\\Phone\\Skype.exe"))));
    ASSERT_FALSE(FortCommon::confAppBlocked(data,
            FortCommon::confAppFind(
                    data, FileUtil::pathToKernelPath("C:\\Utils\\Dev\\Git\\git.exe"))));
    ASSERT_FALSE(FortCommon::confAppBlocked(data,
            FortCommon::confAppFind(
                    data, FileUtil::pathToKernelPath("D:\\Utils\\Dev\\Git\\bin\\git.exe"))));
    ASSERT_FALSE(FortCommon::confAppBlocked(data,
            FortCommon::confAppFind(
                    data, FileUtil::pathToKernelPath("D:\\My\\Programs\\Test.exe"))));
    ASSERT_TRUE(FortCommon::confAppBlocked(data,
            FortCommon::confAppFind(
                    data, FileUtil::pathToKernelPath("C:\\Program Files\\Test.exe"))));

    ASSERT_EQ(FortCommon::confAppPeriodBits(data, 0, 0), 0x01);
    ASSERT_EQ(FortCommon::confAppPeriodBits(data, 12, 0), 0);

    const quint16 firefoxFlags = FortCommon::confAppFind(
            data, FileUtil::pathToKernelPath("C:\\Utils\\Firefox\\Bin\\firefox.exe"));
    ASSERT_TRUE(FortCommon::confAppBlocked(data, firefoxFlags));
    ASSERT_EQ(int(FortCommon::confAppGroupIndex(firefoxFlags)), 1);
}

TEST_F(ConfUtilTest, CheckPeriod)
{
    const quint8 h = 15, m = 35;

    ASSERT_TRUE(FortCommon::isTimeInPeriod(h, m, 0, 0, 24, 0));
    ASSERT_TRUE(FortCommon::isTimeInPeriod(h, m, 15, 0, 16, 0));
    ASSERT_TRUE(FortCommon::isTimeInPeriod(h, m, 15, 0, 10, 0));
    ASSERT_FALSE(FortCommon::isTimeInPeriod(h, m, 15, 0, 15, 0));
    ASSERT_FALSE(FortCommon::isTimeInPeriod(h, m, 0, 0, 15, 0));
    ASSERT_FALSE(FortCommon::isTimeInPeriod(h, m, 16, 0, 15, 0));
    ASSERT_FALSE(FortCommon::isTimeInPeriod(h, m, 24, 0, 0, 0));
    ASSERT_FALSE(FortCommon::isTimeInPeriod(h, m, 16, 0, 14, 0));
    ASSERT_FALSE(FortCommon::isTimeInPeriod(h, m, 16, 0, 24, 0));

    ASSERT_TRUE(FortCommon::isTimeInPeriod(h, m, 15, 35, 15, 37));
    ASSERT_TRUE(!FortCommon::isTimeInPeriod(h, m, 15, 35, 15, 36));
}

TEST_F(ConfUtilTest, CheckEnvManager)
{
    EnvManager envManager;

    envManager.setCachedEnvVar("a", "a");
    envManager.setCachedEnvVar("b", "b");
    envManager.setCachedEnvVar("c", "c");

    ASSERT_EQ(envManager.expandString("%%%a%%b%%c%-%c%%b%%a%%%"), "%abc-cba%");

    envManager.setCachedEnvVar("d", "%e%");
    envManager.setCachedEnvVar("e", "%f%");
    envManager.setCachedEnvVar("f", "%d%");

    ASSERT_EQ(envManager.expandString("%d%"), QString());

    envManager.setCachedEnvVar("d", "%e%");
    envManager.setCachedEnvVar("e", "%f%");
    envManager.setCachedEnvVar("f", "%a%");

    ASSERT_EQ(envManager.expandString("%d%"), "a");

    ASSERT_NE(envManager.expandString("%HOME%"), QString());
}
