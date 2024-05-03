#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPixmap>
#include <QDebug>
#include <QMessageBox>
#include "graphwindow.h"
#include <cmath>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    recentSession = false;
    ui->btn_generate_graph->setDisabled(true);
    inSession = false;
    ui->label_graph_list->setText("");
    pause = false;
    sessionProgress = "";
    avgDominantFreqBefore = 0.0;
    avgDominantFreqAfter = 0.0;
    battery = new Battery(this);
    dateTimeManager = new DateTimeManager(this);
    dateTimeManager->setDateTimeEdit(ui->dateTimeEdit);
    dateTimeManager->enableDisable(false); // By default, disable editing

    //connect all electrodes to their respective buttons
    for (int i = 0; i < 21; i++) {
        QString buttonName = QString("btn_electrode_%1").arg(i + 1);
        QPushButton *button = findChild<QPushButton *>(buttonName); // Find the QPushButton by name
        if (button) {
            electrodes[i] = new Electrode(button, this); // Create Electrode instance with QPushButton instance
        }
        connect(electrodes[i], &Electrode::electrodeChanged, this, &MainWindow::electrodeChanged);
    }

    //declare a timer to control the timer and progress bar for the session
    sessionTimer = new reverseStopwatch(ui->timer_session, ui->progressBar_session, this);
    shutdownTimer = nullptr;

    //declare session file and session text stream to nullptr first
    sessionLog = nullptr;
    sessionLogOut = nullptr;;

    //all connections for the battery
    connect(battery, &Battery::batteryLevelChanged, this, &MainWindow::updateBatteryLevel);
    connect(battery, &Battery::lowBattery, this, &MainWindow::batteryLow);
    connect(battery, &Battery::criticallyLowBattery, this, &MainWindow::batteryCriticallyLow);
    connect(battery, &Battery::turnOffDevice, this, &MainWindow::batteryNone);

    //all connections related to items on the GUI
    connect(ui->btn_power, &QPushButton::clicked, this, &MainWindow::powerOff);
    connect(ui->button_select, &QPushButton::clicked, this, &MainWindow::confirmButtonPressed);
    connect(ui->btn_stop, &QPushButton::clicked, this, &MainWindow::stopButtonPressed);
    connect(ui->btn_play, &QPushButton::clicked, this, &MainWindow::playButtonPressed);
    connect(ui->btn_pause, &QPushButton::clicked, this, &MainWindow::pauseButtonPressed);
    connect(ui->btn_generate_graph, &QPushButton::clicked, this, &MainWindow::generateGraph);

    //all connections within this class itself
    connect(this, &MainWindow::sessionPausedResumed, this, &MainWindow::electrodeChanged);
    connect(this, &MainWindow::sessionStart, this, &MainWindow::measurementRound1);
    connect(this, &MainWindow::sessionStartCheck, this, &MainWindow::electrodeChanged);
    connect(this, &MainWindow::endSessionEarly, this, &MainWindow::onSessionEndEarly);

    //all countdown related connects
    connect(sessionTimer, &reverseStopwatch::seconds5Elapsed, this, &MainWindow::treatmentRound1);
    connect(sessionTimer, &reverseStopwatch::seconds6Elapsed, this, &MainWindow::measurementRound2);
    connect(sessionTimer, &reverseStopwatch::seconds11Elapsed, this, &MainWindow::treatmentRound2);
    connect(sessionTimer, &reverseStopwatch::seconds12Elapsed, this, &MainWindow::measurementRound3);
    connect(sessionTimer, &reverseStopwatch::seconds17Elapsed, this, &MainWindow::treatmentRound3);
    connect(sessionTimer, &reverseStopwatch::seconds18Elapsed, this, &MainWindow::measurementRound4);
    connect(sessionTimer, &reverseStopwatch::seconds23Elapsed, this, &MainWindow::treatmentRound4);
    connect(sessionTimer, &reverseStopwatch::seconds24Elapsed, this, &MainWindow::measurementRound5);
    connect(sessionTimer, &reverseStopwatch::timeout, this, &MainWindow::onSessionEnd);

    //start the battery, set countdown to 29 seconds for a new session
    battery->startSimulation();
    sessionTimer->setDuration(29);


}

