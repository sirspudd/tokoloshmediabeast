#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <QtGui>

class ShortcutDialog : public QDialog
{
    Q_OBJECT
public:
    ShortcutDialog(QWidget *parent = 0);
    void accept();
    void editItem(QTreeWidgetItem *item);
    void contextMenuEvent(QContextMenuEvent *e);

public slots:
    void onDoubleClicked(QTreeWidgetItem *item);
private:
    struct Private {
        QList<QObject*> removedShortcuts;
        QTreeWidget *treeWidget;
    } d;
};


#endif
