#ifndef DATETIMEMANAGER_H
#define DATETIMEMANAGER_H

#include <QObject>
#include <QDateTimeEdit>

/*
    The DATETIMEMANAGER class is used to control the date and time of the GUI for recording purposes, major functionalities include:
     - When GUI starts, the dateTime will be auto set to current system time
     - When user manually overwrite to a custom time, will keep track of custom time until main GUI is closed
     - Able to increment time every minute, so will be almost up to date with the real time, even if it's in a custom time will still increment every minute (worst case off by 59 seconds)
*/

class DateTimeManager : public QObject
{
    Q_OBJECT
public:
    explicit DateTimeManager(QObject *parent = nullptr);

    void setDateTimeEdit(QDateTimeEdit *dateTimeEdit);
    void setGUICurrentTime();
    void setSelfCurrentTime();
    void enableDisable(bool editing);
    QDateTime getCurrentTime();
    bool getEditingEnabled();

signals:

public slots:
    void updateTime();

private:
    QDateTimeEdit *dateTimeEdit;
    QDateTime currentTime;
    bool editingEnabled;
};

#endif // DATETIMEMANAGER_H
