#include "Rendering.h"

#include "Evaluator.h"

QPair<double, double> Bach::lonLatToWorldNormCoord(double lon, double lat)
{
    constexpr double webMercatorPhiCutoff = 1.4844222297;

    // Function to normalize a value from its original range to [0, 1]
    auto normalize = [](double value, double min, double max) {
        return (value - min) / (max - min);
    };

    // Convert longitude and latitude to radians
    auto lambda = lon;
    auto phi = lat;

    // Convert to Web Mercator
    auto x = lambda;
    auto y = std::log(std::tan(M_PI / 4.0 + phi / 2.0));

    // Normalize x and y to [0, 1]
    // Assuming the Web Mercator x range is [-π, π] and y range is calculated from latitude range
    auto xNormalized = normalize(x, -M_PI, M_PI);
    // We have to flip the sign of Y, because Mercator has positive Y moving up,
    // while the world-normalized coordinate space has Y moving down.
    auto yNormalized = normalize(
        -y,
        std::log(std::tan(M_PI / 4.0 + -webMercatorPhiCutoff / 2.0)),
        std::log(std::tan(M_PI / 4.0 + webMercatorPhiCutoff / 2.0)));


    return { xNormalized, yNormalized };
}

QPair<double, double> Bach::lonLatToWorldNormCoordDegrees(double lon, double lat)
{
    auto degToRad = [](double deg) {
        return deg * M_PI / 180.0;
    };
    return lonLatToWorldNormCoord(degToRad(lon), degToRad(lat));
}

int Bach::calcMapZoomLevelForTileSizePixels(
    int vpWidth,
    int vpHeight,
    double vpZoom,
    int desiredTileWidth)
{
    // Calculate current tile size based on the largest dimension and current scale
    int currentTileSize = qMax(vpWidth, vpHeight);

    // Calculate desired scale factor
    double desiredScale = (double)desiredTileWidth / currentTileSize;

    // Figure out how the difference between the zoom levels of viewport and map
    // needed to satisfy the pixel-size requirement.
    double newMapZoomLevel = vpZoom - log2(desiredScale);

    // Round to int, and clamp output to zoom level range.
    return std::clamp((int)round(newMapZoomLevel), 0, maxZoomLevel);
}

QPair<double, double> Bach::calcViewportSizeNorm(double vpZoomLevel, double viewportAspect) {
    // Math formula can be seen in the figure in the report, with the caption
    // "Calculating viewport size as a factor of the world map"
    auto temp = 1 / pow(2, vpZoomLevel);
    return {
        temp * qMin(1.0, viewportAspect),
        temp * qMin(1.0, 1 / viewportAspect)
    };
}

QVector<TileCoord> Bach::calcVisibleTiles(
    double vpX,
    double vpY,
    double vpAspect,
    double vpZoomLevel,
    int mapZoomLevel)
{
    mapZoomLevel = qMax(0, mapZoomLevel);

    // We need to calculate the width and height of the viewport in terms of
    // world-normalized coordinates.
    auto [vpWidthNorm, vpHeightNorm] = calcViewportSizeNorm(vpZoomLevel, vpAspect);

    // Figure out the 4 edges in world-normalized coordinate space.
    auto vpMinNormX = vpX - (vpWidthNorm / 2.0);
    auto vpMaxNormX = vpX + (vpWidthNorm / 2.0);
    auto vpMinNormY = vpY - (vpHeightNorm / 2.0);
    auto vpMaxNormY = vpY + (vpHeightNorm / 2.0);

    // Amount of tiles in each direction for this map zoom level.
    auto tileCount = 1 << mapZoomLevel;

    auto clampToGrid = [&](int i) {
        return std::clamp(i, 0, tileCount-1);
    };

    // Convert edges into the index-based grid coordinates, and apply a clamp operation
    // in case the viewport goes outside the map.
    auto leftTileX = clampToGrid((int)floor(vpMinNormX * tileCount));
    auto rightTileX = clampToGrid((int)floor(vpMaxNormX * tileCount));
    auto topTileY = clampToGrid((int)floor(vpMinNormY * tileCount));
    auto botTileY = clampToGrid((int)floor(vpMaxNormY * tileCount));

    // Iterate over our two ranges to build our list.

    if (mapZoomLevel == 0 &&
        rightTileX - leftTileX == 0 &&
        botTileY - topTileY == 0)
    {
        return { { 0, 0, 0 } };
    } else {
        QVector<TileCoord> visibleTiles;
        for (int y = topTileY; y <= botTileY; y++) {
            for (int x = leftTileX; x <= rightTileX; x++) {
                visibleTiles += { mapZoomLevel, x, y };
            }
        }
        return visibleTiles;
    }
}

