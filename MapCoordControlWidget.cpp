#include "MapCoordControlWidget.h"

#include <QDoubleValidator>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "Rendering.h"

using Bach::MapCoordControlWidget;

MapCoordControlWidget::MapCoordControlWidget(QWidget* parent)
{
    auto temp = new QWidget(parent);
    // Set a dark and transparent background.
    temp->setStyleSheet("background-color: rgba(0, 0, 0, 127);");

    /* From here on we establish a small layout.
     * The outer one is a VBoxLayout, inside are two elements: a grid and then the submit button.
     *
     * The grid contains the labels and the text fields.
     */
    auto outerLayout = new QVBoxLayout;
    temp->setLayout(outerLayout);

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
        int decimalPlaces = 2;
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
        int decimalPlaces = 2;
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
        QObject::connect(btn, &QPushButton::clicked, this, &MapCoordControlWidget::buttonPressed);
        outerLayout->addWidget(btn);
    }

    // Setup the load tiles button
    {
        auto btn = new QPushButton("Load tiles", this);
        //QObject::connect(btn, &QPushButton::clicked, this, [this]() { emit loadNewTiles; });
        outerLayout->addWidget(btn);
    }

}

void MapCoordControlWidget::buttonPressed()
{
    // Try to grab the new viewport values from our text fields.
    // If any of them are invalid, we don't submit.

    double longitude = 0;
    auto longitudeText = longitudeField->text();
    if (longitudeText != "") {
        bool ok = false;
        longitude = longitudeText.toDouble(&ok);
        if (!ok) {
            return;
        }
    }

    double latitude = 0;
    auto latitudeText = latitudeField->text();
    if (latitudeText != "") {
        bool ok = false;
        latitude = latitudeText.toDouble(&ok);
        if (!ok) {
            return;
        }
    }

    double zoom = 0;
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
