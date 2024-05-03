#ifndef ELECTRODE_H
#define ELECTRODE_H

#include <QObject>
#include <QPushButton>
#include <wave.h>

/*
    The ELECTRODE class is used to both simulate and virtualize the physical electrode + site + sensor attached to the patient's head, the main GUI have 21 instances of the electrode class.
    The class has the following functionalities:
     - Each of the 21 electrodes have a physical GUI button on the main GUI that allows the user to simulate connecting and disconnecting the electrode at any time.
     - The electro contains a wave variable to help simulate the brainwave defetected by the electrode at various times, the wave is made complicated by adding 3 waves (wave 1, 2, 3) together
     - An array of size 5 of vector of doubles that stores a snapshot of the wave to help reconstruct wave graphs at the following stages:
        - before treatment
        - after first treatment
        - after 2nd treatment
        - after 3rd treatment
        - after 4th treatment
*/

class Electrode : public QObject
{
    Q_OBJECT
public:
    explicit Electrode(QPushButton *contactButton, QObject *parent = nullptr);
    ~Electrode();
    void applyFreq(int freq);

    int getId();
    bool getConnected();
    double getDominantFrequency();
    void setDominantFrequency(double freq);

    Wave* getWave1();
    Wave* getWave2();
    Wave* getWave3();
    Wave* getCombinedWave();

    std::vector<double> generateWaveform(double frequency, double amplitude, int duration, int sampleRate); //generates a "random" sin wave within the range of human brainwaves
    double calculateDominantFrequency(Wave* wave1, Wave* wave2, Wave* wave3); //calculates dominant frequency using given simplified dominant frequency formula
    std::vector<double> generateCombinedWave(Wave* wave1, Wave* wave2, Wave* wave3);
    void takeCombinedWaveSnapShot(int round);
    std::vector<double> getCombinedWaveSnapShot(int round);

signals:
    void electrodeChanged(); //emitted whenever an electrode is connected or disconnected

private:
    int id;
    int dominantFrequency;
    bool connected;
    Wave* wave1; //represents the 1st sin wave of the brainwaves
    Wave* wave2; //represents the 2nd sin wave of the brainwaves
    Wave* wave3; //represents the 3rd sin wave of the brainwaves
    Wave* combinedWave; //represents the entire brainwave
    QPushButton *contactButton; // Contact button associated with the electrode
    std::vector<double> combinedWaveSnapShot[5];

private slots:
    void flipConnection();
};

#endif // ELECTRODE_H
