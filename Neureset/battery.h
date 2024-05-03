#ifndef BATTERY_H
#define BATTERY_H

#include <QObject>
#include <QTimer>

/*
    The BATTERY class is used to simulate and control the battery progress bar on the main GPU, major functionalities include:
     - When GUI starts, battery is auto charged up to a random percentage between 80 ~ 100 %
     - Loses battery over time, when in session loss is 1% every 2 seconds, when not in session loss is 1% every 4 seconds
     - At 15% battery, will pop up low battery warning
     - At 5% battery, will pop up critically low battery warning
     - At 0% battery, will pop up device shutdown notice, then close main GUI
*/

class Battery : public QObject {
    Q_OBJECT
private:
    int const ACTIVE_BATTERY_LOSS_TICKER = 2000; //when active, battery loss is 1% / 2s
    int const IDLE_BATTERY_LOSS_TICKER = 4000; //when idle, battery loss is 1% / 4s
    int batteryLevel; // Battery level in percentage
    bool isActive; // Flag indicating whether the device is active or idle (active defined as in session)
    QTimer* timer; // Timer to track battery decrease time


public:
    explicit Battery(QObject *parent = nullptr);
    void decreaseBattery();
    void startSimulation();
    void setActiveMode(bool active);
    bool getActiveMode();
    int  getBatteryLevel();

signals:
    void lowBattery();
    void criticallyLowBattery();
    void turnOffDevice();
    void batteryLevelChanged(int level); // Signal emitted when battery level changes
};

#endif // BATTERY_H
