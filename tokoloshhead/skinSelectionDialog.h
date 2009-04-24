#ifndef SKIN_SELECTION_DIALOG
#define SKIN_SELECTION_DIALOG

#include <QDialog>

class SkinSelectionDialog : public QDialog
{
    public:
        static SkinSelectionDialog* instance();
        ~SkinSelectionDialog();
    private:
        SkinSelectionDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
        class Private;
        Private *d;
};

#endif
