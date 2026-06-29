#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Theme.h>

#include <QtGui/QColor>

namespace FluentQt {

enum class FluentThemeColor
{
    YellowGold,
    Gold,
    OrangeBright,
    OrangeDark,
    Rust,
    PaleRust,
    BrickRed,
    ModRed,
    PaleRed,
    Red,
    RoseBright,
    Rose,
    PlumLight,
    Plum,
    OrchidLight,
    Orchid,
    DefaultBlue,
    NavyBlue,
    PurpleShadow,
    PurpleShadowDark,
    IrisPastel,
    IrisSpring,
    VioletRedLight,
    VioletRed,
    CoolBlueBright,
    CoolBlue,
    Seafoam,
    SeafoamTeal,
    MintLight,
    MintDark,
    TurfGreen,
    SportGreen,
    Gray,
    GrayBrown,
    SteelBlue,
    MetalBlue,
    PaleMoss,
    Moss,
    MeadowGreen,
    Green,
    Overcast,
    Storm,
    BlueGray,
    GrayDark,
    LiddyGreen,
    Sage,
    CamouflageDesert,
    Camouflage
};

enum class FluentSystemColor
{
    SuccessForeground,
    CautionForeground,
    CriticalForeground,
    SuccessBackground,
    CautionBackground,
    CriticalBackground
};

enum class ThemeColor
{
    Primary,
    Dark1,
    Dark2,
    Dark3,
    Light1,
    Light2,
    Light3
};

FQW_API QColor color(FluentThemeColor color);
FQW_API QColor color(FluentSystemColor color, Theme theme = Theme::Auto);
FQW_API QColor themeColor();
FQW_API QColor validColor(const QColor &color, const QColor &defaultColor);
FQW_API QColor fallbackThemeColor(const QColor &color);
FQW_API QColor autoFallbackThemeColor(const QColor &light, const QColor &dark);
FQW_API QColor derivedThemeColor(const QColor &base, int lightnessDelta, int saturationDelta = 0);
FQW_API QColor derivedThemeColor(const QColor &accent, ThemeColor type, Theme theme);

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::FluentThemeColor)
Q_DECLARE_METATYPE(FluentQt::FluentSystemColor)
Q_DECLARE_METATYPE(FluentQt::ThemeColor)
