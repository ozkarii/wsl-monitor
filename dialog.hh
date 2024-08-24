#ifndef DIALOG_HH
#define DIALOG_HH

#include <QDialog>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QProcess>

// Maximum time to wait for wsl command to respond in ms
const unsigned int WSL_TIMEOUT_KILL = 10000;
const unsigned int WSL_TIMEOUT_LIST = 500;
const QString APP_NAME = "wsl-monitor";

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();


private slots:
    void on_checkBox_stateChanged(int arg1);
    void iconActivated(QSystemTrayIcon::ActivationReason);

private:
    Ui::Dialog *ui;
    QIcon appIcon;
    QSystemTrayIcon* trayIcon;
    QMenu* trayMenu;
    QList<QAction*> trayActions;
    QAction* quitAction;
    QAction* noDistrosAction;
    QAction* hangingAction;

    void killDistro(QString distro);
    bool updateMenu();
    void setRunAtStartup(bool setting);


};
#endif // DIALOG_HH
