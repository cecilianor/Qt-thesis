#ifndef UTILITIES_H
#define UTILITIES_H

#include <QObject>
#include <QString>
#include <QByteArray>

#include <optional>

/*!
 * \brief The TileType enum determines what tile type to render.
 *
 * Supported types:
 *
 * * Vector: Is in the .mvt file format.
 * * Raster: Can only be the .png file format for now.
 */
enum class TileType {
    Vector,
    Raster,
};

/*!
 * \brief The FileFormat enum can be used to handle different tile file formats.
 *
 * This type is currently not used, but may be relevant if it's critical to handle
 * file types that are not .mvt or .png.
 */
enum class TileFileFormat {
    Mvt,
    Png,
    Jpg,
    Unknown,
};

/*!
 * @brief The StyleSheetType enum covers the basic style sheets provided by MapTiler.
 *
 * Some of the style sheet types have _vX at the end. This matches the current map version
 * that the code is implemented against in case the MapTiler APIs are updated
 * in the future with new endpoints that include -v2, -v3 and so on.
 */
enum class MapType {
    Backdrop,
    BasicV2,
    BrightV2,
    Dataviz,
    Ocean,
    OpenStreetMap,
    OutdoorV2,
    Satellite,
    StreetsV2,
    TonerV2,
    TopoV2,
    WinterV2,
    Unknown,
};

/*!
 * \brief The SourceType enum contains MapTiler sources.
 *
 * These source types are specified by the MapTiler API.
 */
enum class SourceType {
    MaptilerPlanet,
    Land,
    Ocean,
    Unknown,
};

/*!
 * \brief The ResultType enum contains specific result types.
 *
 * This enum is customized to the Mapping project, and
 * contians errors related to http requests, data
 * loading, and parsing. If there is no error,
 * `success` can be used.
 */
enum class ResultType {
    Success,
    MapTilerError,
    StyleSheetNotFound,
    TileSheetNotFound,
    UnknownSourceType,
    NoData,
    NoImplementation,
    NetworkError,
    ParseError,
    UnknownError,
};

/*!
 * \brief PrintResultTypeInfo converts a ResultType to a string.
 * \param r - The result type to print.
 * \return the result type as a string message.
 */
const inline QString PrintResultTypeInfo(ResultType r) {
    QString str;

    switch (r)
    {
    case ResultType::Success:
        str = "Success";
    case ResultType::MapTilerError:
        str = "Maptiler error";
    case ResultType::StyleSheetNotFound:
        str = "Style sheet not found";
    case ResultType::TileSheetNotFound:
        str = "Tile sheet not found";
    case ResultType::UnknownSourceType:
        str = "Unknown source type";
    case ResultType::NoData:
        str ="No returned data";
    case ResultType::NoImplementation:
        str = "No implementation";
    case ResultType::ParseError:
        str = "Parsing error";
    case ResultType::UnknownError:
        str = "Unknown error";
    case ResultType::NetworkError:
        str = "Network error";
    default:
        str = "Unknown error. Check ResultType documentation for PrintResultTypeInfo.";
    }

    return str;
}

/*!
 * \brief The HttpResponse struct contains a HTTP response and a result.
 *
 * The response is stored as a QByteArray, which will be empty if
 * the HTTP request fails. The ResultType records if the HTTP
 * request and the parsing of it was successful or not.
 */
struct HttpResponse  {
    QByteArray response;
    ResultType resultType;
};

/*!
 * \brief The ParsedLink struct contains a link and a result.
 *
 * The link is stored as a QString, which will be empty (or "") if
 * the parsing of the link fails. The ResultType records if the
 * reading and parsing of the link was successful or not.
 */
struct ParsedLink {
    QString link;
    ResultType resultType;
};

namespace Bach {
    /*! \brief
     *  Contains the name of the environment variable where we expect
     *  the MapTiler key to be stored.
    */
    const QString mapTilerKeyEnvName = "MAPTILER_KEY";

    std::optional<QString> rasterTilesheetUrlFromMapType(MapType maptype);

    bool writeNewFileHelper(const QString& path, const QByteArray &bytes);

    std::optional<QString> readMapTilerKey(const QString &filePath);

    HttpResponse requestAndWait(const QString &url);

    HttpResponse requestStyleSheetFromWeb(MapType type, const QString &key);

    HttpResponse loadStyleSheetBytes(
        MapType type,
        const std::optional<QString> &mapTilerKey);

    std::optional<QJsonDocument> loadStyleSheetJson(
        MapType type,
        const std::optional<QString> &mapTilerKey);

    std::optional<QJsonDocument> loadVectorTileSheet(
        const QJsonDocument &styleSheet,
        const QString &sourceType);

    std::optional<QJsonDocument> loadRasterTileSheet(
        MapType mapType,
        std::optional<QString> mapTilerKey);

    ParsedLink getTilesLinkFromStyleSheet(
        const QJsonDocument &styleSheet,
        const QString &sourceType);

    ParsedLink getTileUrlTemplateFromTileSheet(const QJsonDocument &tileSheet);

    ParsedLink getPbfUrlTemplate(
        const QJsonDocument &styleSheet,
        const QString &sourceType);

    ParsedLink getPngUrlTemplate(
        MapType mapType,
        std::optional<QString> mapTilerKey);
}

#endif // UTILITIES_H
