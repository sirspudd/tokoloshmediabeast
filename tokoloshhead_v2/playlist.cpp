static QStringList parseM3U(const QString &path)
{
    QFile file(path);
    if (!file.exists()) {
        qWarning("%s doesn't seem to exist", qPrintable(path));
        return QStringList();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("I can't seem to open %s", qPrintable(path));
        return QStringList();
    }

    QStringList list;
    QTextStream ts(&file);
    do {
        QString line = ts.readLine();
        QFileInfo fi(line);
        if (fi.filePath() != fi.absoluteFilePath()) { // does this work right when the file doesn't exist?
            line.prepend(file.absolutePath());
            // also, what do I do about streaming addresses?
        }
        list.append(line);
    } while (!ts.atEnd());
    return list;
}
