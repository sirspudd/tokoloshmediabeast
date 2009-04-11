#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
class Settings
{
public:
    static template <class T>  T value(const QString &key)
    {
        // check QApplication args first (maybe even store it if it
        // finds -store there. Pretty cool huh?
        if (


    }

    static template <class T> void setValue(const QString &key, const T &t)
    {

    }
private:
    Settings() {}
};


#endif
