#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStringList>
#include <QDesktopServices>
#include <QUrl>
#include "battery.h"
#include "datetimemanager.h"
#include "electrode.h"
#include "reversestopwatch.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/*
    The MAINWINDOW class serve both as the control class and the class that directly or indrectly is the parent (grandparent) of all the GUI components. This class have all the major functionalities that make
    the code functional, please see each fucntions for details
*/

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();
        bool allElectrodesConnected(); //checks if all electrodes are connected, this is used to trigger if the auto pause if any electrodes disconnect and resume when that is fixed

        //red light and green light flashes according to the specification and blue light only turns on and off
        void flashRedLight();
        void turnOffRedLight();
        void flashGreenLight();
        void turnOffGreenLight();
        void turnOnBlueLight();
        void turnOffBlueLight();
        void flipSessionEnable();
        void populateGraphList(); //adds a list of graph names to the graph listview to allow user to pick the graph to generate

    signals:
        void sessionStart(); //signals the start of a session
        void endSessionEarly(); //signals that a session ended early, need to delete the logs related to the session and handle a few things differently
        void sessionPausedResumed(); //signals if the session is paused or resumed
        void sessionStartCheck(); //this signal is used to check the initial states of the electrodes at the beginning of a session so that code used for handling when electrodes disconnect can be reused here.

    private:
        bool inSession; //if the machine is in session (even if paused still counts)
        bool pause; //whether the machine is paused, specifically for the paused and play button
        bool recentSession; //whether there was a recent session --> the graph information should be kept and displayed
        double avgDominantFreqBefore; //average dominant frequency prior session
        double avgDominantFreqAfter; //average dominant frequency post session
        QString sessionProgress; //the text to display on the label that signifies the session progress
        Ui::MainWindow *ui; //the GUI side of the app
        Battery *battery; //device battery
        QFile *sessionLog; //session log file
        QTextStream *sessionLogOut; //text stream for writing to session log file
        DateTimeManager *dateTimeManager; //for managing the date / time of the device
        reverseStopwatch *sessionTimer; //count down timer for the session itself
        reverseStopwatch *shutdownTimer; //5 min count down timer for when session paused or when electrodes are not all connected
        Electrode *electrodes[21]; //21 electrodes associated with the physical device

    private slots:
        void updateBatteryLevel(int level);
        void batteryLow();
        void batteryCriticallyLow();
        void batteryNone();
        void powerOff();
        void confirmButtonPressed();
        void playButtonPressed();
        void pauseButtonPressed();
        void stopButtonPressed();
        void generateGraph();
        void electrodeChanged();
        void shutdownAfterIdle();
        void measurementRound1();
        void treatmentRound1();
        void measurementRound2();
        void treatmentRound2();
        void measurementRound3();
        void treatmentRound3();
        void measurementRound4();
        void treatmentRound4();
        void measurementRound5();
        void openTxtFile();
        void onSessionEnd();
        void onSessionEndEarly();
};
#endif // MAINWINDOW_H
