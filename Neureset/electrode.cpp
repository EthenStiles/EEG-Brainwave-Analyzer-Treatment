#include "electrode.h"
#include <QDebug>
#include <cmath>
#include <QRandomGenerator>

double randomFreq;
double randomAmp;

Electrode::Electrode(QPushButton *contactButton, QObject *parent)
    : QObject{parent}, contactButton(contactButton)
{
    connected = false;
    wave1 = new Wave;
    wave2 = new Wave;
    wave3 = new Wave;
    combinedWave = new Wave;

    //generate 3 random waves at random frequency and amplitude
    //it is important that all 3 waves are given the same sample rate (default 960 samples per minute)
    randomFreq = QRandomGenerator::global()->bounded(1, 31); //pick a number between 1 and 30 for the frequency (most common brainwave range)
    randomAmp = QRandomGenerator::global()->generateDouble(); //pick a number between 0 and 1 for the amplitude
    wave1->setFrequency(randomFreq);
    wave1->setAmplitude(randomAmp);
    wave1->setValueList(generateWaveform(randomFreq, randomAmp, 1, 960)); //assuming the device can only sample 16 times per second, we take 960 samples per minute

    //we repeat the same steps for the remaining two waves (ideally picking a different random number for frequency and amplitude)
    randomFreq = QRandomGenerator::global()->bounded(1, 31);
    randomAmp = QRandomGenerator::global()->generateDouble();
    wave2->setFrequency(randomFreq);
    wave2->setAmplitude(randomAmp);
    wave2->setValueList(generateWaveform(randomFreq, randomAmp, 1, 960));

    randomFreq = QRandomGenerator::global()->bounded(1, 31);
    randomAmp = QRandomGenerator::global()->generateDouble();
    wave3->setFrequency(randomFreq);
    wave3->setAmplitude(randomAmp);
    wave3->setValueList(generateWaveform(randomFreq, randomAmp, 1, 960));

    //combine the three waves into one to simplify a complicated brainwave.
    combinedWave->setAmplitude(wave1->getAmplitude() + wave2->getAmplitude() + wave3->getAmplitude());
    combinedWave->setFrequency(wave1->getFrequency() + wave2->getFrequency() + wave3->getFrequency());
    combinedWave->setValueList(generateCombinedWave(wave1, wave2, wave3));

    //connect the button's click to FlipConnection
    connect(contactButton, &QPushButton::clicked, this, &Electrode::flipConnection);
}

Electrode::~Electrode() // ensures that the dynamically allocated waves are handled properly
{
    delete wave1;
    delete wave2;
    delete wave3;
    delete combinedWave;
}

//getters and setters
int Electrode::getId(){ return id; }
double Electrode::getDominantFrequency(){ return dominantFrequency; }
void Electrode::setDominantFrequency(double freq){ dominantFrequency = freq; }
Wave* Electrode::getWave1(){ return wave1; }
Wave* Electrode::getWave2(){ return wave2; }
Wave* Electrode::getWave3(){ return wave3; }
Wave* Electrode::getCombinedWave(){ return combinedWave; }

//this generates a vector<double> that represents the values of a valueList in a Wave; given frequency, amplitude, duration, and sample rate
std::vector<double> Electrode::generateWaveform(double frequency, double amplitude, int duration, int sampleRate)
{
    std::vector<double> waveform;
    waveform.reserve(duration * sampleRate);
    double offset = 15.5;
    double ampMult = 14.5;

    for (int i = 0; i < duration * sampleRate; ++i)
    {
        double t = i / static_cast<double>(sampleRate);
        double value = (ampMult * amplitude) * std::sin(2 * M_PI * frequency * t) + offset;
        waveform.push_back(value);
        //qInfo() << value;
    }

    return waveform;
}

//calculate the dominant frequency of the combined wave given the 3 waves that it consists of
double Electrode::calculateDominantFrequency(Wave* wave1, Wave* wave2, Wave* wave3) // this is a simplified formula for dominant frequency given by the prof, only works if our brainwave consist of the 3 sin waves
{
    double sin1_freq = wave1->getFrequency();
    double sin1_amp_squared = std::pow(wave1->getAmplitude(), 2);

    double sin2_freq = wave2->getFrequency();
    double sin2_amp_squared = std::pow(wave2->getAmplitude(), 2);

    double sin3_freq = wave3->getFrequency();
    double sin3_amp_squared = std::pow(wave3->getAmplitude(), 2);

    //this is the simplified dominant frequency formula given by the professor
    double dominantFrequency = ((sin1_freq * sin1_amp_squared) + (sin2_freq * sin2_amp_squared) + (sin3_freq * sin3_amp_squared)) / (sin1_amp_squared + sin2_amp_squared + sin3_amp_squared);

    setDominantFrequency(dominantFrequency);
    return dominantFrequency;
}

//apply a wave of specified frequency to the electrode and make the change correspondingly
void Electrode::applyFreq(int freq)
{
    double baselineFreq = getDominantFrequency();

    if (baselineFreq == 0)
    {
        qInfo() << "Baseline frequency is 0, dominant frequency has not been calculated.";
        return;
    }

    for (int i = 0; i < getCombinedWave()->getValueList().size(); ++i)
    {
        getCombinedWave()->getValueList()[i] += freq;
    }
    setDominantFrequency(baselineFreq + freq);
    return;
}

//generate the entire wave object from the 3 waves
std::vector<double> Electrode::generateCombinedWave(Wave* wave1, Wave* wave2, Wave* wave3)
{
    std::vector<double> waveform;
    int samples = wave1->getValueList().size();

    waveform.reserve(samples);

    for (int i = 0; i < samples; ++i)
    {
        waveform.push_back((wave1->getValueList()[i] + wave2->getValueList()[i] + wave3->getValueList()[i]) / 3);
    }

    return waveform;
}

//used mainly for the physical GUI button, switches the connection of the electrode between on and off
void Electrode::flipConnection()
{
    connected = !connected;
    if (connected) {
        contactButton->setText("Disconnect");
        contactButton->setStyleSheet("color: rgb(0, 0, 0);\nbackground-color: rgb(51, 209, 122);");
    }else {
        contactButton->setText("Connect");
        contactButton->setStyleSheet("color: rgb(255, 255, 255);\nbackground-color: rgb(224, 27, 36);");
    }
    emit electrodeChanged();
}

//getter for electrode connection
bool Electrode::getConnected()
{
    return connected;
}

//store a snapshot of the current combined wave given a round
void Electrode::takeCombinedWaveSnapShot(int round)
{
    combinedWaveSnapShot[round] = getCombinedWave()->getValueList();
}

//similar to last function, but returns the snapshot instead of saving it class variable of electrode class
std::vector<double> Electrode::getCombinedWaveSnapShot(int round)
{
    return combinedWaveSnapShot[round];
}
