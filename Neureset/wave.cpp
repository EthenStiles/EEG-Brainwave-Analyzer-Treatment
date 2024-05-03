#include "wave.h"
#include <QDebug>

Wave::Wave(QObject *parent)
    : QObject{parent}
{
    frequency = 0.0;
    amplitude = 0.0;

}
//get the frequency of the wave
double Wave::getFrequency(){ return frequency; }
//get the amplitude of the wave
double Wave::getAmplitude(){ return amplitude; }
//get the valueList of the wave
std::vector<double>& Wave::getValueList(){ return valueList; }

//sets the wave's frequency to the given double value
void Wave::setFrequency(double freq){ frequency = freq; }
//sets the wave's amplitude to the given double value
void Wave::setAmplitude(double amp){ amplitude = amp; }
//sets the wave's valueList to the given vector<double> list
void Wave::setValueList(std::vector<double> list){ valueList = list; }
