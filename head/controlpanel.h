#ifndef CONTROL_PANEL_DIALOG
#define CONTROL_PANEL_DIALOG

class Player;

#include <QDialog>

class ControlPanel : public QWidget
{
    Q_OBJECT
public:
    static ControlPanel *instance(Player *player);
    ~ControlPanel();
signals:
    void skinSelected(const QString &selectedSkinPath);
private:
    ControlPanel(QWidget *parent = 0, Qt::WindowFlags f = 0);
    class Private;
    Private *d;
};

#endif
