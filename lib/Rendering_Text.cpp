#include "Rendering.h"
#include "Evaluator.h"

/* Finds the text color of this feature at given zoom level.
 */
static QColor getTextColor(
    const SymbolLayerStyle &layerStyle,
    const PointFeature &feature,
    int mapZoom,
    double vpZoom)
{
    QVariant color = layerStyle.getTextColorAtZoom(mapZoom);
    // The layer style might return an expression, we need to resolve it.
    if(color.typeId() == QMetaType::Type::QJsonArray){
        color = Evaluator::resolveExpression(
            color.toJsonArray(),
            &feature,
            mapZoom,
            vpZoom);
    }
    return color.value<QColor>();
}


/* Finds the text size of this feature at given zoom level.
 */
static int getTextSize(
    const SymbolLayerStyle &layerStyle,
    const PointFeature &feature,
    int mapZoom,
    double vpZoom)
{
    QVariant size = layerStyle.getTextSizeAtZoom(mapZoom);
    // The layer style might return an expression, we need to resolve it.
    if(size.typeId() == QMetaType::Type::QJsonArray){
        size = Evaluator::resolveExpression(
            size.toJsonArray(),
            &feature,
            mapZoom,
            vpZoom);
    }
    return size.value<int>();
}


/* Finds the text opacity of this feature at given zoom level.
 */
static float getTextOpacity(
    const SymbolLayerStyle &layerStyle,
    const PointFeature &feature,
    int mapZoom,
    double vpZoom)
{
    QVariant opacity = layerStyle.getTextOpacityAtZoom(mapZoom);
    // The layer style might return an expression, we need to resolve it.
    if(opacity.typeId() == QMetaType::Type::QJsonArray){
        opacity = Evaluator::resolveExpression(
            opacity.toJsonArray(),
            &feature,
            mapZoom,
            vpZoom);
    }
    return opacity.value<float>();
}



/* Gets the text to be rendered.
 */
static QString getTextContent(
    const SymbolLayerStyle &layerStyle,
    const PointFeature &feature,
    int mapZoom,
    double vpZoom)
{
    QVariant textVariant = layerStyle.m_textField;
    if(textVariant.isNull() || !textVariant.isValid()) return "";
    // The layer style might return an expression, we need to resolve it.
    if(textVariant.typeId() == QMetaType::Type::QJsonArray){
        textVariant = Evaluator::resolveExpression(
            textVariant.toJsonArray(),
            &feature,
            mapZoom,
            vpZoom);
        return textVariant.toString();
    }else{ //In case the text field is just a string of the key for the metadata map.
        QString textFieldKey = textVariant.toString();
        textFieldKey.remove("{");
        textFieldKey.remove("}");
        if(!feature.featureMetaData.contains(textFieldKey)){
            return "";
        }
        return feature.featureMetaData[textFieldKey].toString();
    }
}



/* Checks if the bouding rect for textRect collids with any rects in rectList
 */
static bool isOverlapping(const QRect &textRect, const QVector<QRect> &rectList){
    for(const auto& rect : rectList){
        if(rect.intersects(textRect)) return true;
    }
    return false;
}


/* Splits text to multiple strings depending on the text length and the maximum allowed rect width
 */
static QList<QString> getCorrectedText(const QString &text, const QFont & font, const int rectWidth){
    QFontMetrics fontMetrics(font);
    int rectWidthInPix = font.pixelSize() * rectWidth;
    if(fontMetrics.horizontalAdvance(text) <= rectWidthInPix)
        return QList<QString>(text);

    QList<QString> words = text.split(" ");
    QList<QString> wordClusters;
    QString currentCluster = words.at(0);
    for(const auto &word : words.sliced(1)){
        if(fontMetrics.horizontalAdvance(currentCluster + " " + word) > rectWidthInPix){
            wordClusters.append(currentCluster);
            currentCluster = word;
            continue;
        }
        currentCluster += " " + word;
    }
    wordClusters.append(currentCluster);
    return wordClusters;
}


