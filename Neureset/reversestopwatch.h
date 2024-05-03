#ifndef REVERSESTOPWATCH_H
#define REVERSESTOPWATCH_H

#include <QObject>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>

/*
    The REVERSESTOPWATCH class is a modified timer class in essence, but since QTimer is already a class, this class was given its unique name to avoid confusion, the class have the following functionalities:
     - The ability to start a countdown timer from a given time, in practical use this class is used as the timer for both the session and also the 5 min shutdown timer if electrodes are disconnected or session paused
     - This class also updates the current countdown timer on a label (as a class variable) by converting the remaining time into a ##:## format then store a QString for label to update
     - Together with the label this class also updates a progressbar to accurately reflect the progress from the timer
     - Multiple signals are set to emit at various times of the timer, this is only used for the session purpose and the signals are ignored for the 5 min shutdown timer
*/

class reverseStopwatch : public QObject
{
    Q_OBJECT
public:
    explicit reverseStopwatch(QLabel* label, QProgressBar* progressBar, QObject *parent = nullptr);
    ~reverseStopwatch();

    void setDuration(int seconds); // Set the duration of the timer in seconds
    void start(); // Start the timer
    void stop(); // Stop the timer
    void resume();
signals:
    void timeout();
    void seconds5Elapsed();
    void seconds6Elapsed();
    void seconds11Elapsed();
    void seconds12Elapsed();
    void seconds17Elapsed();
    void seconds18Elapsed();
    void seconds23Elapsed();
    void seconds24Elapsed();

private slots:
    void updateTimer(); // Slot to update the timer display

private:
    QLabel *label; // Label to display the timer value
    QProgressBar *progressBar; // Progress bar to show the progress
    QTimer *timer; // QTimer to update the timer value
    int duration; // Duration of the timer in seconds
    int remainingTime; // Remaining time of the timer in seconds

    void updateProgressBar(); // Update the progress bar
};

#endif // REVERSESTOPWATCH_H
