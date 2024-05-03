#ifndef GRAPHWINDOW_H
#define GRAPHWINDOW_H

#include <QWidget>
#include <QGraphicsScene>

/*
    The GRAPHWINDOW class is used for the sole purpose of plotting snapshots of the combined wave of each of the 21 electrodes at various time of the session.
    It uses to QGraphicsScene in combination with a Electrode's combinedWave's valueList to display the waveform.
*/

namespace Ui {
class GraphWindow;
}

class GraphWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GraphWindow(QWidget *parent = nullptr);
    ~GraphWindow();
    void displayGraph(std::vector<double> values, int min, int max);

private:
    Ui::GraphWindow *ui;
    QGraphicsScene *scene;
};

#endif // GRAPHWINDOW_H