/* This is a helper function for visualizing the boundaries of each tile.
 *
 * It is responsible for drawing green borders around each tile.
 *
 * This function assumes the painter's transform has moved the origin of the
 * canvas to the tile's origin, and has not been scaled.
 *
 * The pen width needs to be set appropriately before using this function.
 *
 * The 'scale' parameter is a scalar that geometry should be multiplied
 * with such that coordinates in the range [0, 1] become correctly mapped
 * inside tile.
 */
static void paintSingleTileDebug(
    QPainter &painter,
    const TileCoord &tileCoord,
    double scale)
{
    painter.setPen(Qt::green);

    QTransform transform;
    transform.scale(scale, scale);

    // Draw the X in the middle of the tile first, by using tile-normalized coordinates
    // from 0.45 to 0.55.
    painter.drawLine(transform.map(QLineF{ QPointF(0.45, 0.45), QPointF(0.55, 0.55) }));
    painter.drawLine(transform.map(QLineF{ QPointF(0.55, 0.45), QPointF(0.45, 0.55) }));

    // Then draw the boundary of the tile.
    // We do a save and restore because we need to set the
    // brush to not draw anything.
    {
        painter.save();
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(transform.mapRect(QRectF(0, 0, 1, 1)));
        painter.restore();
    }

    // Draw the text that shows the coordinates of the tile itself.
    {
        // Text rendering has issues if our coordinate system is [0, 1].
        // So we reset the coordinate system back to unscaled and
        // just offset the text to where we need it.
        painter.drawText(10, 30, tileCoord.toString());
    }
}

/* Finds the fill color of given feature at the given zoom level.
 *
 * This function also takes opacity into account.
 */
static QColor getFillColor(
    const FillLayerStyle &layerStyle,
    const AbstractLayerFeature &feature,
    int mapZoom,
    double vpZoom)
{
    QVariant colorVariant = layerStyle.getFillColorAtZoom(mapZoom);
    QColor color;
    // The layer style might return an expression, we need to resolve it.
    if(colorVariant.typeId() == QMetaType::Type::QJsonArray){
        color = Evaluator::resolveExpression(
            colorVariant.toJsonArray(),
            &feature,
            mapZoom,
            vpZoom).value<QColor>();
    } else {
        color = colorVariant.value<QColor>();
    }

    QVariant fillOpacityVariant = layerStyle.getFillOpacityAtZoom(mapZoom);
    float fillOpacity;
    // The layer style might return an expression, we need to resolve it.
    if (fillOpacityVariant.typeId() == QMetaType::Type::QJsonArray){
        fillOpacity = Evaluator::resolveExpression(
          fillOpacityVariant.toJsonArray(),
          &feature,
          mapZoom,
          vpZoom).value<float>();
    } else {
        fillOpacity = fillOpacityVariant.value<float>();
    }

    color.setAlphaF(fillOpacity * color.alphaF());
    return color;
}

/* Render a single feature of a tile, where the feature is of type polygon.
 *
 * Assumes the painter's position has been moved to the origin of the tile.
 *
 */