MainWindow::~MainWindow()
{
    delete ui;
}

//update the battery level to be reflected on the GUI
void MainWindow::updateBatteryLevel(int level)
{
    ui->battery->setValue(level);
}

//pop up for warning low battery 15%
void MainWindow::batteryLow()
{
     QMessageBox::warning(this, "Battery Warning", "Battery is low (15%). Consider recharging the device.");
}

//pop up for warning extra low battery 5%
void MainWindow::batteryCriticallyLow()
{
    QMessageBox::warning(this, "Battery Warning", "Battery is critically low (5%). Please recharge the device.");
}

//pop up to let the user know that battery is none, app is clsoing
void MainWindow::batteryNone()
{
    QMessageBox::critical(this, "Battery Depleted", "No battery remaining. Turning off device.");
    //session end early
    QTimer::singleShot(1000, this, &MainWindow::close); //
}

//force closing the app
void MainWindow::powerOff()
{
    //if app is in session, then need to run through the early closing procedure
    if  (inSession) {
        emit endSessionEarly();
    }
    QApplication::quit();
}

//if the confirm button is pressed
void MainWindow::confirmButtonPressed()
{
    QListWidgetItem *selectedMenuItem = ui->list_options->currentItem();
    if (!selectedMenuItem) return; //Pass if no item is selected (should not happen on paper))

    QString selectedMenuItemText = selectedMenuItem->text();
    if (selectedMenuItemText == "NEW SESSION") { //if NEW SESSION is selected
        inSession = true; //app is now considered in session
        //generate the path and filename for session log file
        sessionLog = new QFile(QString(QDir::currentPath() + QString("/logs/") + QString(ui->dateTimeEdit->dateTime().toString("MM/dd/yy hh:mm:ss AP").remove(QRegExp("[/:]"))) + QString(".txt")));
        if(!QDir(QDir::currentPath() + "/logs").exists()) {
            QDir().mkdir(QDir::currentPath() + "/logs");
        }
        if (sessionLog->open(QIODevice::WriteOnly | QIODevice::Text)) {
            sessionLogOut = new QTextStream(sessionLog);
        }
        flipSessionEnable(); //session is now enabled (which means all non related GUI aspects are disabled
        ui->label_dialogue->setText("Starting new session...");
        sessionTimer->start(); //start the session timer
        ui->button_select->setEnabled(false);
        emit sessionStart(); //this should run as a chain of multiple slots to start the session successfullly
        return;
    }
    else if (selectedMenuItemText == "SESSION LOG") { //if SESSION LOG is selected
        if (ui->list_options->isEnabled() == false) { //if user is meant to exit the session log phase
            disconnect(ui->list_graphs, &QListWidget::itemDoubleClicked, this, &MainWindow::openTxtFile);
            //enable the options list once the user has finished setting the time
            ui->list_options->setEnabled(true);
            ui->list_options->show();
            ui->label_dialogue->setText("Please choose an option");
            ui->list_graphs->clear();
            ui->label_graph_list->setText("");
            if (recentSession)
            {
                ui->label_graph_list->setText("Graphs");
                populateGraphList();
                ui->btn_generate_graph->setDisabled(false);
            }
        }else { //if user is meant to enter the session log phase
            ui->list_graphs->clear();
            ui->label_dialogue->setText("Accessing session log...");
            ui->label_graph_list->setText("Session logs");
            ui->btn_generate_graph->setDisabled(true);

            //find all the .txt files in the session log folder
            QDir logsDir(QDir::currentPath() + "/logs");
            QStringList filters;
            filters << "*.txt";
            QStringList txtFiles = logsDir.entryList(filters);

            //create a line for each log file
            foreach(const QString& txtFile, txtFiles) {
                QFileInfo fileInfo(txtFile);
                QString fileName = fileInfo.baseName(); // Get the file name without extension

                // Format the time string
                QString formattedFileName = QString("Log for session at ") +
                                            fileName.mid(0, 2) + "/" + fileName.mid(2, 2) + "/" + QString("20") +
                                            fileName.mid(4, 2) + " - " + fileName.mid(7, 2) + ":" +
                                            fileName.mid(9, 2) + ":" + fileName.mid(11, 5);

                // Add the formatted file name as an option to the list widget
                ui->list_graphs->addItem(formattedFileName);
            }

            //if file is selected and then double clicked open file in default txt editor
            connect(ui->list_graphs, &QListWidget::itemDoubleClicked, this, &MainWindow::openTxtFile);
            ui->list_options->setEnabled(false);
            ui->list_options->hide();
        }
        return;
    }
    else if (selectedMenuItemText == "TIME AND DATE") { //if TIME AND DATE is selected
        //if time cannot be edited, enable editing, once finished editing, press again to disable editing
        dateTimeManager->enableDisable(!dateTimeManager->getEditingEnabled());
        //disable the options list while the user is setting the time
        ui->list_options->setEnabled(false);
        ui->list_options->hide();
        ui->label_dialogue->setText("Please set time and date...");
        //if the widget is now disable, which means time set is done, which means switch the current system time to match with the user input time
        if (dateTimeManager->getEditingEnabled() == false) {
            dateTimeManager->setSelfCurrentTime();
            //enable the options list once the user has finished setting the time
            ui->list_options->setEnabled(true);
            ui->list_options->show();
            ui->label_dialogue->setText("Please choose an option");
        }
    }

    else
    {
        return; //should not happen on paper
    }
}

