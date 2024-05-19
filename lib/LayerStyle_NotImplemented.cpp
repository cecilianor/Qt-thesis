// Copyright (c) 2024 Cecilia Norevik Bratlie, Nils Petter Skålerud, Eimen Oueslati
// SPDX-License-Identifier: MIT

#include <QRegularExpression>
#include <QtMath>

#include "LayerStyle.h"

/*!
 * \brief NotImplementedStyle::fromJson returns the NotImplementedStyle
 * layer style.
 *
 * This is used for any layer with type value other than "background",
 * "fill", "line", or "symbol".
 *
 * \param jsonObj is a JSonObject. It's passed here to match Layer style types.
 * \return the NotImplementedStyle.
 */
std::unique_ptr<NotImplementedStyle> NotImplementedStyle::fromJson(const QJsonObject &json)
{
    std::unique_ptr<NotImplementedStyle> returnLayerPtr = std::make_unique<NotImplementedStyle>();
    return returnLayerPtr;
}
