// Qt header files
#include <QGridLayout>
#include <QPushButton>

// Other header files
#include "MapPanControlWidget.h"

using Bach::MapPanControlWidget;

MapPanControlWidget::MapPanControlWidget(MapWidget* parent) : QWidget(parent) {
    // We set up our 4 directional buttons in a 3x3 grid where most cells are empty.
    auto layout = new QGridLayout(this);
    setLayout(layout);
    //layout->setContentsMargins(0,0,0,0);
    //layout->setSpacing(0);

    // Set up each individual button and connect their signals.
    auto upBtn = new QPushButton("Up", this);
    QObject::connect(upBtn, &QPushButton::clicked, parent, &MapWidget::panUp);
    layout->addWidget(upBtn, 0, 1);

    auto downBtn = new QPushButton("Down", this);
    QObject::connect(downBtn, &QPushButton::clicked, parent, &MapWidget::panDown);
    layout->addWidget(downBtn, 2, 1);

    auto leftBtn = new QPushButton("Left", this);
    QObject::connect(leftBtn, &QPushButton::clicked, parent, &MapWidget::panLeft);
    layout->addWidget(leftBtn, 1, 0);

    auto rightBtn = new QPushButton("Right", this);
    QObject::connect(rightBtn, &QPushButton::clicked, parent, &MapWidget::panRight);
    layout->addWidget(rightBtn, 1, 2);
}
