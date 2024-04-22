// Qt headers
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>

// Other header files
#include "MapRenderSettingsWidget.h"
#include "Rendering.h"

using Bach::MapRenderSettingsWidget;

/*!
 * \brief getShowingDebugBtnLabel creates a string to label the debug button.
 * \param mapWidget is the QWidget to render to.
 * \return the label string.
 */
static QString getShowingDebugBtnLabel(const MapWidget* mapWidget)
{
    auto name = QString("Showing debug ");
    if (mapWidget->isShowingDebug()) {
        name += "on";
    } else {
        name += "off";
    }
    return name;
}

/*!
 * \brief getRenderingTileBtnLabel creates a string to label the toggle rendering tile button.
 * \param mapWidget is the QWidget to render to.
 * \return the label string.
 */
static QString getRenderingTileBtnLabel(const MapWidget* mapWidget)
{
    auto name = QString("Showing tile type: ");
    if (mapWidget->isRenderingVector()) {
        name += "Vector";
    } else {
        name += "Raster";
    }
    return name;
}
/*!
 * \brief MapCoordControlWidget::MapCoordControlWidget controls ???
 * \param mapWidget is the QWidget to render to.
 */
MapRenderSettingsWidget::MapRenderSettingsWidget(MapWidget* mapWidget) : QWidget(mapWidget)
{
    // We set up our buttons in a simple vertical box layout.
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    // Setup the debug lines toggle switch
    {
        QString name = getShowingDebugBtnLabel(mapWidget);
        QPushButton *btn = new QPushButton(name, this);
        layout->addWidget(btn);
        QObject::connect(
            btn,
            &QPushButton::clicked,
            mapWidget,
            [=]() {
                // Send signal to mapWidget
                mapWidget->toggleIsShowingDebug();
                QString name = getShowingDebugBtnLabel(mapWidget);
                btn->setText(name);
            });
    }

    // Setup the toggle tile type button (supports vector and raster for now).
    {
        QString name = getRenderingTileBtnLabel(mapWidget);
        QPushButton *btn = new QPushButton(name, this);
        layout->addWidget(btn);
        QObject::connect(
            btn,
            &QPushButton::clicked,
            mapWidget,
            [=]() {
                // Send signal to mapWidget
                mapWidget->toggleIsRenderingVectorTile();
                QString name = getRenderingTileBtnLabel(mapWidget);
                btn->setText(name);
            });
    }

    // Setup the checkbox and text for drawing fill elements.
    {
        QCheckBox *checkbox = new QCheckBox("Fill", this);
        checkbox->setCheckState(mapWidget->isRenderingFill() ? Qt::Checked : Qt::Unchecked);
        layout->addWidget(checkbox);
        QObject::connect(
            checkbox,
            &QCheckBox::checkStateChanged,
            mapWidget,
            [=](Qt::CheckState boxIsChecked) {
                mapWidget->setShouldDrawFill(boxIsChecked == Qt::Checked);
            });
    }

    // Setup the checkbox and text for drawing line elements.
    {
        QCheckBox *checkbox = new QCheckBox("Lines", this);
        checkbox->setCheckState(mapWidget->isRenderingLines() ? Qt::Checked : Qt::Unchecked);
        layout->addWidget(checkbox);
        QObject::connect(
            checkbox,
            &QCheckBox::checkStateChanged,
            mapWidget,
            [=](Qt::CheckState boxIsChecked) {
                mapWidget->setShouldDrawLines(boxIsChecked == Qt::Checked);
            });
    }

     // Setup the checkbox and text for drawing text elements.
    {
        QCheckBox *checkbox = new QCheckBox("Text", this);
        checkbox->setCheckState(mapWidget->isRenderingText() ? Qt::Checked : Qt::Unchecked);
        layout->addWidget(checkbox);
        QObject::connect(
            checkbox,
            &QCheckBox::checkStateChanged,
            mapWidget,
            [=](Qt::CheckState boxIsChecked) {
                mapWidget->setShouldDrawText(boxIsChecked == Qt::Checked);
            });
    }

    // Setup the checkbox and text for using textLayout.
    {
        QCheckBox *checkbox = new QCheckBox("QTextLayout", this);
        checkbox->setCheckState(mapWidget->isUsingTextLayout() ? Qt::Checked : Qt::Unchecked);
        layout->addWidget(checkbox);
        QObject::connect(
            checkbox,
            &QCheckBox::checkStateChanged,
            mapWidget,
            [=](Qt::CheckState boxIsChecked) {
                mapWidget->setShouldUseTextLayout(boxIsChecked == Qt::Checked);
            });
    }
}
