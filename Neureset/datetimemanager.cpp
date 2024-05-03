#include "datetimemanager.h"
#include <QTimer>

DateTimeManager::DateTimeManager(QObject *parent) : QObject(parent), editingEnabled(false)
{
    QTimer *timer = new QTimer(this); //a QTimer is running the time incrementation behind the scene
    connect(timer, &QTimer::timeout, this, &DateTimeManager::updateTime);
    timer->start(60000); // Update time every minute
}

//this sets the GUI dateTimeEdit object as a class variable to allow easier access and modification later
void DateTimeManager::setDateTimeEdit(QDateTimeEdit *dateTimeEdit)
{
    this->dateTimeEdit = dateTimeEdit;
    setGUICurrentTime(); // Set the initial time
}

//set the current time of the instance of dateTimeManager, not the GUI
void DateTimeManager::setSelfCurrentTime()
{
    QDateTime userSetTime = dateTimeEdit->dateTime();
    currentTime = userSetTime;
}

//set the current time on the GUI aspect NOT the dateTimeManager
void DateTimeManager::setGUICurrentTime()
{
    currentTime = QDateTime::currentDateTime();
    dateTimeEdit->setDateTime(currentTime);
}

//enables or disables the GUI aspect being editable, only enabled when user needs to specify custom time
void DateTimeManager::enableDisable(bool editing)
{
    editingEnabled = editing;
    dateTimeEdit->setEnabled(editing);
}

//getter
QDateTime DateTimeManager::getCurrentTime()
{
    return currentTime;
}

//getter
bool DateTimeManager::getEditingEnabled()
{
    return editingEnabled;
}

//update time on the GUI aspect when needed
void DateTimeManager::updateTime()
{
    if (!editingEnabled) {
        currentTime = currentTime.addSecs(60); // Add one minute
        dateTimeEdit->setDateTime(currentTime);
    }
}
