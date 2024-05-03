#include "battery.h"
#include <QRandomGenerator>

Battery::Battery(QObject *parent) : QObject(parent) {
    batteryLevel = QRandomGenerator::global()->bounded(80, 101); // Random battery level between 80% to 100%
    isActive = false; // Device starts in idle mode
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Battery::decreaseBattery);

}

//decrease battery by 1% every tick
void Battery::decreaseBattery() {
    batteryLevel--;
    emit batteryLevelChanged(batteryLevel);
    if (batteryLevel == 0) {
        emit turnOffDevice();
        timer->stop();
    }else if (batteryLevel == 5) {
        emit criticallyLowBattery();
    } else if (batteryLevel == 15) {
        emit lowBattery();
    }

}

void Battery::startSimulation() {
    timer->start(isActive ? ACTIVE_BATTERY_LOSS_TICKER : IDLE_BATTERY_LOSS_TICKER); // Start timer with appropriate interval based on mode
    emit batteryLevelChanged(batteryLevel);
}

void Battery::setActiveMode(bool active) {
    isActive = active;
    // Restart timer with appropriate interval based on mode
    timer->setInterval(isActive ? ACTIVE_BATTERY_LOSS_TICKER : IDLE_BATTERY_LOSS_TICKER);
}

bool Battery::getActiveMode() {
    return isActive;
}

int Battery::getBatteryLevel() {
    return batteryLevel;
}
