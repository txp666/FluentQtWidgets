#include <FluentQtWidgets/Color.h>

#include <FluentQtWidgets/Theme.h>

namespace FluentQt {

QColor color(FluentThemeColor color)
{
    switch (color) {
    case FluentThemeColor::YellowGold:
        return QColor(QStringLiteral("#FFB900"));
    case FluentThemeColor::Gold:
        return QColor(QStringLiteral("#FF8C00"));
    case FluentThemeColor::OrangeBright:
        return QColor(QStringLiteral("#F7630C"));
    case FluentThemeColor::OrangeDark:
        return QColor(QStringLiteral("#CA5010"));
    case FluentThemeColor::Rust:
        return QColor(QStringLiteral("#DA3B01"));
    case FluentThemeColor::PaleRust:
        return QColor(QStringLiteral("#EF6950"));
    case FluentThemeColor::BrickRed:
        return QColor(QStringLiteral("#D13438"));
    case FluentThemeColor::ModRed:
        return QColor(QStringLiteral("#FF4343"));
    case FluentThemeColor::PaleRed:
        return QColor(QStringLiteral("#E74856"));
    case FluentThemeColor::Red:
        return QColor(QStringLiteral("#E81123"));
    case FluentThemeColor::RoseBright:
        return QColor(QStringLiteral("#EA005E"));
    case FluentThemeColor::Rose:
        return QColor(QStringLiteral("#C30052"));
    case FluentThemeColor::PlumLight:
        return QColor(QStringLiteral("#E3008C"));
    case FluentThemeColor::Plum:
        return QColor(QStringLiteral("#BF0077"));
    case FluentThemeColor::OrchidLight:
        return QColor(QStringLiteral("#BF0077"));
    case FluentThemeColor::Orchid:
        return QColor(QStringLiteral("#9A0089"));
    case FluentThemeColor::DefaultBlue:
        return QColor(QStringLiteral("#0078D7"));
    case FluentThemeColor::NavyBlue:
        return QColor(QStringLiteral("#0063B1"));
    case FluentThemeColor::PurpleShadow:
        return QColor(QStringLiteral("#8E8CD8"));
    case FluentThemeColor::PurpleShadowDark:
        return QColor(QStringLiteral("#6B69D6"));
    case FluentThemeColor::IrisPastel:
        return QColor(QStringLiteral("#8764B8"));
    case FluentThemeColor::IrisSpring:
        return QColor(QStringLiteral("#744DA9"));
    case FluentThemeColor::VioletRedLight:
        return QColor(QStringLiteral("#B146C2"));
    case FluentThemeColor::VioletRed:
        return QColor(QStringLiteral("#881798"));
    case FluentThemeColor::CoolBlueBright:
        return QColor(QStringLiteral("#0099BC"));
    case FluentThemeColor::CoolBlue:
        return QColor(QStringLiteral("#2D7D9A"));
    case FluentThemeColor::Seafoam:
        return QColor(QStringLiteral("#00B7C3"));
    case FluentThemeColor::SeafoamTeal:
        return QColor(QStringLiteral("#038387"));
    case FluentThemeColor::MintLight:
        return QColor(QStringLiteral("#00B294"));
    case FluentThemeColor::MintDark:
        return QColor(QStringLiteral("#018574"));
    case FluentThemeColor::TurfGreen:
        return QColor(QStringLiteral("#00CC6A"));
    case FluentThemeColor::SportGreen:
        return QColor(QStringLiteral("#10893E"));
    case FluentThemeColor::Gray:
        return QColor(QStringLiteral("#7A7574"));
    case FluentThemeColor::GrayBrown:
        return QColor(QStringLiteral("#5D5A58"));
    case FluentThemeColor::SteelBlue:
        return QColor(QStringLiteral("#68768A"));
    case FluentThemeColor::MetalBlue:
        return QColor(QStringLiteral("#515C6B"));
    case FluentThemeColor::PaleMoss:
        return QColor(QStringLiteral("#567C73"));
    case FluentThemeColor::Moss:
        return QColor(QStringLiteral("#486860"));
    case FluentThemeColor::MeadowGreen:
        return QColor(QStringLiteral("#498205"));
    case FluentThemeColor::Green:
        return QColor(QStringLiteral("#107C10"));
    case FluentThemeColor::Overcast:
        return QColor(QStringLiteral("#767676"));
    case FluentThemeColor::Storm:
        return QColor(QStringLiteral("#4C4A48"));
    case FluentThemeColor::BlueGray:
        return QColor(QStringLiteral("#69797E"));
    case FluentThemeColor::GrayDark:
        return QColor(QStringLiteral("#4A5459"));
    case FluentThemeColor::LiddyGreen:
        return QColor(QStringLiteral("#647C64"));
    case FluentThemeColor::Sage:
        return QColor(QStringLiteral("#525E54"));
    case FluentThemeColor::CamouflageDesert:
        return QColor(QStringLiteral("#847545"));
    case FluentThemeColor::Camouflage:
        return QColor(QStringLiteral("#7E735F"));
    }

    return QColor(QStringLiteral("#0078D7"));
}

QColor color(FluentSystemColor color, Theme theme)
{
    const Theme resolvedTheme = theme == Theme::Auto ? ThemeManager::instance()->effectiveTheme() : theme;
    const bool dark = resolvedTheme == Theme::Dark;

    switch (color) {
    case FluentSystemColor::SuccessForeground:
        return QColor(dark ? QStringLiteral("#6ccb5f") : QStringLiteral("#0f7b0f"));
    case FluentSystemColor::CautionForeground:
        return QColor(dark ? QStringLiteral("#fce100") : QStringLiteral("#9d5d00"));
    case FluentSystemColor::CriticalForeground:
        return QColor(dark ? QStringLiteral("#ff99a4") : QStringLiteral("#c42b1c"));
    case FluentSystemColor::SuccessBackground:
        return QColor(dark ? QStringLiteral("#393d1b") : QStringLiteral("#dff6dd"));
    case FluentSystemColor::CautionBackground:
        return QColor(dark ? QStringLiteral("#433519") : QStringLiteral("#fff4ce"));
    case FluentSystemColor::CriticalBackground:
        return QColor(dark ? QStringLiteral("#442726") : QStringLiteral("#fde7e9"));
    }

    return QColor();
}

QColor themeColor() { return ThemeManager::instance()->accentColor(); }

QColor validColor(const QColor &color, const QColor &defaultColor) { return color.isValid() ? color : defaultColor; }

QColor fallbackThemeColor(const QColor &color) { return validColor(color, themeColor()); }

QColor autoFallbackThemeColor(const QColor &light, const QColor &dark)
{
    return fallbackThemeColor(ThemeManager::instance()->effectiveTheme() == Theme::Dark ? dark : light);
}

QColor derivedThemeColor(const QColor &base, int lightnessDelta, int saturationDelta)
{
    if (!base.isValid()) {
        return base;
    }

    int hue = 0;
    int saturation = 0;
    int value = 0;
    int alpha = 255;
    base.getHsv(&hue, &saturation, &value, &alpha);

    saturation = qBound(0, saturation + saturationDelta, 255);
    value = qBound(0, value + lightnessDelta, 255);
    return QColor::fromHsv(hue, saturation, value, alpha);
}

QColor derivedThemeColor(const QColor &accent, ThemeColor type, Theme theme)
{
    if (!accent.isValid()) {
        return accent;
    }

    if (type == ThemeColor::Primary) {
        return accent;
    }

    const Theme resolved = theme == Theme::Auto ? ThemeManager::instance()->effectiveTheme() : theme;
    const bool isDark = resolved == Theme::Dark;

    float h = 0.0f;
    float s = 0.0f;
    float v = 0.0f;
    float a = 1.0f;
    accent.getHsvF(&h, &s, &v, &a);

    if (isDark) {
        s *= 0.84f;
        v = 1.0f;
        switch (type) {
        case ThemeColor::Dark1:
            v *= 0.9f;
            break;
        case ThemeColor::Dark2:
            s *= 0.977f;
            v *= 0.82f;
            break;
        case ThemeColor::Dark3:
            s *= 0.95f;
            v *= 0.7f;
            break;
        case ThemeColor::Light1:
            s *= 0.92f;
            break;
        case ThemeColor::Light2:
            s *= 0.78f;
            break;
        case ThemeColor::Light3:
            s *= 0.65f;
            break;
        case ThemeColor::Primary:
            break;
        }
    } else {
        switch (type) {
        case ThemeColor::Dark1:
            v *= 0.75f;
            break;
        case ThemeColor::Dark2:
            s *= 1.05f;
            v *= 0.5f;
            break;
        case ThemeColor::Dark3:
            s *= 1.1f;
            v *= 0.4f;
            break;
        case ThemeColor::Light1:
            v *= 1.05f;
            break;
        case ThemeColor::Light2:
            s *= 0.75f;
            v *= 1.05f;
            break;
        case ThemeColor::Light3:
            s *= 0.65f;
            v *= 1.05f;
            break;
        case ThemeColor::Primary:
            break;
        }
    }

    s = (s < 0.0f) ? 0.0f : ((s > 1.0f) ? 1.0f : s);
    v = (v < 0.0f) ? 0.0f : ((v > 1.0f) ? 1.0f : v);

    return QColor::fromHsvF(h, s, v, a);
}

} // namespace FluentQt