static void paintSingleTileFeature_Fill_Polygon(
    QPainter &painter,
    const PolygonFeature &feature,
    const FillLayerStyle &layerStyle,
    int mapZoom,
    double vpZoom,
    const QTransform &transformIn)
{
    auto brushColor = getFillColor(layerStyle, feature, mapZoom, vpZoom);

    painter.setBrush(brushColor);
    painter.setRenderHints(QPainter::Antialiasing, layerStyle.m_antialias);
    painter.setPen(Qt::NoPen);

    auto const& path = feature.polygon();

    QTransform transform = transformIn;
    transform.scale(1 / 4096.0, 1 / 4096.0);
    auto newPath = transform.map(path);

    painter.drawPath(newPath);
}

/* Finds the line color of given feature at the given zoom level.
 */
static QColor getLineColor(
    const LineLayerStyle &layerStyle,
    const LineFeature &feature,
    int mapZoom,
    double vpZoom)
{
    QVariant color = layerStyle.getLineColorAtZoom(mapZoom);
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

/* Finds the line opacity of given feature at the given zoom level.
 */
static float getLineOpacity(
    const LineLayerStyle &layerStyle,
    const LineFeature &feature,
    int mapZoom,
    double vpZoom)
{
    QVariant lineOpacity = layerStyle.getLineOpacityAtZoom(mapZoom);
    // The layer style might return an expression, we need to resolve it.
    if(lineOpacity.typeId() == QMetaType::Type::QJsonArray){
        lineOpacity = Evaluator::resolveExpression(
            lineOpacity.toJsonArray(),
            &feature,
            mapZoom,
            vpZoom);
    }
    return lineOpacity.value<float>();
}

/* Finds the line width of this feature at given zoom level.
 */
static int getLineWidth(
    const LineLayerStyle &layerStyle,
    const LineFeature &feature,
    int mapZoom,
    double vpZoom)
{
    QVariant lineWidth = layerStyle.getLineWidthAtZoom(mapZoom);
    // The layer style might return an expression, we need to resolve it.
    if(lineWidth.typeId() == QMetaType::Type::QJsonArray){
        lineWidth = Evaluator::resolveExpression(
            lineWidth.toJsonArray(),
            &feature,
            mapZoom,
            vpZoom);
    }
    return lineWidth.value<int>();
}


/* Paints a single Line feature within a tile.
 *
 * Assumes the painters origin has moved to the tiles origin.
 */
static void paintSingleTileFeature_Line(
    QPainter &painter,
    const LineFeature &feature,
    const LineLayerStyle &layerStyle,
    int mapZoom,
    double vpZoom,
    const QTransform &transformIn)
{
    auto pen = painter.pen();

    pen.setColor(getLineColor(layerStyle, feature, mapZoom, vpZoom));
    // Not sure how to take opacity into account yet.
    // There's some interaction happening with the color.
    //painter.setOpacity(getLineOpacity(layerStyle, feature, mapZoom, vpZoom));

    pen.setWidth(getLineWidth(layerStyle, feature, mapZoom, vpZoom));
    pen.setCapStyle(layerStyle.getCapStyle());
    pen.setJoinStyle(layerStyle.getJoinStyle());

    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    // Not sure yet how to determine AA for lines.
    painter.setRenderHints(QPainter::Antialiasing, false);

    auto const& path = feature.line();
    QTransform transform = transformIn;
    transform.scale(1 / 4096.0, 1 / 4096.0);
    auto newPath = transform.map(path);

    painter.drawPath(newPath);
}

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


static QString getTextContent(
    const SymbolLayerStyle &layerStyle,
    const PointFeature &feature,
    int mapZoom,
    double vpZoom)
{
    QVariant text = layerStyle.m_textField;
    // The layer style might return an expression, we need to resolve it.
    if(text.typeId() == QMetaType::Type::QJsonArray){
        text = Evaluator::resolveExpression(
            text.toJsonArray(),
            &feature,
            mapZoom,
            vpZoom);
    }else if(text.typeId() != QMetaType::Type::QString)
    {
        text = "";
    }
    return text.value<QString>();
}


static void paintSingleTileFeature_Point(
    QPainter &painter,
    const PointFeature &feature,
    const SymbolLayerStyle &layerStyle,
    int mapZoom,
    double vpZoom,
    const QTransform &transformIn,
    int tileSize)
{

    //if there is no text to draw, exit the function
    QString text = getTextContent(layerStyle, feature, mapZoom, vpZoom);
    if(text == "") return;
    text.remove("{");
    text.remove("}");
    QString textToDraw;
    if(feature.fetureMetaData.contains(text)){
         textToDraw = feature.fetureMetaData[text].value<QString>();
    }else{
        return;
    }
    auto pen = painter.pen();

    pen.setColor(getTextColor(layerStyle, feature, mapZoom, vpZoom));

    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    QRect boundingRect =  painter.fontMetrics().boundingRect(textToDraw);


    int textSize = getTextSize(layerStyle, feature, mapZoom, vpZoom);
    if(tileSize < 12) return;
    QFont textFont = QFont(layerStyle.m_textFont, textSize);
    painter.setFont(textFont);
    painter.setOpacity(getTextOpacity(layerStyle, feature, mapZoom, vpZoom));

    painter.setRenderHints(QPainter::Antialiasing, false);



    auto const& coordinates = feature.points().at(0);
    QTransform transform = {};
    transform.scale(1 / 4096.0, 1 / 4096.0);
    transform.scale(tileSize, tileSize);
    auto  newCoordinates = transform.map(coordinates);

    auto actualNewCoordinates = QPoint(newCoordinates.x() - boundingRect.width()/2, newCoordinates.y() + boundingRect.height()/4);

    painter.drawText(actualNewCoordinates,textToDraw);
}

// Determines whether this layer is hidden.
static bool isLayerHidden(const AbstractLayereStyle &layerStyle, int mapZoom)
{
    return
        layerStyle.m_visibility == "none" ||
        layerStyle.m_maxZoom < mapZoom ||
        layerStyle.m_minZoom >= mapZoom;
}

// Determines whether a feature should be included when rendering this layer-style.
static bool includeFeature(
    const AbstractLayereStyle &layerStyle,
    const AbstractLayerFeature &feature,
    int mapZoom,
    double vpZoom)
{
    return !Evaluator::resolveExpression(
        layerStyle.m_filter,
        &feature,
        mapZoom,
        vpZoom).toBool();
}

/* This function takes care of rendering a single tile.
 *
 * This is called repeatedly from the 'paintTiles' function.
 *
 * It assumes the painter object has had its origin moved to the tiles origin.
 *
 * This does not handle background color.
 */
static void paintSingleTile(
    const VectorTile &tileData,
    QPainter &painter,
    int mapZoom,
    double vpZoom,
    const StyleSheet &styleSheet,
    const QTransform &transformIn,
    int tileSize)
{
    // We start by iterating over each layer style, it determines the order
    // at which we draw the elements of the map.
    for (auto const& abstractLayerStyle : styleSheet.m_layerStyles) {
        if (isLayerHidden(*abstractLayerStyle, mapZoom))
            continue;

        // Check if this layer style has an associated layer in the tile.
        auto layerIt = tileData.m_layers.find(abstractLayerStyle->m_sourceLayer);
        if (layerIt == tileData.m_layers.end()) {
            continue;
        }
        // If we find it, we dereference it to access it's data.
        auto const& layer = **layerIt;

        // We do different types of rendering based on whether the layer is a polygon
        // or line.
        if (abstractLayerStyle->type() == AbstractLayereStyle::LayerType::fill) {
            auto const& layerStyle = *static_cast<FillLayerStyle const*>(abstractLayerStyle);

            // Iterate over all the features, and filter out anything that is not fill.
            for (auto const& abstractFeature : layer.m_features) {
                if (abstractFeature->type() != AbstractLayerFeature::featureType::polygon)
                    continue;
                const auto& feature = *static_cast<const PolygonFeature*>(abstractFeature);

                // Tests whether the feature should be rendered at all based on possible expression.
                if (includeFeature(layerStyle, feature, mapZoom, vpZoom))
                    continue;

                // Render the feature in question.
                painter.save();
                paintSingleTileFeature_Fill_Polygon(
                    painter,
                    feature,
                    layerStyle,
                    mapZoom,
                    vpZoom,
                    transformIn);
                painter.restore();
            }
        } else if (abstractLayerStyle->type() == AbstractLayereStyle::LayerType::line) {
            auto const& layerStyle = *static_cast<LineLayerStyle const*>(abstractLayerStyle);

            // Iterate over all the features, and filter out anything that is not fill.
            for (auto const& abstractFeature : layer.m_features) {
                if (abstractFeature->type() != AbstractLayerFeature::featureType::line)
                    continue;
                const auto &feature = *static_cast<const LineFeature*>(abstractFeature);

                // Tests whether the feature should be rendered at all based on possible expression.
                if (includeFeature(layerStyle, feature, mapZoom, vpZoom))
                    continue;

                // Render the feature in question.
                painter.save();
                paintSingleTileFeature_Line(
                    painter,
                    feature,
                    layerStyle,
                    mapZoom,
                    vpZoom,
                    transformIn);
                painter.restore();
            }
        } else if(abstractLayerStyle->type() == AbstractLayereStyle::LayerType::symbol){
             auto const& layerStyle = *static_cast<const SymbolLayerStyle *>(abstractLayerStyle);

             // Iterate over all the features, and filter out anything that is not fill.
             for (auto const& abstractFeature : layer.m_features) {
                 if (abstractFeature->type() != AbstractLayerFeature::featureType::point)
                     continue;
                 const auto &feature = *static_cast<const PointFeature*>(abstractFeature);

                 // Tests whether the feature should be rendered at all based on possible expression.
                 if (includeFeature(layerStyle, feature, mapZoom, vpZoom))
                     continue;
                 if(feature.fetureMetaData.contains("name") && feature.fetureMetaData["name"] == "Australia")
                 {
                     auto x = 1;
                 }
                 // Render the feature in question.
                 painter.save();
                 paintSingleTileFeature_Point(
                     painter,
                     feature,
                     layerStyle,
                     mapZoom,
                     vpZoom,
                     transformIn,
                     tileSize);
                 painter.restore();
             }
        }
    }
}

// Assumes the painter is unchanged.
static void drawBackgroundColor(
    QPainter &painter,
    const StyleSheet &styleSheet,
    int mapZoom)
{
    bool styleFound = false;
    QColor color;

    // We start by iterating over each layer style, it determines the order
    // at which we draw the elements of the map.
    for (auto const& abstractLayerStyle : styleSheet.m_layerStyles) {
        // Background is a special case and has no associated layer.
        // We just draw it and move onto the next layer style.
        if (abstractLayerStyle->type() == AbstractLayereStyle::LayerType::background) {
            // Fill the entire tile with a single color
            const auto& layerStyle = *static_cast<const BackgroundStyle*>(abstractLayerStyle);

            color = layerStyle.getColorAtZoom(mapZoom).value<QColor>();
            styleFound = true;
            break;
        }
    }

    if (styleFound) {
        painter.fillRect(
            0,
            0,
            painter.window().width(),
            painter.window().height(),
            color);
    } else {
        qWarning() << "No background color found while drawing. Possible bug.\n";
    }
}

void Bach::paintTiles(
    QPainter &painter,
    double vpX,
    double vpY,
    double viewportZoomLevel,
    int mapZoomLevel,
    const QMap<TileCoord, const VectorTile*> &tileContainer,
    const StyleSheet &styleSheet,
    bool drawDebug)
{
    // Start by drawing the background color on the entire canvas.
    drawBackgroundColor(painter, styleSheet, mapZoomLevel);


    // Gather width and height of the viewport, in pixels..
    auto vpWidth = painter.window().width();
    auto vpHeight = painter.window().height();

    // Aspect ratio of the viewport.
    auto vpAspect = (double)vpWidth / (double)vpHeight;

    // The longest length between vpWidth and vpHeight
    auto vpMaxDim = qMax(vpWidth, vpHeight);

    // Helper function to turn coordinates that are expressed as fraction of the viewport,
    // into pixel coordinates.
    auto toPixelSpace = [&](double in) { return (int)round(in * vpMaxDim); };

    // Calculate the set of visible tiles that fit in the viewport.
    auto visibleTiles = calcVisibleTiles(
        vpX,
        vpY,
        vpAspect,
        viewportZoomLevel,
        mapZoomLevel);

    // The scale of the world map as a fraction of the viewport.
    auto vpScale = pow(2, viewportZoomLevel);
    // The scale of a single tile as a fraction of the viewport.
    auto tileSizeNorm = vpScale / (1 << mapZoomLevel);

    // Calculate where the top-left origin of the world map is relative to the viewport.
    double worldOriginX = vpX * vpScale - 0.5;
    double worldOriginY = vpY * vpScale - 0.5;
    // The world such that our worldmap is still centered around our center-coordinate
    // when the aspect ratio changes.
    if (vpAspect < 1.0) {
        worldOriginX += -0.5 * vpAspect + 0.5;
    } else if (vpAspect > 1.0) {
        worldOriginY += -0.5 / vpAspect + 0.5;
    }

    // Iterate over all possible tiles that can possibly fit in this viewport.
    for (const auto& tileCoord : visibleTiles) {
        // Position the top-left of the tile inside the world map, as a fraction of the viewport size.
        // We use the tile index coords and the size of a tile to generate the offset.
        auto posNormX = (tileCoord.x * tileSizeNorm) - worldOriginX;
        auto posNormY = (tileCoord.y * tileSizeNorm) - worldOriginY;

        // Convert tile position and size to pixels.
        auto tilePixelPosX = toPixelSpace(posNormX);
        auto tilePixelPosY = toPixelSpace(posNormY);
        auto tileSizePixels = toPixelSpace(tileSizeNorm);

        painter.save();

        // We move the origin point of the painter to the top-left of the tile.
        // We do not apply scaling because it interferes with other sized elements,
        // like the size of pen width.
        QTransform transform;
        transform.translate(tilePixelPosX, tilePixelPosY);
        painter.setTransform(transform);

        // Temporary. Should likely just be passed as a single scalar to tile-drawing function.
        QTransform geometryTransform;
        geometryTransform.scale(vpMaxDim * tileSizeNorm, vpMaxDim * tileSizeNorm);

        // See if the tile being rendered has any tile-data associated with it.
        auto tileIt = tileContainer.find(tileCoord);
        if (tileIt != tileContainer.end()) {
            auto const& tileData = **tileIt;
            if(tileCoord == TileCoord({2,2,2})){
                int i = 0;
            }
            painter.save();

            // We create a clip rect around our tile, as to only render into
            // the region on-screen the tile occupies.
            painter.setClipRect(
                0,
                0,
                tileSizePixels,
                tileSizePixels);

            // Run the rendering function for a single tile.
            paintSingleTile(
                tileData,
                painter,
                mapZoomLevel,
                viewportZoomLevel,
                styleSheet,
                geometryTransform,
                tileSizePixels);

            painter.restore();
        }

        // Paint debug lines around the tile.
        if (drawDebug) {
            paintSingleTileDebug(
                painter,
                tileCoord,
                vpMaxDim * tileSizeNorm);
        }

        painter.restore();
    }
}
