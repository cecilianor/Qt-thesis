#include "MapCoordControlWidget.h"

#include <QDoubleValidator>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "Rendering.h"

using Bach::MapCoordControlWidget;

QString getShowingDebugBtnLabel(const MapWidget* mapWidget)
    auto name = QString("Showing debug ");
    if (mapWidget->isShowingDebug()) {
        name += "on";
    } else {
        name += "off";
    }
    return name;
}

QString getRenderingTileBtnLabel(const MapWidget* mapWidget)
{
    auto name = QString("Showing tile type: ");
    if (mapWidget->isRenderingVector()) {
        name += "Vector";
    } else {
        name += "Raster";
    }
    return name;
}
void MapCoordControlWidget::setupInputFields(QBoxLayout* outerLayout)
{
    // Build the grid layout
    auto layout = new QGridLayout;
    outerLayout->addLayout(layout);

    // This scope inserts the longitude field and label.
    {
        layout->addWidget(new QLabel("Longitude"), 0, 0);

        longitudeField = new QLineEdit;
        layout->addWidget(longitudeField, 0, 1);

        longitudeField->setPlaceholderText("Enter a number...");

        // Set up the text field to be limited to doubles in a specific range.
        double minValue = -180;
        double maxValue = 180;
        int decimalPlaces = 4;
        auto* validator = new QDoubleValidator(minValue, maxValue, decimalPlaces, longitudeField);
        validator->setNotation(QDoubleValidator::StandardNotation);
        validator->setLocale(QLocale::English);
        longitudeField->setValidator(validator);
    }

    // Sets up the latitude field
    {
        layout->addWidget(new QLabel("Latitude"), 1, 0);

        latitudeField = new QLineEdit;
        layout->addWidget(latitudeField, 1, 1);

        latitudeField->setPlaceholderText("Enter a number...");

        // Set up the text field to be limited to doubles in a specific range.
        double minValue = -85;
        double maxValue = 85;
        int decimalPlaces = 4;
        auto* validator = new QDoubleValidator(minValue, maxValue, decimalPlaces, latitudeField);
        validator->setNotation(QDoubleValidator::StandardNotation);
        validator->setLocale(QLocale::English);
        latitudeField->setValidator(validator);
    }

    // Sets up the longitude field.
    {
        layout->addWidget(new QLabel("Zoom"), 2, 0);

        zoomField = new QLineEdit;
        layout->addWidget(zoomField, 2, 1);

        zoomField->setPlaceholderText(QString("Enter a number..."));

        // Set up the text field to be limited to doubles in a specific range.
        double minValue = -1;
        double maxValue = 16;
        int decimalPlaces = 2;
        auto* validator = new QDoubleValidator(minValue, maxValue, decimalPlaces, zoomField);
        validator->setNotation(QDoubleValidator::StandardNotation);
        validator->setLocale(QLocale::English);
        zoomField->setValidator(validator);
    }

    // Setup the submit button at the end.
    {
        auto btn = new QPushButton("Go", this);
        QObject::connect(
            btn,
            &QPushButton::clicked,
            this,
            &MapCoordControlWidget::submitButtonPressed);
        outerLayout->addWidget(btn);
    }
}

void MapCoordControlWidget::setupButtons(QBoxLayout *outerLayout, MapWidget *mapWidget)
{
    // Setup the debug lines toggle switch
    {
        auto name = getShowingDebugBtnLabel(mapWidget);
        auto btn = new QPushButton(name, this);
        outerLayout->addWidget(btn);
        QObject::connect(
            btn,
            &QPushButton::clicked,
            this,
            [=]() {
                // Send signal to mapWidget?
                mapWidget->toggleIsShowingDebug();
                auto name = getShowingDebugBtnLabel(mapWidget);
                btn->setText(name);
            });
    }

    // Setup the toggle tile type button (supports vector and raster for now).
    {
        auto name = getRenderingTileBtnLabel(mapWidget);
        auto btn = new QPushButton(name, this);
        outerLayout->addWidget(btn);
        QObject::connect(
            btn,
            &QPushButton::clicked,
            this,
            [=]() {
                // Send signal to mapWidget
                mapWidget->toggleIsRenderingVectorTile();
                auto name = getRenderingTileBtnLabel(mapWidget);
                btn->setText(name);
            });
    }

    // Create buttons to move the viewport to Nydalen.
    {
        auto btn = new QPushButton("Nydalen", this);
        outerLayout->addWidget(btn);
        QObject::connect(
            btn,
            &QPushButton::clicked,
            mapWidget,
            [=]() {
                auto [x, y] = Bach::lonLatToWorldNormCoordDegrees(10.765248, 59.949584413);
                mapWidget->setViewport(x, y, 12);
            });
    }

    // Create buttons to move the viewport to Gjøvik.
    {
        auto btn = new QPushButton("Gjøvik", this);
        outerLayout->addWidget(btn);
        QObject::connect(
            btn,
            &QPushButton::clicked,
            mapWidget,
            [=]() {
                auto [x, y] = Bach::lonLatToWorldNormCoordDegrees(10.683791293772392, 60.79004068859685);
                mapWidget->setViewport(x, y, 12);
            });
    }
}

MapCoordControlWidget::MapCoordControlWidget(MapWidget* mapWidget)
{
    auto temp = new QWidget(mapWidget);
    // Set a dark and transparent background.
    //temp->setStyleSheet("background-color: rgba(0, 0, 0, 127);");

    /* From here on we establish a small layout.
     */
    auto outerLayout = new QVBoxLayout;
    temp->setLayout(outerLayout);

    setupInputFields(outerLayout);
    setupButtons(outerLayout, mapWidget);
}

void MapCoordControlWidget::submitButtonPressed()
{
    // Initial longitute, latitude, and zoom values.
    double longitude = 0;
    double latitude = 0;
    double zoom = 0;

    // Try to grab new viewport values from text fields.
    // Check that all values are valid before submitting them.
    auto longitudeText = longitudeField->text();
    if (longitudeText != "") {
        bool ok = false;
        longitude = longitudeText.toDouble(&ok);
        if (!ok) {
            return;
        }
    }

    auto latitudeText = latitudeField->text();
    if (latitudeText != "") {
        bool ok = false;
        latitude = latitudeText.toDouble(&ok);
        if (!ok) {
            return;
        }
    }

    auto zoomText = zoomField->text();
    if (zoomText != "") {
        bool ok = false;
        zoom = zoomText.toDouble(&ok);
        if (!ok) {
            return;
        }
    }

    auto [x, y] = Bach::lonLatToWorldNormCoordDegrees(longitude, latitude);

    emit submitNewCoords(x, y, zoom);
}
