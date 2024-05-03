#ifndef WAVE_H
#define WAVE_H

#include <QObject>

/* The wave class represents the sin wave that makes up a part of the total EEG waveform. In combination with other waves,
   the complete EEG waveform can be produced and displayed. A wave is made up of three parts: frequency, amplitude, and valueList.
   Frequency and amplitude are fairly straightforward. valueList, is a vector of double values that stores an number of values
   depending on the sample rate chosen in the electrode constructor. These values are used to display the graph of any given wave,
   usually the combinedWave of an electrode.*/

class Wave : public QObject
{
    Q_OBJECT
public:
    explicit Wave(QObject *parent = nullptr);

    double getFrequency();
    double getAmplitude();
    std::vector<double>& getValueList();

    void setFrequency(double freq);
    void setAmplitude(double amp);
    void setValueList(std::vector<double> list);

private:
    double frequency;
    double amplitude;
    std::vector<double> valueList;
};

#endif // WAVE_H
