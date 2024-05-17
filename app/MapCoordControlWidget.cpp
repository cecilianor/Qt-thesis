// Copyright (c) 2024 Cecilia Norevik Bratlie, Nils Petter Skålerud, Eimen Oueslati
// SPDX-License-Identifier: MIT

// Qt header files.
#include <QDoubleValidator>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

// Other header files.
#include "MapCoordControlWidget.h"
#include "Rendering.h"

using Bach::MapCoordControlWidget;

/*!
 * \brief MapCoordControlWidget::setupInputFields places input fields into a map application.
 *
 * The input fields have labels for longitude, latitude, and zoom level. These fields
 * can take input from the user. The function also sets up a "Go" button that will
 * move the map to the specified longitude, latitude and zoom level.
 *
 * \param outerLayout The layout of the input fields, will be set to grid by the function.
 */
void MapCoordControlWidget::setupInputFields(QBoxLayout* outerLayout)
{
    // Build the grid layout
    QGridLayout *layout = new QGridLayout;
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

    // Sets up the latitude field.
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

    // Set up the submit button at the end.
    {
        QPushButton *btn = new QPushButton("Go", this);
        QObject::connect(
            btn,
            &QPushButton::clicked,
            this,
            &MapCoordControlWidget::goButtonPressed);
        outerLayout->addWidget(btn);
    }
}

/*!
 * \brief MapCoordControlWidget::setupButtons generates all application buttons.
 *
 * \param outerLayout The layout of application buttons for Gjøvik and Nydalen.
 * This is used to quickly be able to move to Gjøvik (where the developers are from),
 * and where Qt have their offices (in Nydalen).
 *
 * \param mapWidget The QWidget to render to.
 */
void MapCoordControlWidget::setupButtons(QBoxLayout *outerLayout, MapWidget *mapWidget)
{
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

/*!
 * \brief MapCoordControlWidget::MapCoordControlWidget
 * Controls placement of the map application's coordinate controls.
 *
 * \param mapWidget The QWidget to place this widget on top of.
 */
MapCoordControlWidget::MapCoordControlWidget(MapWidget* mapWidget) : QWidget(mapWidget)
{
    auto temp = new QWidget(mapWidget);

    //Line up widgets vertically.
    auto outerLayout = new QVBoxLayout;
    temp->setLayout(outerLayout);

    setupInputFields(outerLayout);
    setupButtons(outerLayout, mapWidget);
}

/*!
 * \brief MapCoordControlWidget::submitButtonPressed
 * Grabs zoom, longitude, and latitude values from a GUI.
 */
void MapCoordControlWidget::goButtonPressed()
{
    // Initial longitute, latitude, and zoom values.
    double longitude = 0;
    double latitude = 0;
    double zoom = 0;

    // Try to grab new viewport values from text fields.
    // Check that all values are valid before submitting them.
    QString longitudeText = longitudeField->text();
    if (longitudeText != "") {
        bool ok = false;
        longitude = longitudeText.toDouble(&ok);
        if (!ok)
            return;
    }

    QString latitudeText = latitudeField->text();
    if (latitudeText != "") {
        bool ok = false;
        latitude = latitudeText.toDouble(&ok);
        if (!ok)
            return;
    }

    QString zoomText = zoomField->text();
    if (zoomText != "") {
        bool ok = false;
        zoom = zoomText.toDouble(&ok);
        if (!ok)
            return;
    }
    // Convert coordinates to world normalised coordinates.
    auto [x, y] = Bach::lonLatToWorldNormCoordDegrees(longitude, latitude);

    emit submitNewCoords(x, y, zoom);
}
