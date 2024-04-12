#include "Evaluator.h"
#include "Rendering.h"


/*!
 * \brief getTextColor
 * Get the QVariant of the color from the layerStyle and resolve and return it if its an expression,
 * or return it as a QColor otherwise
 * \param layerStyle the layerStyle containing the color variable
 * \param feature the feature to be used in case the QVariant is an expression
 * \param mapZoom the map zoom level to be used in case the QVariant is an expression
 * \param vpZoom the viewport zoom level to be used in case the QVariant is an expression
 * \return the QColor to be used to render the text
 */
static QColor getTextColor(
    const SymbolLayerStyle &layerStyle,
    const AbstractLayerFeature &feature,
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


/*!
 * \brief getTextSize
 * Get the QVariant of the size from the layerStyle and resolve and return it if its an expression,
 * or return it as an int otherwise
 * \param layerStyle the layerStyle containing the size variable
 * \param feature the feature to be used in case the QVariant is an expression
 * \param mapZoom the map zoom level to be used in case the QVariant is an expression
 * \param vpZoom the viewport zoom level to be used in case the QVariant is an expression
 * \return and int for the size to be used to render the text
 */
static int getTextSize(
    const SymbolLayerStyle &layerStyle,
    const AbstractLayerFeature &feature,
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


/*!
 * \brief getTextOpacity
 * Get the QVariant of the opacity from the layerStyle and resolve and return it if its an expression,
 * or return it as a QColor otherwise
 * \param layerStyle the layerStyle containing the opacity variable
 * \param feature the feature to be used in case the QVariant is an expression
 * \param mapZoom the map zoom level to be used in case the QVariant is an expression
 * \param vpZoom the viewport zoom level to be used in case the QVariant is an expression
 * \return a float for the opacity to be used to render the text
 */
static float getTextOpacity(
    const SymbolLayerStyle &layerStyle,
    const AbstractLayerFeature &feature,
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



/*!
 * \brief getTextContent
 * Get the String of the text to be redered.
 * \param layerStyle the layerStyle containing the color variable
 * \param feature the feature to be used in case the QVariant is an expression
 * \param mapZoom the map zoom level to be used in case the QVariant is an expression
 * \param vpZoom the viewport zoom level to be used in case the QVariant is an expression
 * \return a QString with the content of the text to be rendered
 */
static QString getTextContent(
    const SymbolLayerStyle &layerStyle,
    const AbstractLayerFeature &feature,
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



/*!
 * \brief isOverlapping
 * Checks if the passed QRect intersects any QRects in the passed list. This is used to eliminate text
 * overlap in the map.
 * \param textRect the bounding rect of the current text.
 * \param rectList A vector containing all the bounding rects of all the texts previously rendered.
 * \return True if the text overlaps, or false otherwise.
 */
static bool isOverlapping(const QRect &textRect, const QVector<QRect> &rectList){
    for(const auto& rect : rectList){
        if(rect.intersects(textRect)) return true;
    }
    return false;
}


/* Splits text to multiple strings depending on the text length and the maximum allowed rect width
 */
/*!
 * \brief getCorrectedText
 * This functions will split the text into a list of string depending on if the text fits into the
 * passed rectangle width or not. The number of strings that the text is diveded to is dependent on
 * how much space the text will take with the provided font conpared to the allowed width of the rectangle.
 * \param text The text to be checked and split.
 * \param font the font to be used in the calculation.
 * \param rectWidth the maximum width allowed for the text rectangle.
 * \return a QList containing 1 QString if the text fits in the rectable of n > 1 QStrings if it does not.
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



/*!
 * \brief processSimpleText
 * This function renders text that fits in one line and does not require wrapping.
 * \param text the text to be rendered
 * \param coordinate the coordinates where the text should be rendered. The text is rendered so that the point is right in the middle of the bounding rect of the text.
 * \param outlineSize the width of the text outline.
 * \param outlineColor the color of the text outline
 * \param textFont the text font
 * \param rects the QList of previously rendered texts to be used to check for overlapping.
 * \param painter The painter object to paint into.
 * \param feature the text feature
 * \param layerStyle the layerStyle to style the text
 * \param mapZoom
 * \param vpZoom
 */
static void processSimpleText(
    const QString &text,
    const QPoint &coordinate,
    const int outlineSize,
    const QColor &outlineColor,
    const QFont &textFont,
    QVector<QRect> &rects,
    const PointFeature &feature,
    const SymbolLayerStyle &layerStyle,
    const int mapZoom,
    const double vpZoom,
    int tileOriginX,
    int tileOriginY,
    QVector<Bach::vpGlobalText> &vpTextList)
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
    QRect globalRect(QPoint(tileOriginX + coordinate.x() - boundingRect.width()/2, tileOriginY + coordinate.y() - boundingRect.height()/2),
                     QSize(boundingRect.width(), boundingRect.height()));
    if(isOverlapping(globalRect, rects)) return;
    //Add the total bouding rect to the list of the text rects to check for overlap for upcoming text.
    rects.append(globalRect);
    vpTextList.append({QPoint(tileOriginX, tileOriginY), {textPath}, getTextColor(layerStyle, feature, mapZoom, vpZoom), outlineSize, outlineColor});
    //Draw  the text.
    //painter.strokePath(textPath, outlinePen);
    //painter.fillPath(textPath, getTextColor(layerStyle, feature, mapZoom, vpZoom));
}


/* Render a text that should be drawn on multiple lines.
 */
/*!
 * \brief processCompositeText
 * This function renders text that requires myltiple lines
 * \param texts the List containing the strings of text to be rendered, each on a separate line.
 * \param coordinates the coordinates where the text should be rendered. The text is rendered so that the point is right in the middle of the union of all the bounding rects of the text strings.
 * \param outlineSize the width of the text outline.
 * \param outlineColor the color of the text outline
 * \param textFont the text font
 * \param rects the QList of previously rendered texts to be used to check for overlapping.
 * \param painter The painter object to paint into.
 * \param feature the text feature
 * \param layerStyle the layerStyle to style the text
 * \param mapZoom
 * \param vpZoom
 */
static void processCompositeText(
    const QList<QString> &texts,
    const QPoint &coordinates,
    const int outlineSize,
    const QColor &outlineColor,
    const QFont &textFont,
    QVector<QRect> &rects,
    const PointFeature &feature,
    const SymbolLayerStyle &layerStyle,
    const int mapZoom,
    const double vpZoom,
    int tileOriginX,
    int tileOriginY,
    QVector<Bach::vpGlobalText> &vpTextList)
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
     QRect globalRect(QPoint((tileOriginX + coordinates.x()) - boundingRect.width()/2, (tileOriginY + coordinates.y() - boundingRect.height()/2)),
                     QSize(boundingRect.width(), boundingRect.height()));
    if(isOverlapping(globalRect, rects)) return;
    //Add the total bouding rect to the list of the text rects to check for overlap for upcoming text.
    rects.append(globalRect);
    //Draw all the text parts
    QList<QPainterPath> pathsList;
    for(const auto &path : paths){
        pathsList.append(path);
    }
    vpTextList.append({QPoint(tileOriginX, tileOriginY), pathsList, getTextColor(layerStyle, feature, mapZoom, vpZoom), outlineSize, outlineColor});
}

/*!
 * \brief Bach::paintSingleTileFeature_Point
 * This function is called in the tile rendering loop. It is responsible for processing the feture and layerstyle, and passing the text to be rendered
 * along with the styling information to one of the two rendering functions above depending if the text is a one liner or if it requires multiple lines.
 * \param details the struct containig all the elemets needed to paint the feature includeing the layerStyle and the feature itself.
 * \param tileSize the size of the current tile in pixels, used to scale the the transform.
 *
 * \param forceNoChangeFontType If set to true, the text font
 * rendered will be the one currently set by the QPainter object.
 * If set to false, it will try to use the font suggested by the stylesheet.
 *
 * \param rects the list of rectangles used to check for text overlapping.
 */
void Bach::processSingleTileFeature_Point(
    PaintingDetailsPoint details,
    const int tileSize,
    int tileOriginX,
    int tileOriginY,
    bool forceNoChangeFontType,
    QVector<QRect> &rects,
    QVector<vpGlobalText> &vpTextList)
{
    QPainter &painter = *details.painter;
    const SymbolLayerStyle &layerStyle = *details.layerStyle;
    const PointFeature &feature = *details.feature;
    //Get the text to be rendered.
    QString textToDraw = getTextContent(layerStyle, feature, details.mapZoom, details.vpZoom);
    //If there is no text then there is nothing to render, we return
    if(textToDraw == "") return;
    painter.setClipping(false);

    //Get the rendering parameters from the layerstyle and set the relevant painter field.
    painter.setBrush(Qt::NoBrush);
    int textSize = getTextSize(layerStyle, feature, details.mapZoom, details.vpZoom);

    QFont textFont;
    if (forceNoChangeFontType) {
        textFont = painter.font();
    } else {
        textFont = QFont(layerStyle.m_textFont);
    }

    textFont.setPixelSize(textSize);
    painter.setOpacity(getTextOpacity(layerStyle, feature, details.mapZoom, details.vpZoom));
    const int outlineSize = layerStyle.m_textHaloWidth.toInt();
    QColor outlineColor = layerStyle.m_textHaloColor.value<QColor>();
    //Text is always antialised (otherwise it does not look good)
    painter.setRenderHints(QPainter::Antialiasing, true);

    //Get the corrected version of the text.
    //This means that text is split up for text wrapping depending on if it exceeds the maximum allowed width.
    QList<QString> correctedText = getCorrectedText(textToDraw, textFont, layerStyle.m_textMaxWidth.toInt());

    // Get the coordinates for the text rendering
    // We don't actually know why
    // but when there are 3 points inside the text feature,
    // only index 1 contains the one we expect.
    QPoint coordinates;
    if (feature.points().length() > 1) {
        coordinates = feature.points().at(1);
    } else {
        coordinates = feature.points().at(0);
    }
    QTransform transform = {};
    transform.scale(1 / 4096.0, 1 / 4096.0);
    transform.scale(tileSize, tileSize);
    //Remap the original coordinates so that they are positioned correctly.
    const QPoint newCoordinates = transform.map(coordinates);
    if(newCoordinates.x() < 0 || newCoordinates.x() > tileSize || newCoordinates.y() < 0 || newCoordinates.y() > tileSize){
        painter.setClipping(true);
        return;
    }

    //The text is rendered differently depending on it it wraps or not.
    if(correctedText.size() == 1) //In case there is only one string to be rendered (no wrapping)
        processSimpleText(
            correctedText.at(0),
            newCoordinates,
            outlineSize,
            outlineColor,
            textFont,
            rects,
            feature,
            layerStyle,
            details.mapZoom,
            details.vpZoom,
            tileOriginX,
            tileOriginY,
            vpTextList);
    else{ //In case there are multiple strings to be redered (text wrapping)
        processCompositeText(
            correctedText,
            newCoordinates,
            outlineSize,
            outlineColor,
            textFont,
            rects,
            feature,
            layerStyle,
            details.mapZoom,
            details.vpZoom,
            tileOriginX,
            tileOriginY,
            vpTextList);
    }

    painter.setClipping(true);
}

/*!
 * \brief correctAngle
 * correct the Angle for the road names so that the text rotation angle is always between 270 and 90 (through 360)
 * \param angle the angle to be corrected
 * \return the corrected angle
 */
static double correctAngle(double angle){
    if(angle < 270 && angle > 90){
        angle += 180;
    }
    return angle;
}

void Bach::paintSingleTileFeature_Point_Curved(PaintingDetailsPointCurved details)
{
    QPainter &painter = *details.painter;
    const SymbolLayerStyle &layerStyle = *details.layerStyle;
    const LineFeature &feature = *details.feature;
    //Get the text to be rendered.
    QString textToDraw = getTextContent(layerStyle, feature, details.mapZoom, details.vpZoom).toUpper();
    //If there is no text then there is nothing to render, we return
    if(textToDraw == "") return;

    //Get the rendering parameters from the layerstyle and set the relevant painter field.
    painter.setBrush(Qt::NoBrush);
    int textSize = getTextSize(layerStyle, feature, details.mapZoom, details.vpZoom);
    QFont textFont = QFont(layerStyle.m_textFont);
    textFont.setPixelSize(textSize);
    painter.setOpacity(getTextOpacity(layerStyle, feature, details.mapZoom, details.vpZoom));
    //Text is always antialised (otherwise it does not look good)
    painter.setRenderHints(QPainter::Antialiasing, true);

    //Get the coordinates for the text rendering
    QTransform transform = details.transformIn;
    transform.scale(1 / 4096.0, 1 / 4096.0);
    const QPainterPath &path = transform.map(feature.line());

    //transform.scale(tileSize, tileSize);
    //Remap the original coordinates so that they are positioned correctly.
    QFontMetrics fMetrics(textFont);
    //double letterSpacing = layerStyle.m_textLetterSpacing.toDouble() * textFont.pixelSize();
    qreal length = 10;
    qreal percentage = path.percentAtLength(length);
    qreal angle;
    QPointF charPosition;
    for(int i = 0; i < textToDraw.size(); i++){
        charPosition = path.pointAtPercent(percentage);
        angle = path.angleAtPercent(percentage);
        painter.save();
        painter.translate(charPosition);
        painter.rotate(-correctAngle(angle));
        painter.drawText(QRectF(0, -100, 100, 200), textToDraw.at(i), Qt::AlignLeft | Qt::AlignVCenter);
        painter.restore();
        length = length + fMetrics.horizontalAdvance(textToDraw.at(i)) + 2;
        percentage = path.percentAtLength(length);
    }
}


