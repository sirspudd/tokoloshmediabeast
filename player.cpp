#include "skin.h"

SkinButton::SkinButton(QWidget *parent)
    : QAbstractButton(parent)
{
}

void SkinButton::setNormalPixmap(const QPixmap &pixmap, const QRect &source)
{
    setPixmap(Normal, pixmap, source);
}

void SkinButton::setDownPixmap(const QPixmap &pixmap, const QRect &source)
{
    setPixmap(Down, pixmap, source);
}

void SkinButton::setHoverPixmap(const QPixmap &pixmap, const QRect &source)
{
    setAttribute(Qt::WA_Hover, !pixmap.isNull());
    setPixmap(Hover, pixmap, source);
}

void SkinButton::paintEvent(QPaintEvent *)
{
    // ### use paintEvent->rect() ?
    int i = Normal;
    if (isDown()) {
        i = Down;
    } else if (underMouse() && testAttribute(Qt::WA_Hover)) {
        i = Hover;
    }
    if (!d.pixmaps[i].isNull()) {
        QPainter p(this);
        p.drawPixmap(rect(), d.pixmaps[i], d.sourceRects[i]);
    }
}

void SkinButton::setPixmap(int idx, const QPixmap &pixmap, const QRect &source)
{
    for (int i=0; i<3; ++i) {
        if (idx != i && !d.pixmaps[i].isNull() && d.sourceRects[i].size() != source.size()) {
            qWarning() << "Button pixmaps must all be the same size" << source << i << d.sourceRects[i];
        }
    }
    d.pixmaps[idx] = pixmap;
    d.sourceRects[idx] = source;
}

Player::Player(QWidget *parent)
    : QWidget(parent)
{
    d.tokolosh = 0;
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    struct {
        QString name;
        const char *member;
        QString tooltip;
        QRect rect;
    } buttonInfo[] = {
        // prev, pause, pause, stop, next
        { tr("Previous"), SLOT(previous()), tr("Previous"), QRect(16, 86, 22, 18) },
        { tr("Play"), SLOT(play()), tr("Play"), QRect(38, 86, 22, 18) },
        { tr("Pause"), SLOT(pause()), tr("Pause"), QRect(60, 86, 22, 18) },
        { tr("Stop"), SLOT(stop()), tr("Stop"), QRect(82, 86, 22, 18) },
        { tr("Next"), SLOT(next()), tr("Next"), QRect(82, 86, 22, 18) },
        { tr("Open"), SLOT(open()), tr("Open"), QRect(82, 86, 22, 18) },
        { tr("OpenSkin"), SLOT(openSkin()), tr("OpenSkin"), QRect(82, 86, 22, 18) },
        { tr("Shuffle"), SLOT(shuffle()), tr("Shuffle"), QRect(82, 86, 22, 18) },
        { tr("Repeat"), SLOT(repeat()), tr("Repeat"), QRect(82, 86, 22, 18) },
        { tr("Playlist"), SLOT(playlist()), tr("Playlist"), QRect(82, 86, 22, 18) },
        { QString(), 0, QString(), QRect() }
    };

    for (int i=0; buttonInfo[i].member; ++i) {
        Q_ASSERT(i < ButtonCount);
        d.buttons[i] = new SkinButton(this);
        d.buttons[i]->setGeometry(buttonInfo[i].rect);
        d.buttons[i]->setObjectName(buttonInfo[i].name);
        d.buttons[i]->setToolTip(buttonInfo[i].tooltip);
        // connect to tokoloshinterface stuff
    }
}

void Player::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::blue);
    p.drawPixmap(e->rect(), d.background, e->rect()); // background brush instead?
}

void Player::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        d.dragOffset = e->pos();
    }
}

void Player::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() == Qt::LeftButton) {
        move(e->globalPos() - d.dragOffset);
    }
}

void Player::setSkin(const QString &path)
{
    QStringList entryList = QDir(path).entryList();
}

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    Player w;
    w.show();
    return a.exec();
}
