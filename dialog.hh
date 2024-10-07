#ifndef DIALOG_HH
#define DIALOG_HH

#include <QDialog>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QProcess>

// Maximum time to wait for wsl command to respond in ms
const unsigned int WSL_TIMEOUT_KILL = 10000;
const unsigned int WSL_TIMEOUT_SHUTDOWN = 10000;
const unsigned int WSL_TIMEOUT_LIST = 500;
const QString APP_NAME = "wsl-monitor";

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();


private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason);

private:
    QIcon appIcon;
    QSystemTrayIcon* trayIcon;
    QMenu* trayMenu;
    QMenu* prefMenu;
    QList<QAction*> trayActions;
    QAction* quitAction;
    QAction* prefAction;
    QAction* noDistrosAction;
    QAction* hangingAction;
    QAction* startupAction;
    QAction* shutdownAction;

    void killDistro(QString distro);
    bool updateMenu();
    void setRunAtStartup(bool setting);
    void shutdownWsl();

};
#endif // DIALOG_HH
