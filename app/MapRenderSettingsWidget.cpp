// Qt headers.
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>

// Other header files.
#include "MapRenderSettingsWidget.h"

using Bach::MapRenderSettingsWidget;

/*!
 * \brief getShowingDebugBtnLabel creates a string to label the debug button.
 * \param mapWidget The QWidget to render to.
 * \return the generated label string.
 */
static QString getShowingDebugBtnLabel(const MapWidget* mapWidget)
{
    auto name = QString("Showing debug ");
    if (mapWidget->isShowingDebug())
        name += "on";
    else
        name += "off";
    return name;
}

/*!
 * \brief getRenderingTileBtnLabel creates a string to label the toggle rendering tile button.
 * \param mapWidget The QWidget to render to.
 * \return the generated label string.
 */
static QString getRenderingTileBtnLabel(const MapWidget* mapWidget)
{
    auto name = QString("Showing tile type: ");
    if (mapWidget->isRenderingVector())
        name += "Vector";
    else
        name += "Raster";
    return name;
}

/*!
 * \brief MapCoordControlWidget::MapCoordControlWidget
 * Controls map coordinate widget.
 *
 * \param mapWidget The QWidget to render to.
 */
MapRenderSettingsWidget::MapRenderSettingsWidget(MapWidget* mapWidget) : QWidget(mapWidget)
{
    // Set up buttons in a simple vertical box layout.
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    // Set up the debug lines toggle switch.
    QString debugBtnName = getShowingDebugBtnLabel(mapWidget);
    QPushButton *debugBtn = new QPushButton(debugBtnName, this);
    layout->addWidget(debugBtn);
    QObject::connect(
        debugBtn,
        &QPushButton::clicked,
        mapWidget,
        [=]() {
            // Send signal to mapWidget.
            mapWidget->toggleIsShowingDebug();
            QString name = getShowingDebugBtnLabel(mapWidget);
            debugBtn->setText(name);
        });

    // Set up the toggle map tile type button.
    QString renderBtnName = getRenderingTileBtnLabel(mapWidget);
    QPushButton *renderBtn = new QPushButton(renderBtnName, this);
    layout->addWidget(renderBtn);
    QObject::connect(
        renderBtn,
        &QPushButton::clicked,
        mapWidget,
        [=]() {
            // Send signal to mapWidget
            mapWidget->toggleIsRenderingVectorTile();
            QString name = getRenderingTileBtnLabel(mapWidget);
            renderBtn->setText(name);
        });

    // Set up the checkbox and text for drawing fill elements.
    QCheckBox *fillCheckbox = new QCheckBox("Fill", this);
    fillCheckbox->setCheckState(mapWidget->isRenderingFill() ? Qt::Checked : Qt::Unchecked);
    layout->addWidget(fillCheckbox);
    QObject::connect(
        fillCheckbox,
        &QCheckBox::checkStateChanged,
        mapWidget,
        [=](Qt::CheckState boxIsChecked) {
            mapWidget->setShouldDrawFill(boxIsChecked == Qt::Checked);
        });

    // Set up the checkbox and text for drawing line elements.
    QCheckBox *linesCheckbox = new QCheckBox("Lines", this);
    linesCheckbox->setCheckState(mapWidget->isRenderingLines() ? Qt::Checked : Qt::Unchecked);
    layout->addWidget(linesCheckbox);
    QObject::connect(
        linesCheckbox,
        &QCheckBox::checkStateChanged,
        mapWidget,
        [=](Qt::CheckState boxIsChecked) {
            mapWidget->setShouldDrawLines(boxIsChecked == Qt::Checked);
        });

    // Set up the checkbox and text for drawing text elements.
    QCheckBox *textCheckbox = new QCheckBox("Text", this);
    textCheckbox->setCheckState(mapWidget->isRenderingText() ? Qt::Checked : Qt::Unchecked);
    layout->addWidget(textCheckbox);
    QObject::connect(
        textCheckbox,
        &QCheckBox::checkStateChanged,
        mapWidget,
        [=](Qt::CheckState boxIsChecked) {
            mapWidget->setShouldDrawText(boxIsChecked == Qt::Checked);
        });
}
