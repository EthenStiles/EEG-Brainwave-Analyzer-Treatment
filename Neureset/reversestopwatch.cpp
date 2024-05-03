#include "reversestopwatch.h"

reverseStopwatch::reverseStopwatch(QLabel* label, QProgressBar* progressBar, QObject *parent) : QObject(parent),
    label(label), progressBar(progressBar)
{
    // Initialize the timer
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &reverseStopwatch::updateTimer);

    // Initialize duration and remaining time
    duration = 0;
    remainingTime = 0;
}

reverseStopwatch::~reverseStopwatch(){}

void reverseStopwatch::setDuration(int seconds)
{
    duration = seconds;
    remainingTime = duration;
    updateProgressBar();
}

//starts the class timer
void reverseStopwatch::start()
{
    if (duration > 0) {
        timer->start(1000); // Update every second
    }
}

//stops the class timer the more approxiate term here is pause as the timer can be resumed
void reverseStopwatch::stop()
{
    timer->stop();
}

//resume the timer
void reverseStopwatch::resume()
{
    if (duration > 0 && remainingTime > 0) {
        timer->start(1000); // Resume updating every second
    }
}

//updates the text on the label associated with the timer, also emits the corresponding signal after x seconds have passed
void reverseStopwatch::updateTimer()
{
    if (remainingTime > 0) {
        remainingTime--;
        int minutes = remainingTime / 60;
        int seconds = remainingTime % 60;
        label->setText(QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0')));
        updateProgressBar();
        if (remainingTime == (duration - 5)) {
            emit seconds5Elapsed();
        }else if (remainingTime == (duration - 6)) {
            emit seconds6Elapsed();
        }else if (remainingTime == (duration - 11)) {
            emit seconds11Elapsed();
        }else if (remainingTime == (duration - 12)) {
            emit seconds12Elapsed();
        }else if (remainingTime == (duration - 17)) {
            emit seconds17Elapsed();
        }else if (remainingTime == (duration - 18)) {
            emit seconds18Elapsed();
        }else if (remainingTime == (duration - 23)) {
            emit seconds23Elapsed();
        }else if (remainingTime == (duration - 24)) {
            emit seconds24Elapsed();
        }
    } else {
        timer->stop();
        emit timeout();
    }
}

//up[date the progress bar associated with the timer
void reverseStopwatch::updateProgressBar()
{
    int progress = 0;
    if (duration > 0) {
        progress = ((duration - remainingTime) * 100) / duration;
    }
    progressBar->setValue(progress);
}