//if play button is pressed
void MainWindow::playButtonPressed()
{
    pause = false;
    emit sessionPausedResumed();
}

//if pause button is pressed
void MainWindow::pauseButtonPressed()
{
    pause = true;
    emit sessionPausedResumed();
}

//if stop button is pressed
void MainWindow::stopButtonPressed()
{
    emit endSessionEarly();
}

//generate the graphs for the session
void MainWindow::generateGraph()
{
    //ensure that there has been a recent session and a graph has been selected
    if (!recentSession || ui->list_graphs->selectedItems().empty()) { return; }
    //get the current selected item in the graph list
    int index = ui->list_graphs->currentIndex().row();
    //since there are 5 rounds per electrode we can divide the index by 5 to get the electrode number
    int electrodeNum = std::floor(index / 5.0);
    //since there are 5 rounds per electrode we can divide the index by 5 and use the remainder to get the round number
    int roundNum = (index % 5);

    std::vector<double> values = electrodes[electrodeNum]->getCombinedWaveSnapShot(roundNum);
    if (values.empty()){ return; }

    //generate the min and max values of the valueList so that we can display them in the graph
    double minValue = *std::min_element(values.begin(), values.end());
    double maxValue = *std::max_element(values.begin(), values.end());

    if (values.empty()) return;

    GraphWindow *graphWindow = new GraphWindow(this);
    graphWindow->setWindowTitle("Graph Display");
    graphWindow->displayGraph(values, minValue, maxValue);
    graphWindow->setAttribute(Qt::WA_DeleteOnClose);
    graphWindow->show();
    return;
}

//checks if all electrodes are connected
bool MainWindow::allElectrodesConnected()
{
    for (int i = 0; i < 21; i++) {
        if (!(electrodes[i]->getConnected())) {
            return false;
        }
    }
    return true;
}

