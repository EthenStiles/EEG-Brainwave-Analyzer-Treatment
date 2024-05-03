#include "graphwindow.h"
#include "ui_graphwindow.h"
#include <QGraphicsTextItem>

GraphWindow::GraphWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GraphWindow),
    scene(new QGraphicsScene(this))
{
    ui->setupUi(this);
    ui->graphicsView->setScene(scene);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
}

GraphWindow::~GraphWindow()
{
    delete ui;
}

//generate the graph and display it using the graphwindow.ui
void GraphWindow::displayGraph(std::vector<double> values, int min, int max)
{
    //clear the scene so that graphs do not accidentally overlap
    scene->clear();

    //use the min and max values to set the text of the labels on the y-axis
    ui->label_min->setText(QString::number(min - 1));
    ui->label_max->setText(QString::number(max + 1));

    int range = max - min;

    for (int i = 0; i < int(values.size() - 1); i++)
    {

        int graphHeight = ui->graphicsView->height();
        // reduce graphHeight variable slightly so the graph is less likely to have a vertical scroll bar
        graphHeight *= 0.9;

        //calculate how far the current value is from the min
        double value = values[i] - min;
        // determine where the value should be in terms of the y-axis of the graph
        double percentage = value / range;

        //x-axis doesnt change
        double x1 = i;
        //since origin of QGraphicsScene is at top left, we negate the value
        //multiply the graph height by the percentage to calculate where to place the y-value of the point
        double y1 = -(graphHeight * percentage);

        //repeat the previous steps for the next point
        value = values[i+1] - min;
        percentage = value / range;

        double x2 = (i + 1);
        double y2 = -(graphHeight * percentage);

        //create a line between the two previously generated points
        QLineF line(x1, y1, x2, y2);
        //add the line to the scene
        scene->addLine(line);
    }
}