/* Render a text that does not wrap (one-liners)
 */
static void paintSimpleText(
    const QString &text,
    const QPoint &coordinate,
    const int outlineSize,
    const QColor &outlineColor,
    const QFont &textFont,
    QVector<QRect> &rects,
    QPainter &painter,
    const PointFeature &feature,
    const SymbolLayerStyle &layerStyle,
    const int mapZoom,
    const double vpZoom)
{

    //Create a QPainterPath for the text.
    QPainterPath textPath;
    //Create the path with no offset first.
    textPath.addText({}, textFont, text);

    QRectF boundingRect = textPath.boundingRect().toRect();
    //We account for the text outline when calculating the bounding rect size.
    boundingRect.setWidth(boundingRect.width() + 2 * outlineSize);
    boundingRect.setHeight(boundingRect.height() + 2 * outlineSize);
    //The text is supposed to be rendered such that the goemetry point is poistioned at the cented of the text,
    //however, the painter draws the text such that the point is at the bottom left of the text.
    //So we have to account for that and translate the drawing point with half the width and height of the bounding
    //rectangle of the original text.
    qreal textCenteringOffsetX = -boundingRect.width() / 2.;
    qreal textCenteringOffsetY = boundingRect.height() / 2.;
    textPath.translate({textCenteringOffsetX, textCenteringOffsetY});
    textPath.translate(coordinate);
    boundingRect.translate({textCenteringOffsetX, textCenteringOffsetY});
    boundingRect.translate(coordinate);


    // Set the pen for the outline color and width.
    QPen outlinePen(outlineColor, outlineSize); // Outline color and width
    //Check if the text overlaps with any previously rendered text.
    if(isOverlapping(boundingRect.toRect(), rects)) return;
    //Add the total bouding rect to the list of the text rects to check for overlap for upcoming text.
    rects.append(boundingRect.toRect());
    //Draw  the text.
    painter.strokePath(textPath, outlinePen);
    painter.fillPath(textPath, getTextColor(layerStyle, feature, mapZoom, vpZoom));
}


/* Render a text that should be drawn on multiple lines.
 */
static void paintCompositeText(
    const QList<QString> &texts,
    const QPoint &coordinates,
    const int outlineSize,
    const QColor &outlineColor,
    const QFont &textFont,
    QVector<QRect> &rects,
    QPainter &painter,
    const PointFeature &feature,
    const SymbolLayerStyle &layerStyle,
    const int mapZoom,
    const double vpZoom)
{
    //The font metrics var is used to calculate how much space does each word consume.
    QFontMetricsF fmetrics(textFont);
    //This is the hight of text character, this is used to calculate the combined hight of all the substrings' bounding rects.
    qreal height = fmetrics.height();
    //This will hold the paths for all the substrings of the text.
    QList<QPainterPath> paths;
    // Create a temporary QPainterPath for the loop.
    QPainterPath temp;
    //Loop over each substring and calculate its correct position.
    for(int i = 0; i < texts.size(); i++){
        temp.addText({}, textFont, texts.at(i));
        QRectF boundingRect = temp.boundingRect().toRect();
        //We account for the text outline when calculating the bounding rect size.
        boundingRect.setWidth(boundingRect.width() + 2 * outlineSize);
        boundingRect.setHeight(boundingRect.height() + 2 * outlineSize);
        //The text is supposed to be rendered such that the goemetry point is poistioned at the cented of the text,
        //however, the painter draws the text such that the point is at the bottom left of the text.
        //So we have to account for that and translate the drawing point with half the width and height of the bounding
        //rectangle of the original text. We also have to consider the postion of the current substring relative to the
        //other substrings.
        qreal textCenteringOffsetX = -boundingRect.width() / 2.;
        qreal textCenteringOffsetY = boundingRect.height() / 2.;
        temp.translate({textCenteringOffsetX, textCenteringOffsetY + ((i - (texts.size() / 2.)) * height)});
        temp.translate(coordinates);
        boundingRect.translate({textCenteringOffsetX, textCenteringOffsetY + ((i - (texts.size() / 2.)) * height)});
        boundingRect.translate(coordinates);
        //Add the current text path to the list and clear it for the next iteration.
        paths.append(temp);
        temp.clear();
    }

    //Combine the bounding rects of all the substrings to get the total bounding rect.
    QRect boundingRect = paths.at(0).boundingRect().toRect();
    for(const auto &path : paths.sliced(1)){
        boundingRect = boundingRect.united(path.boundingRect().toRect());
    }

    //Check if the text overlaps with any previously rendered text.
    if(isOverlapping(boundingRect, rects)) return;

    // Set the pen for the outline color and width.
    QPen outlinePen(outlineColor, outlineSize);
    //Add the total bouding rect to the list of the text rects to check for overlap for upcoming text.
    rects.append(boundingRect);
    //Draw all the text parts
    for(const auto &path : paths){
        painter.strokePath(path, outlinePen);
        painter.fillPath(path, getTextColor(layerStyle, feature, mapZoom, vpZoom));
    }
}

