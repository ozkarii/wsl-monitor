#include "dialog.hh"
#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QApplication>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    // Tray icon menu
    quitAction = new QAction("&Quit", this);
    prefAction = new QAction("&Preferences", this);
    noDistrosAction = new QAction("&No distros running!", this);
    hangingAction = new QAction("&WSL not responding, try again!", this);
    startupAction = new QAction("&Run on startup");
    shutdownAction = new QAction("&Shutdown WSL");


    startupAction->setCheckable(true);

    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(prefAction, &QAction::triggered, this, [this](){this->show();});
    connect(startupAction, &QAction::triggered, this,
            [this](){this->setRunAtStartup(this->startupAction->isChecked());});
    connect(shutdownAction, &QAction::triggered, this, &Widget::shutdownWsl);

    trayIcon = new QSystemTrayIcon(this);

    trayMenu = new QMenu(this);
    trayIcon->setContextMenu(trayMenu);

    prefMenu = new QMenu(this);
    prefMenu->addAction(startupAction);
    prefAction->setMenu(prefMenu);

    appIcon = QIcon(":/icons/Tux.png");
    trayIcon->setIcon(appIcon);
    trayIcon->show();

    connect(trayIcon, &QSystemTrayIcon::activated, this, &Widget::iconActivated);

    this->hide();
}

Widget::~Widget()
{
    delete quitAction;
    delete prefAction;
    delete noDistrosAction;
    delete hangingAction;
    delete startupAction;
}

void Widget::killDistro(QString distro)
{
    QProcess wslProcess;
    wslProcess.startCommand("wsl.exe --terminate " + distro);
    if (not wslProcess.waitForFinished(WSL_TIMEOUT_KILL)) {
        QString message = "WSL did not respond after "
                          + QString::number((WSL_TIMEOUT_KILL / 1000))
                          + " seconds.";

        trayIcon->showMessage("WSL Hanging", message, appIcon);
        return;
    }
    int wslExitCode = wslProcess.exitCode();
    if (wslExitCode == 0) {
        return;
    }

    // something went wrong
    QString wslOutput = QString::fromWCharArray(
        reinterpret_cast<const wchar_t*>(
            wslProcess.readAllStandardOutput().constData()));
    qDebug() << wslOutput;

}

bool Widget::updateMenu()
{
    trayMenu->clear();

    QProcess wslProcess;
    wslProcess.startCommand("wsl.exe --list --running");
    if (not wslProcess.waitForFinished(WSL_TIMEOUT_LIST)) {
        trayMenu->addAction(hangingAction);
        trayMenu->addAction(shutdownAction);
        trayMenu->addAction(prefAction);
        trayMenu->addAction(quitAction);
        return false;
    }
    else if (wslProcess.exitCode() < 0) {
        trayMenu->addAction(noDistrosAction);
        trayMenu->addAction(shutdownAction);
        trayMenu->addAction(prefAction);
        trayMenu->addAction(quitAction);
        return false;
    }


    QString wslOutput = QString::fromWCharArray(
        reinterpret_cast<const wchar_t*>(
            wslProcess.readAllStandardOutput().constData()));

    QStringList runningDistros = wslOutput.split("\r\n");
    runningDistros.removeFirst();
    runningDistros.removeLast();

    for (int i = 0; i < runningDistros.size(); ++i) {

        QString distroName = runningDistros.at(i);

        // Handle <distro> (Default) case
        int spaceIdx = runningDistros.at(i).indexOf(' ');
        if (spaceIdx != -1) {
            distroName.truncate(spaceIdx);
        }

        QAction* newAction = new QAction("&Kill " + distroName, trayMenu);

        connect(newAction, &QAction::triggered, this,
                [this, distroName](){killDistro(distroName);});
        trayMenu->addAction(newAction);
    }
    trayMenu->addAction(shutdownAction);
    trayMenu->addAction(prefAction);
    trayMenu->addAction(quitAction);

    return true;
}

void Widget::setRunAtStartup(bool setting)
{
    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        QSettings::NativeFormat
    );
    if (setting) {
        settings.setValue(
            APP_NAME, QDir::toNativeSeparators(QCoreApplication::applicationFilePath())
        );
    }
    else {
        settings.remove(APP_NAME);
    }
}

void Widget::shutdownWsl()
{
    QProcess wslProcess;
    wslProcess.startCommand("wsl.exe --shutdown");
    wslProcess.waitForFinished(WSL_TIMEOUT_SHUTDOWN);
    trayMenu->hide();
}


void Widget::iconActivated(QSystemTrayIcon::ActivationReason reason_)
{
    switch (reason_)
    {
        case QSystemTrayIcon::Context:
            updateMenu();
            break;

        default:
            break;
    }
}

