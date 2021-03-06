#include <QApplication>
#include <QMessageBox>
#include <QStyle>
#include <QStyleFactory>

#ifdef USE_VISUAL_LEAK_DETECTOR
#    include <vld.h>
#endif

#include "../version.h"
#include "control/controlmanager.h"
#include "control/controlworker.h"
#include "driver/drivermanager.h"
#include "fortcommon.h"
#include "fortmanager.h"
#include "fortsettings.h"
#include "util/app/appinfo.h"
#include "util/osutil.h"

#define FORT_ERROR_INSTANCE 1
#define FORT_ERROR_CONTROL  2

static void registerMetaTypes(void)
{
    qRegisterMetaType<AppInfo>("AppInfo");
}

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

    QApplication::setQuitOnLastWindowClosed(false);

    QApplication app(argc, argv);
    QApplication::setApplicationName(APP_NAME);
    QApplication::setApplicationVersion(APP_VERSION_STR);
    QApplication::setApplicationDisplayName(APP_NAME " v" APP_VERSION_STR);

    FortSettings fortSettings(QCoreApplication::arguments());

    // Unregister booted provider and exit
    if (fortSettings.hasProvBoot()) {
        FortCommon::provUnregister();
        return 0;
    }

#ifdef USE_CONTROL_COMMANDS
    ControlManager controlManager(QApplication::applicationName(), fortSettings.controlCommand());

    // Send control request to running instance
    if (controlManager.isClient()) {
        return controlManager.post(fortSettings.args()) ? 0 : FORT_ERROR_CONTROL;
    }
#endif

#ifdef APP_SINGLE_INSTANCE
    // Check running instance
    if (!OsUtil::createGlobalMutex(APP_NAME)) {
        QMessageBox::critical(nullptr, QString(), "Application is already running!");
        return FORT_ERROR_INSTANCE;
    }
#endif

    // Style & Palette
    const auto fusionStyle = QStyleFactory::create("Fusion");
    QApplication::setStyle(fusionStyle);
    QApplication::setPalette(fusionStyle->standardPalette());

    registerMetaTypes();

    FortManager fortManager(&fortSettings);
    fortManager.launch();

#ifdef USE_CONTROL_COMMANDS
    // Process control requests from clients
    if (!controlManager.listen(&fortManager)) {
        return FORT_ERROR_CONTROL;
    }
#endif

    return QApplication::exec();
}
