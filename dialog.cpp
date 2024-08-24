#include "dialog.hh"
#include "ui_dialog.h"
#include <QDebug>
#include <QSettings>
#include <QDir>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
    , trayIcon(new QSystemTrayIcon(this))
{
    ui->setupUi(this);
    this->setWindowTitle(APP_NAME);

    // Tray icon menu
    quitAction = new QAction("&Quit", this);
    noDistrosAction = new QAction("&No distros running!", this);
    hangingAction = new QAction("&WSL not responding, please wait!", this);

    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    trayMenu = new QMenu(this);
    trayActions = {quitAction, noDistrosAction};
    trayMenu->addAction(quitAction);

    this->trayIcon->setContextMenu(trayMenu);

    appIcon = QIcon(":/icons/Tux.png");
    this->trayIcon->setIcon(appIcon);
    this->setWindowIcon(appIcon);

    // Displaying the tray icon
    this->trayIcon->show();

    // Interaction
    connect(trayIcon, &QSystemTrayIcon::activated, this, &Dialog::iconActivated);

    this->hide();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::killDistro(QString distro)
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

bool Dialog::updateMenu()
{
    trayMenu->clear();

    QProcess wslProcess;
    wslProcess.startCommand("wsl.exe --list --running");
    if (not wslProcess.waitForFinished(WSL_TIMEOUT_LIST)) {
        trayMenu->addAction(hangingAction);
        trayMenu->addAction(quitAction);
        return false;
    }
    else if (wslProcess.exitCode() < 0) {
        trayMenu->addAction(noDistrosAction);
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

        connect(newAction, &QAction::triggered, this, [this, distroName](){killDistro(distroName);});
        trayMenu->addAction(newAction);
    }

    trayMenu->addAction(quitAction);

    return true;
}

void Dialog::setRunAtStartup(bool setting)
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (setting) {
        settings.setValue(APP_NAME, QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
        qDebug() << APP_NAME + QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    }
    else {
        settings.remove(APP_NAME);
    }
}


void Dialog::iconActivated(QSystemTrayIcon::ActivationReason reason_)
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


void Dialog::on_checkBox_stateChanged(int arg1)
{
    setRunAtStartup((bool)arg1);
}

