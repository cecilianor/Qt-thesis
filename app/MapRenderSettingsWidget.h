#ifndef MAPRENDERSETTINGSWIDGET_H
#define MAPRENDERSETTINGSWIDGET_H

#include <QWidget>
#include <QBoxLayout>

#include "MapWidget.h"

namespace Bach {
    /*!
     * \class
     * Contains the group of controls for adjusting the
     * rendering settings of a MapWidget.
     */
    class MapRenderSettingsWidget : public QWidget {
    public:
        MapRenderSettingsWidget(MapWidget* parent);
    };
}

#endif // MAPRENDERSETTINGSWIDGET_H
