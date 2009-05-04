#ifndef SKIN_SELECTION_DIALOG
#define SKIN_SELECTION_DIALOG

class Player;

#include <QDialog>

class SkinSelectionDialog : public QDialog
{
    Q_OBJECT
public:
    static SkinSelectionDialog *instance(Player *player);
    ~SkinSelectionDialog();
signals:
    void skinSelected(const QString &selectedSkinPath);
private:
    SkinSelectionDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
    class Private;
    Private *d;
};

#endif