//slot to handle if any of the electrode is flipped either to on or off
void MainWindow::electrodeChanged()
{
    //only care if electrode is connected or disconnected with in session
    if(inSession) {
        //if all electrodes are connected AND session is NOT paused
        if (allElectrodesConnected() && !pause) {
            ui->label_session_progress->setText(sessionProgress);
            ui->label_sessionTimer->setText("Session Remain Time");
            sessionTimer->resume();
            turnOnBlueLight();
            turnOffRedLight();
            if (shutdownTimer){
                shutdownTimer->stop();
                delete shutdownTimer;
                shutdownTimer = nullptr;
            }
        }else{ //else so either at least one electrode is disconnected or session is paused
            sessionTimer->stop();
            if(!allElectrodesConnected()){
                if (ui->label_session_progress->text() != "Please reconnect electrodes in 5 min") {
                    sessionProgress = ui->label_session_progress->text();
                }
                ui->label_session_progress->setText("Please reconnect electrodes in 5 min");
                flashRedLight();

            }else{
                if (ui->label_session_progress->text() != "Please press play (resume) in 5 min") {
                    sessionProgress = ui->label_session_progress->text();
                }
                ui->label_session_progress->setText("Please press play (resume) in 5 min");
                turnOffRedLight();
            }

            ui->label_sessionTimer->setText("Time Before Shut Down");
            turnOffBlueLight();
            // Start a 5-minute shutdown timer if not already running
            if (!shutdownTimer) {
                shutdownTimer = new reverseStopwatch(ui->timer_session, ui->progressBar_session, this);
                shutdownTimer->setDuration(5 * 60); // 5 minutes
                connect(shutdownTimer, &reverseStopwatch::timeout, this, &MainWindow::shutdownAfterIdle);
                shutdownTimer->start();
            }
        }
    }
}

//shutdown the app after 5 min of idle
void MainWindow::shutdownAfterIdle()
{
    QMessageBox::critical(this, "Forced Shutdown", "5 min after electrode(s) disconnect / pausing, system shutting down...");
    //need to run through the early closing procedure
    emit endSessionEarly();
    QApplication::quit();
}

//flashes the red light --> ERROR
void MainWindow::flashRedLight()
{
    turnOffRedLight(); // Start by turning off red light first as other instances of red light timer might interfere with the flashing
    turnOffGreenLight(); //Turn off greenlight for the offchange that we manage to pause the code while green light is on;
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        static bool lightOn = false;
        if (lightOn) {
            ui->light_contactLost->setStyleSheet("background-color: rgb(50, 0, 0);");
        } else {
            ui->light_contactLost->setStyleSheet("background-color: rgb(255, 0, 0);");
        }
        lightOn = !lightOn;
    });
    timer->start(100); // Flash every 100 milliseconds
}

//turns off the red light
void MainWindow::turnOffRedLight()
{
    // Stop the flashing timer for red light
    for (const auto& obj : QObject::children()) {
        QTimer* timer = qobject_cast<QTimer*>(obj);
        if (timer && timer->isActive()) {
            timer->stop();
            delete timer;
        }
    }
    // Ensure the red light is turned off
    ui->light_contactLost->setStyleSheet("background-color: rgb(50, 0, 0);");
}

//flashes the green light --> delivering treatment
void MainWindow::flashGreenLight()
{
    turnOffGreenLight(); // Start by turning off green light first as other instances of green light timer might interfere with the flashing
    turnOffRedLight(); //Turn off red light for the offchange that we manage to pause the code while red light is on;
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        static bool lightOn = false;
        if (lightOn) {
            ui->light_treatmentSignal->setStyleSheet("background-color: rgb(0, 50, 0);");
        } else {
            ui->light_treatmentSignal->setStyleSheet("background-color: rgb(0, 255, 0);");
        }
        lightOn = !lightOn;
    });
    timer->start(50); // Flash every 50 milliseconds
}

//turn off green light
void MainWindow::turnOffGreenLight()
{
    // Stop the flashing timer for green light
    for (const auto& obj : QObject::children()) {
        QTimer* timer = qobject_cast<QTimer*>(obj);
        if (timer && timer->isActive()) {
            timer->stop();
            delete timer;
        }
    }
    // Ensure the green light is turned off
    ui->light_treatmentSignal->setStyleSheet("background-color: rgb(0, 50, 0);");
}

//turns on the blue light --> in session
void MainWindow::turnOnBlueLight()
{
    ui->light_contact->setStyleSheet("background-color: rgb(0, 0, 255);");
}

//turns off the blue light
void MainWindow::turnOffBlueLight()
{
    ui->light_contact->setStyleSheet("background-color: rgb(0, 0, 50);");
}