/* Paints a single Point feature within a tile.
 *
 * Assumes the painters origin has moved to the tiles origin.
 */
void Bach::paintSingleTileFeature_Point(
    PaintingDetailsPoint details,
    const int tileSize,
    QVector<QRect> &rects)
{
    QPainter &painter = *details.painter;
    const SymbolLayerStyle &layerStyle = *details.layerStyle;
    const PointFeature &feature = *details.feature;
    //Get the text to be rendered.
    QString textToDraw = getTextContent(layerStyle, feature, details.mapZoom, details.vpZoom);
    //If there is no text then there is nothing to render, we return
    if(textToDraw == "") return;

    //Get the rendering parameters from the layerstyle and set the relevant painter field.
    painter.setBrush(Qt::NoBrush);
    int textSize = getTextSize(layerStyle, feature, details.mapZoom, details.vpZoom);
    QFont textFont = QFont(layerStyle.m_textFont);
    textFont.setPixelSize(textSize);
    painter.setOpacity(getTextOpacity(layerStyle, feature, details.mapZoom, details.vpZoom));
    const int outlineSize = layerStyle.m_textHaloWidth.toInt();
    QColor outlineColor = layerStyle.m_textHaloColor.value<QColor>();
    //Text is always antialised (otherwise it does not look good)
    painter.setRenderHints(QPainter::Antialiasing, true);

    //Get the corrected version of the text.
    //This means that text is split up for text wrapping depending on if it exceeds the maximum allowed width.
    QList<QString> correctedText = getCorrectedText(textToDraw, textFont, layerStyle.m_textMaxWidth.toInt());

    //Get the coordinates for the text rendering
    const QPoint &coordinates = feature.points().at(0);
    QTransform transform = {};
    transform.scale(1 / 4096.0, 1 / 4096.0);
    transform.scale(tileSize, tileSize);
    //Remap the original coordinates so that they are positioned correctly.
    const QPoint newCoordinates = transform.map(coordinates);

    //The text is rendered differently depending on it it wraps or not.
    if(correctedText.size() == 1) //In case there is only one string to be rendered (no wrapping)
        paintSimpleText(
            correctedText.at(0),
            newCoordinates,
            outlineSize,
            outlineColor,
            textFont,
            rects,
            painter,
            feature,
            layerStyle,
            details.mapZoom,
            details.vpZoom);
    else{ //In case there are multiple strings to be redered (text wrapping)
        paintCompositeText(
            correctedText,
            newCoordinates,
            outlineSize,
            outlineColor,
            textFont,
            rects,
            painter,
            feature,
            layerStyle,
            details.mapZoom,
            details.vpZoom);
    }
}

