#include "homepage.h"

#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <driver/drivermanager.h>
#include <form/controls/controlutil.h>
#include <form/home/homecontroller.h>
#include <fortmanager.h>
#include <fortsettings.h>
#include <manager/windowmanager.h>
#include <util/guiutil.h>
#include <util/iconcache.h>
#include <util/startuputil.h>

namespace {

QGroupBox *createGroupBox()
{
    auto c = new QGroupBox();
    c->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    c->setMinimumWidth(200);

    auto layout = new QVBoxLayout();
    layout->setSpacing(10);
    c->setLayout(layout);

    return c;
}

QVBoxLayout *groupBoxLayout(QGroupBox *c)
{
    return static_cast<QVBoxLayout *>(c->layout());
}

}

HomePage::HomePage(HomeController *ctrl, QWidget *parent) : HomeBasePage(ctrl, parent)
{
    setupUi();
}

void HomePage::onRetranslateUi()
{
    m_gbDriver->setTitle(tr("Driver"));
    m_gbService->setTitle(tr("Windows Service"));
    m_gbPortable->setTitle(tr("Portable"));

    retranslateDriverMessage();
    m_btInstallDriver->setText(tr("Reinstall"));
    m_btRemoveDriver->setText(tr("Remove"));

    retranslateServiceMessage();
    m_btInstallService->setText(tr("Install"));
    m_btInstallService->setToolTip(tr("Run Fort Firewall as a Service in background"));
    m_btRemoveService->setText(tr("Remove"));

    m_btUninstallPortable->setText(tr("Uninstall"));
}

void HomePage::retranslateDriverMessage()
{
    const auto text = driverManager()->isDeviceError()
            ? driverManager()->errorMessage()
            : (driverManager()->isDeviceOpened() ? tr("Driver Installed")
                                                 : tr("Driver Not Installed"));

    m_labelDriverMessage->setText(text);
}

void HomePage::retranslateServiceMessage()
{
    const auto text = hasService() ? tr("Service Installed") : tr("Service Not Installed");

    m_labelServiceMessage->setText(text);
}

void HomePage::setupUi()
{
    // Driver Group Box
    setupDriverBox();

    // Service Group Box
    setupServiceBox();

    // Portable Group Box
    setupPortableBox();

    updateIsUserAdmin();
    updateIsPortable();
    updateHasService();

    auto layout = new QVBoxLayout();
    layout->addWidget(m_gbDriver, 0, Qt::AlignHCenter);
    layout->addWidget(m_gbService, 0, Qt::AlignHCenter);
    layout->addWidget(m_gbPortable, 0, Qt::AlignHCenter);
    layout->addStretch();

    this->setLayout(layout);
}

void HomePage::setupDriverBox()
{
    // Label Row
    auto labelLayout = setupDriverLabelLayout();

    // Buttons Row
    auto buttonsLayout = setupDriverButtonsLayout();

    m_gbDriver = createGroupBox();

    auto layout = groupBoxLayout(m_gbDriver);
    layout->addLayout(labelLayout);
    layout->addLayout(buttonsLayout);
}

QLayout *HomePage::setupDriverLabelLayout()
{
    // Message
    m_labelDriverMessage = ControlUtil::createLabel();
    m_labelDriverMessage->setWordWrap(true);
    m_labelDriverMessage->setFont(GuiUtil::fontBold());

    // Icon
    setupDriverIcon();

    auto layout = new QHBoxLayout();
    layout->setSpacing(4);
    layout->addStretch();
    layout->addWidget(m_iconDriver, 0, Qt::AlignTop);
    layout->addWidget(m_labelDriverMessage);
    layout->addStretch();

    return layout;
}

void HomePage::setupDriverIcon()
{
    const QSize iconSize(16, 16);
    m_iconDriver = ControlUtil::createIconLabel(":/icons/server_components.png", iconSize);

    const auto refreshDriverInfo = [&] {
        m_iconDriver->setEnabled(driverManager()->isDeviceOpened());
    };

    refreshDriverInfo();

    connect(driverManager(), &DriverManager::isDeviceOpenedChanged, this, refreshDriverInfo);
    connect(driverManager(), &DriverManager::errorCodeChanged, this,
            &HomePage::retranslateDriverMessage);
}

