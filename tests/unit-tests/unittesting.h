#include <QTest>

#include <QObject>

class UnitTesting : public QObject
{
    Q_OBJECT

private slots:
    /* TileLoader tests */
    void readKey_returns_success_when_valid_key();
    void readKey_returns_failure_when_invalid_key();
    void getStyleSheet_returns_success_on_supported_stylesheet();
    void getStyleSheet_returns_failure_on_unsupported_stylesheet();
    void getTilesLink_valid_style_sheet_returns_success();
    void getTilesLink_unknown_source_type_returns_unknown_source_type_error();
    void getTilesLink_missing_url_returns_tile_sheet_not_found_error();
    void loadTileFromCache_fails_on_broken_file();
    void loadTileFromCache_parses_cached_file_successfully();
    void check_new_tileLoader_has_no_tiles();

    /* Rendering tests
     */
    void longLatToWorldNormCoordDegrees_returns_expected_basic_values();
    void calcVisibleTiles_returns_expected_basic_cases();
    void calcViewportSizeNorm_returns_expected_basic_cases();
    void calcMapZoomLevelForTileSizePixels_returns_expected_basic_values();


    /* Layerstyle tests
     */
    void getStopOutput_returns_basic_values();
    void parseSheet_returns_basic_values();

    /* VectorTile tests
     */
    void tileFromByteArray_returns_basic_values();

    /* Evaluator test
     */
    void resolveExpression_returns_basic_values();


};
