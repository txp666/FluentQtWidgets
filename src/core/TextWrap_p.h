#pragma once

#include <QtCore/QString>

namespace FluentQt::Private {

// Wraps text for the dialog, info bar and flyout labels the way qfluentwidgets' TextWrap.wrap()
// does: whitespace runs in each line collapse to one space, East Asian wide characters count as
// two columns, and lines longer than maxColumns break between words, or inside a word that does
// not fit on a line of its own. Works on grapheme clusters, so a surrogate pair or an emoji
// sequence is never cut in half.
QString wrapText(const QString &text, int maxColumns);

// Columns the text takes up by the same measure.
int textColumns(const QString &text);

} // namespace FluentQt::Private