QLayout *HomePage::setupDriverButtonsLayout()
{
    m_btInstallDriver = ControlUtil::createButton(QString(), [&] {
        windowManager()->showConfirmBox([&] { fortManager()->installDriver(); },
                tr("Are you sure to reinstall the Driver?"));
    });
    m_btRemoveDriver = ControlUtil::createButton(QString(), [&] {
        windowManager()->showConfirmBox(
                [&] { fortManager()->removeDriver(); }, tr("Are you sure to remove the Driver?"));
    });

    auto layout = new QHBoxLayout();
    layout->setSpacing(10);
    layout->addStretch();
    layout->addWidget(m_btInstallDriver);
    layout->addWidget(m_btRemoveDriver);
    layout->addStretch();

    return layout;
}

void HomePage::setupServiceBox()
{
    // Label Row
    auto labelLayout = setupServiceLabelLayout();

    // Buttons Row
    auto buttonsLayout = setupServiceButtonsLayout();

    m_gbService = createGroupBox();

    auto layout = groupBoxLayout(m_gbService);
    layout->addLayout(labelLayout);
    layout->addLayout(buttonsLayout);
}

QLayout *HomePage::setupServiceLabelLayout()
{
    // Message
    m_labelServiceMessage = ControlUtil::createLabel();
    m_labelServiceMessage->setWordWrap(true);
    m_labelServiceMessage->setFont(GuiUtil::fontBold());

    // Icon
    setupServiceIcon();

    auto layout = new QHBoxLayout();
    layout->setSpacing(4);
    layout->addStretch();
    layout->addWidget(m_iconService, 0, Qt::AlignTop);
    layout->addWidget(m_labelServiceMessage);
    layout->addStretch();

    return layout;
}

void HomePage::setupServiceIcon()
{
    const QSize iconSize(16, 16);
    m_iconService = ControlUtil::createIconLabel(":/icons/widgets.png", iconSize);
}

QLayout *HomePage::setupServiceButtonsLayout()
{
    auto layout = new QHBoxLayout();
    layout->setSpacing(10);

    m_btInstallService = ControlUtil::createButton(QString(), [&] {
        windowManager()->showConfirmBox(
                [&] { setServiceInstalled(true); }, tr("Are you sure to install the Service?"));
    });
    m_btRemoveService = ControlUtil::createButton(QString(), [&] {
        windowManager()->showConfirmBox(
                [&] { setServiceInstalled(false); }, tr("Are you sure to remove the Service?"));
    });

    layout->addStretch();
    layout->addWidget(m_btInstallService);
    layout->addWidget(m_btRemoveService);
    layout->addStretch();

    return layout;
}

void HomePage::setupPortableBox()
{
    // Buttons Row
    auto buttonsLayout = setupPortableButtonsLayout();

    m_gbPortable = createGroupBox();

    auto layout = groupBoxLayout(m_gbPortable);
    layout->addLayout(buttonsLayout);
}

QLayout *HomePage::setupPortableButtonsLayout()
{
    m_btUninstallPortable = ControlUtil::createButton(QString(), [&] {
        windowManager()->showConfirmBox(
                [&] {
                    FortManager::uninstall();
                    fortManager()->removeDriver();

                    updateHasService();
                },
                tr("Are you sure to uninstall the Fort Firewall?"));
    });

    auto layout = new QHBoxLayout();
    layout->setSpacing(10);
    layout->addStretch();
    layout->addWidget(m_btUninstallPortable);
    layout->addStretch();

    return layout;
}

void HomePage::updateIsUserAdmin()
{
    if (settings()->isUserAdmin())
        return;

    m_btInstallDriver->setEnabled(false);
    m_btRemoveDriver->setEnabled(false);

    m_btInstallService->setEnabled(false);
    m_btRemoveService->setEnabled(false);

    m_btUninstallPortable->setEnabled(false);
}

void HomePage::updateIsPortable()
{
    if (settings()->isPortable())
        return;

    m_gbPortable->setVisible(false);
}

void HomePage::updateHasService()
{
    m_hasService = StartupUtil::isServiceInstalled();

    m_btRemoveDriver->setVisible(!hasService());
    retranslateServiceMessage();
    m_iconService->setEnabled(hasService());
    m_btInstallService->setVisible(!hasService());
    m_btRemoveService->setVisible(hasService());
}

void HomePage::setServiceInstalled(bool install)
{
    StartupUtil::setServiceInstalled(install);

    updateHasService();

    windowManager()->processRestartRequired(tr("Windows Service installation changed."));
}
