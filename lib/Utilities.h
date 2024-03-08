#ifndef UTILITIES_H
#define UTILITIES_H

#include <QObject>
#include <QString>
#include <QByteArray>

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
        str = "Unknown error. Check documentation for PrintResultTypeInfo.";
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

#endif // UTILITIES_H
