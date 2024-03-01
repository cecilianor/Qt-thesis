#include <QTest>

#include <QObject>

class UnitTesting : public QObject
{
    Q_OBJECT

private slots:

    void readKey_returns_success_when_valid_key();
    void readKey_returns_failure_when_invalid_key();
    void getStyleSheet_returns_success_on_supported_stylesheet();
    void getStyleSheet_returns_failure_on_unsupported_stylesheet();


    /* Rendering tests
     */
    void longLatToWorldNormCoordDegrees_returns_expected_basic_values();
    void calcVisibleTiles_returns_expected_basic_cases();
    void calcViewportSizeNorm_returns_expected_basic_cases();
    void calcMapZoomLevelForTileSizePixels_returns_expected_basic_values();


    /* Layerstyle tests
     */
    void interpolate_returns_basic_expected_values();
    void getLerpedValue_returns_basic_expceted_value();
};
