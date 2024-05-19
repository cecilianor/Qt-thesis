// Copyright (c) 2024 Cecilia Norevik Bratlie, Nils Petter Skålerud, Eimen Oueslati
// SPDX-License-Identifier: MIT

// Qt header files.
#include <QGridLayout>
#include <QPushButton>

// Other header files.
#include "MapPanControlWidget.h"

using Bach::MapPanControlWidget;

/*!
 * \brief MapPanControlWidget::MapPanControlWidget
 * Sets up graphical user interface buttons to move up, down, left, and right in app.
 *
 * \param parent The MapWidget application that panning controls are attached to.
 */
MapPanControlWidget::MapPanControlWidget(MapWidget* parent) : QWidget(parent)
{
    // 4 directional buttons are set up in a 3x3 grid where most cells are empty.
    auto layout = new QGridLayout(this);
    setLayout(layout);

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