//flips the enable-bility of all the GUI elements when in / out of session
void MainWindow::flipSessionEnable()
{
    ui->label_sessionTimer->setEnabled(!ui->label_sessionTimer->isEnabled());
    ui->timer_session->setEnabled(!ui->timer_session->isEnabled());
    ui->progressBar_session->setEnabled(!ui->progressBar_session->isEnabled());
    ui->label_session_progress->setEnabled(!ui->label_session_progress->isEnabled());
    ui->btn_stop->setEnabled(!ui->btn_stop->isEnabled());
    ui->btn_pause->setEnabled(!ui->btn_pause->isEnabled());
    ui->btn_play->setEnabled(!ui->btn_play->isEnabled());
}

//initial measurement at the beginning of session
void MainWindow::measurementRound1()
{
    battery->setActiveMode(true);
    ui->list_graphs->clear();
    turnOffGreenLight();
    sessionProgress = QString("Measuring electrodes signals (1/5)");
    ui->label_session_progress->setText(sessionProgress);
    for (int i = 0; i < 21; i++) {
        avgDominantFreqBefore += electrodes[i]->calculateDominantFrequency(electrodes[i]->getWave1(), electrodes[i]->getWave2(), electrodes[i]->getWave3());
        electrodes[i]->takeCombinedWaveSnapShot(0);
        std::vector<double> combinedWave = electrodes[i]->getCombinedWaveSnapShot(0);
    }
    avgDominantFreqBefore /= 21;
    *sessionLogOut << "[" << QString(ui->dateTimeEdit->dateTime().toString("MM/dd/yy hh:mm:ss AP")) <<  "]: Avg. Dominant Frequency Before Session: " << QString::number(avgDominantFreqBefore) << "\n";
    emit sessionStartCheck();
}

//first treatment
void MainWindow::treatmentRound1()
{
    flashGreenLight();
    sessionProgress = QString("Administing treatment (1/5)");
    ui->label_session_progress->setText(sessionProgress);

    for (int i = 0; i < 21; i++) {
        int freq = 5;
        for (int j = 0; j < 16; j++) {
            electrodes[i]->applyFreq(freq);
            freq += 5;
        }
    }
}

//measure after first treatment
void MainWindow::measurementRound2()
{
    turnOffGreenLight();
    sessionProgress = QString("Measuring electrodes signals (2/4)");
    ui->label_session_progress->setText(sessionProgress);
    for (int i = 0; i < 21; i++) {
        electrodes[i]->takeCombinedWaveSnapShot(1);
    }
}

//second treatment
void MainWindow::treatmentRound2()
{
    flashGreenLight();
    sessionProgress = QString("Administing treatment (2/4)");
    ui->label_session_progress->setText(sessionProgress);

    for (int i = 0; i < 21; i++) {
        int freq = 5;
        for (int j = 0; j < 16; j++) {
            electrodes[i]->applyFreq(freq);
            freq += 5;
        }
    }
}

//measure after 2nd treatment
void MainWindow::measurementRound3()
{
    turnOffGreenLight();
    sessionProgress = QString("Measuring electrodes signals (3/5)");
    ui->label_session_progress->setText(sessionProgress);
    for (int i = 0; i < 21; i++) {
        electrodes[i]->takeCombinedWaveSnapShot(2);
    }
}

//3rd treatment
void MainWindow::treatmentRound3()
{
    flashGreenLight();
    sessionProgress = QString("Administing treatment (3/4)");
    ui->label_session_progress->setText(sessionProgress);

    for (int i = 0; i < 21; i++) {
        int freq = 5;
        for (int j = 0; j < 16; j++) {
            electrodes[i]->applyFreq(freq);
            freq += 5;
        }
    }
}

//meansure after 3rd treatment
void MainWindow::measurementRound4()
{
    turnOffGreenLight();
    sessionProgress = QString("Measuring electrodes signals (4/5)");
    ui->label_session_progress->setText(sessionProgress);

    for (int i = 0; i < 21; i++) {
        //electrodes[i]->calculateDominantFrequency(electrodes[i]->getWave1(), electrodes[i]->getWave2(), electrodes[i]->getWave3());
        electrodes[i]->takeCombinedWaveSnapShot(3);
    }
}

