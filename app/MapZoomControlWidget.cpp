#include "MapZoomControlWidget.h"

#include <QVBoxLayout>
#include <QPushButton>

using Bach::MapZoomControlWidget;

MapZoomControlWidget::MapZoomControlWidget(MapWidget* parent) : QWidget(parent) {
    // We set up our buttons in a simple vertical box layout.
    auto vLayout = new QVBoxLayout(this);
    setLayout(vLayout);
    //vLayout->setContentsMargins(0,0,0,0);
    //vLayout->setSpacing(0);

    // Then we set up each individual button and hook them up to the MapWidget events.
    auto zoomInBtn = new QPushButton("+", this);
    QObject::connect(zoomInBtn, &QPushButton::clicked, parent, &MapWidget::zoomIn);
    vLayout->addWidget(zoomInBtn);

    auto zoomOutBtn = new QPushButton("-", this);
    QObject::connect(zoomOutBtn, &QPushButton::clicked, parent, &MapWidget::zoomOut);
    vLayout->addWidget(zoomOutBtn);
}
