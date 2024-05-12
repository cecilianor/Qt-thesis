#ifndef MAPRENDERSETTINGSWIDGET_H
#define MAPRENDERSETTINGSWIDGET_H

// Qt header files.
#include <QBoxLayout>
#include <QWidget>

// Other header files.
#include "MapWidget.h"

namespace Bach {
/*!
 * \class MapRenderSettingsWidget
 * \brief The MapRenderSettingsWidget class contains the group of controls for adjusting the
 * rendering settings of a MapWidget.
 */
class MapRenderSettingsWidget : public QWidget {
public:
    MapRenderSettingsWidget(MapWidget* parent);
};
}

#endif // MAPRENDERSETTINGSWIDGET_H