//fourth treatment
void MainWindow::treatmentRound4()
{
    flashGreenLight();
    sessionProgress = QString("Administing treatment (4/4)");
    ui->label_session_progress->setText(sessionProgress);
    for (int i = 0; i < 21; i++) {
        int freq = 5;
        for (int j = 0; j < 16; j++) {
            electrodes[i]->applyFreq(freq);
            freq += 5;
        }
    }
}

//final measurement before ending session
void MainWindow::measurementRound5()
{
    turnOffGreenLight();
    sessionProgress = QString("Measuring electrodes signals (5/5)");
    ui->label_session_progress->setText(sessionProgress);

    for (int i = 0; i < 21; i++) {
        avgDominantFreqAfter += electrodes[i]->getDominantFrequency();
        electrodes[i]->takeCombinedWaveSnapShot(4);
    }
    avgDominantFreqAfter /= 21;

    *sessionLogOut << "[" << QString(ui->dateTimeEdit->dateTime().toString("MM/dd/yy hh:mm:ss AP")) <<  "]: Avg. Dominant Frequency After Session: " << QString::number(avgDominantFreqAfter) << "\n";
    recentSession = true;
    populateGraphList();
    ui->label_graph_list->setText("Graphs");
    ui->btn_generate_graph->setDisabled(false);
    battery->setActiveMode(false);
}

//opens a .txt file in the default .txt file editor
void MainWindow::openTxtFile()
{
    QString fileNameRaw = ui->list_graphs->currentItem()->text();
    QString fileName = fileNameRaw.mid(19, 2) + fileNameRaw.mid(22, 2) + fileNameRaw.mid(27, 2) + " " + fileNameRaw.mid(32, 2) + fileNameRaw.mid(35, 2) + fileNameRaw.mid(38, 5) + ".txt";
    QString fullPath = QDir::currentPath() + "/logs/" + fileName;
    QDesktopServices::openUrl(QUrl::fromLocalFile(fullPath));
}

//when a session ends properly
void MainWindow::onSessionEnd()
{
    sessionTimer->stop();
    if (shutdownTimer){
        shutdownTimer->stop();
        delete shutdownTimer;
        shutdownTimer = nullptr;
    }
    turnOffRedLight();
    turnOffBlueLight();
    turnOffGreenLight();
    ui->timer_session->setText("00:00");
    flipSessionEnable();
    if (sessionLog) {
        sessionLog->close();
        delete sessionLog;
        sessionLog = nullptr;
    }
    sessionTimer->setDuration(29);
    ui->button_select->setEnabled(true);
    avgDominantFreqBefore = 0.0;
    avgDominantFreqAfter = 0.0;
}

//when a session ends early due to either user or system interruption
void MainWindow::onSessionEndEarly()
{
    sessionTimer->stop();
    if (shutdownTimer){
        shutdownTimer->stop();
        delete shutdownTimer;
        shutdownTimer = nullptr;
    }
    turnOffRedLight();
    turnOffBlueLight();
    turnOffGreenLight();
    ui->timer_session->setText("00:00");
    flipSessionEnable();
    if (sessionLog) {
        sessionLog->close();
        sessionLog->remove(); //delete the log file if its clsoed early
        delete sessionLog;
        sessionLog = nullptr;
    }
    sessionTimer->setDuration(29);
    ui->button_select->setEnabled(true);
    avgDominantFreqBefore = 0.0;
    avgDominantFreqAfter = 0.0;
}

//add graphs to the listview for graphs
void MainWindow::populateGraphList()
{
    ui->list_graphs->clear();
    for (int i = 1; i < 22; i++)
    {
        for (int j = 1; j < 6; j++)
        {
            ui->list_graphs->addItem("Electrode " + QString::number(i) + ", round " + QString::number(j));
        }
    }
}
