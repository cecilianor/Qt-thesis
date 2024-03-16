#ifndef UTILITIES_H
#define UTILITIES_H

#include <QObject>
#include <QString>
#include <QByteArray>

#include <optional>

/*!
 * @brief The StyleSheetType enum covers the basic style sheets provided by MapTiler.
 *
 * Some of the style sheet types have _vX at the end. This matches the current map version
 * that the code is implemented against in case the MapTiler APIs are updated
 * in the future with new endpoints that include -v2, -v3 and so on.
 */
enum class StyleSheetType {
    backdrop,
    basic_v2,
    bright_v2,
    dataviz,
    ocean,
    open_street_map,
    outdoor_v2,
    satellite,
    streets_v2,
    toner_v2,
    topo_v2,
    winter_v2,
    unknown,
};


/*!
 * \brief The SourceType enum contains MapTiler sources.
 *
 * These source types are specified by the MapTiler API.
 */
enum class SourceType {
    maptiler_planet,
    land,
    ocean,
    unknown,
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
    success,
    mapTilerError,
    styleSheetNotFound,
    tileSheetNotFound,
    unknownSourceType,
    noData,
    noImplementation,
    networkError,
    parseError,
    unknownError,
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
    case ResultType::success:
        str = "Success";
    case ResultType::mapTilerError:
        str = "Maptiler error";
    case ResultType::styleSheetNotFound:
        str = "Style sheet not found";
    case ResultType::tileSheetNotFound:
        str = "Tile sheet not found";
    case ResultType::unknownSourceType:
        str = "Unknown source type";
    case ResultType::noData:
        str ="No returned data";
    case ResultType::noImplementation:
        str = "No implementation";
    case ResultType::parseError:
        str = "Parsing error";
    case ResultType::unknownError:
        str = "Unknown error";
    case ResultType::networkError:
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
    /*!
     * @brief Helper function to write a byte array to disk.
     * This function will automatically establish any necessary
     * parent directories, unlike the standard QFile::open.
     *
     * This function will only succeed if it was able to establish the path
     * This function will only succeed if the file did not already exist.
     *
     * Created directory will not be removed in the case that writing to file fails.
     *
     * @param path is the path to the file. Must contain filename, cannot be directory.
     * @param Takes the byte-array to write.
     * @return true if success, false if failed.
     */
    bool writeNewFileHelper(const QString& path, const QByteArray &bytes);

    /*!
     * @brief Reads MapTiler key from file.
     * @param filePath is the relative path + filename that's storing the key.
     * @return The key if successfully read from file.
     */
    std::optional<QString> readMapTilerKey(const QString &filePath);

    /*!
     * \brief Helper function to make a network request.
     * This function makes a simple GET request to the URL supplied.
     *
     * It waits for the response before
     * returning the result.
     *
     * Should only be used during startup of the program.
     *
     * This is a re-entrant function.
     * \return
     */
    HttpResponse requestAndWait(const QString &url);

    /*!
     * @brief Makes a blocking network request to get a stylesheet from MapTiler
     * @param type The style of the stylesheet
     * @param key The MapTiler key.
     * @return The response from MapTiler.
     */
    HttpResponse requestStyleSheetFromWeb(StyleSheetType type, const QString &key);

    /*!
     * \brief Loads the bytes of the stylesheet.
     * Will attempt to load from cache first, then try downloading from web.
     * If loaded from web, it will then try to write the result to disk cache.
     * This is a blocking and re-entrant function.
     *
     */
    HttpResponse loadStyleSheetBytes(
        StyleSheetType type,
        const std::optional<QString> &mapTilerKey);

    /*!
     * @brief TileURL::getTilesLink grabs a link to a mapTiler tile sheet
     * @param styleSheet is the stylesheet to get the link from
     * @param sourceType is the map source type passed as a QString
     * @return link as a string
     */
    ParsedLink getTilesLinkFromStyleSheet(
        const QJsonDocument &styleSheet,
        const QString &sourceType);

    ParsedLink getPbfLinkTemplate(const QJsonDocument &styleSheet, const QString &sourceType);

    /*!
     * @brief TileURL::getPBFLink gets a PBF link based on the url to a tile sheet.
     *
     * The function returns either a success message or an error message and error code.
     *
     * @param tileSheetUrl the link/url to the stylesheet.
     * @return The link to PBF tiles.
     */
    ParsedLink getPbfLinkFromTileSheet(const QString &tileSheetUrl);
}

#endif // UTILITIES_H
