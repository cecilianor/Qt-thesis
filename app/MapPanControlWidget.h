// Copyright (c) 2024 Cecilia Norevik Bratlie, Nils Petter Skålerud, Eimen Oueslati
// SPDX-License-Identifier: MIT

#ifndef MAPPANCONTROLWIDGET_H
#define MAPPANCONTROLWIDGET_H

// Qt header files.
#include <QWidget>

// Other header files.
#include "MapWidget.h"

namespace Bach
{
/*!
 * \class MapPanControlWidget
 * \brief The MapPanControlWidget class contains the group of controls for panning.
 *
 * This is made to be used in tandem with a MapWidget.
 */
class MapPanControlWidget : public QWidget
{
public:
    MapPanControlWidget(MapWidget* parent);
};
}

#endif // MAPPANCONTROLWIDGET_H
