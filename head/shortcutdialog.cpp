/*
    Copyright (c) 2010 Anders Bakken
    Copyright (c) 2010 Donald Carr
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer. Redistributions in binary
    form must reproduce the above copyright notice, this list of conditions and
    the following disclaimer in the documentation and/or other materials
    provided with the distribution. Neither the name of any associated
    organizations nor the names of its contributors may be used to endorse or
    promote products derived from this software without specific prior written
    permission. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
    NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
    OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
    OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
    OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

#include "shortcutdialog.h"

#include "config.h"
enum {
    ObjectRole = Qt::UserRole + 1,
    DefaultShortcutRole = Qt::UserRole + 2
};

Q_DECLARE_METATYPE(QList<QShortcut*>);
ShortcutDialog::ShortcutDialog(QWidget *parent)
    : QDialog(parent)
{
    setSizeGripEnabled(true);
    setWindowTitle(tr("Shortcut Dialog"));
    Q_ASSERT(parent);
    QVBoxLayout *l = new QVBoxLayout(this);
    l->addWidget(d.treeWidget = new QTreeWidget(this));
    d.treeWidget->setHeaderLabels(QStringList() << tr("Action"));
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,
                                                 Qt::Horizontal, this);
    connect(box, SIGNAL(accepted()), this, SLOT(accept()));
    connect(box, SIGNAL(rejected()), this, SLOT(reject()));
    l->addWidget(box);

    foreach(QObject *o, qFindChildren<QObject*>(parent)) {
        const QVariant defaultShortcut = o->property("defaultShortcut");
        if (!defaultShortcut.isNull()) {
            Q_ASSERT(!o->objectName().isEmpty());
            QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << o->property("text").toString());
            item->setData(0, ObjectRole, qVariantFromValue<QObject*>(o));
            item->setData(0, DefaultShortcutRole, defaultShortcut);
            QList<QKeySequence> seqs;
            QKeySequence seq = qVariantValue<QKeySequence>(o->property("shortcut"));
            QList<QObject*> shortcuts;
            if (!seq.isEmpty()) {
                seqs.append(seq);
                shortcuts.append(0);
            }
            foreach(QShortcut *shortcut, qVariantValue<QList<QShortcut*> >(o->property("shortcuts"))) {
                seqs.append(shortcut->key());
                shortcuts.append(shortcut);
            }
//            if (!seqs.contains(defaultShortcut))  // do something here
            for (int i=0; i<seqs.size(); ++i) {
                QTreeWidgetItem *child = new QTreeWidgetItem(item);
                child->setText(0, seqs.at(i).toString());
                if (shortcuts.at(i)) {
                    child->setData(0, ObjectRole, qVariantFromValue<QObject*>(shortcuts.at(i)));
                }
            }
            d.treeWidget->addTopLevelItem(item);
        }
    }
    const QByteArray geometry = Config::value<QByteArray>("ShortcutDialogGeometry");
    if (!geometry.isEmpty())
        restoreGeometry(geometry);
    d.treeWidget->expandAll();
}

void ShortcutDialog::accept()
{
    Config::setValue<QByteArray>("ShortcutDialogGeometry", saveGeometry());
    QDialog::accept();
}

void ShortcutDialog::onDoubleClicked(QTreeWidgetItem *item)
{
    if (!item->parent()) {
        editItem(new QTreeWidgetItem(item));
    }
}

void ShortcutDialog::contextMenuEvent(QContextMenuEvent *e)
{
    QTreeWidgetItem *item = d.treeWidget->itemAt(e->pos() - d.treeWidget->viewport()->pos());
    if (!item)
        return;
    QMenu menu(this);
    QAction *addChild = 0;
    QAction *revert = 0;
    QAction *remove = 0;
    if (!item->parent()) {
        addChild = menu.addAction(tr("Add shortcut"));
        revert = menu.addAction(tr("Revert to default"));
    } else {
        remove = menu.addAction(tr("Remove shortcut"));
    }

    const QAction *ret = menu.exec(e->globalPos());
    if (ret == addChild) {
        editItem(new QTreeWidgetItem(item));
    } else if (ret == revert) {
        QObject *o = qVariantValue<QObject*>(item->data(0, ObjectRole));
        foreach(QShortcut *shortcut, qFindChildren<QShortcut*>(o)) {
            d.removedShortcuts.append(shortcut);
        }
        o->setProperty("shortcut", o->property("defaultShortcut"));
    } else if (ret == remove) {
        d.removedShortcuts.append(qVariantValue<QObject*>(item->data(0, ObjectRole)));
        delete item;
    }
}

void ShortcutDialog::editItem(QTreeWidgetItem *item)
{
    // grabKeyboard etc
}

