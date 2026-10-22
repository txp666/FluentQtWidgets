#include "TextWrap_p.h"

#include <QtCore/QStringList>
#include <QtCore/QTextBoundaryFinder>
#include <QtCore/QVector>

namespace FluentQt::Private {

namespace {

enum class ClusterType
{
    Space,
    Asian,
    Latin,
};

// East Asian Wide and Fullwidth ranges, including the supplementary planes (emoji, CJK
// extensions), which a single UTF-16 code unit cannot represent.
int codePointColumns(char32_t u)
{
    if ((u >= 0x1100 && u <= 0x115F) || u == 0x2329 || u == 0x232A || (u >= 0x2E80 && u <= 0xA4CF) ||
        (u >= 0xAC00 && u <= 0xD7A3) || (u >= 0xF900 && u <= 0xFAFF) || (u >= 0xFE10 && u <= 0xFE19) ||
        (u >= 0xFE30 && u <= 0xFE6F) || (u >= 0xFF00 && u <= 0xFF60) || (u >= 0xFFE0 && u <= 0xFFE6) ||
        (u >= 0x1F300 && u <= 0x1F64F) || (u >= 0x1F900 && u <= 0x1F9FF) || (u >= 0x20000 && u <= 0x3FFFD)) {
        return 2;
    }
    return 1;
}

// A cluster is as wide as its base character; combining marks and joined emoji add nothing.
int clusterColumns(const QString &cluster)
{
    if (cluster.isEmpty()) {
        return 0;
    }
    const QChar first = cluster.at(0);
    if (first.isHighSurrogate() && cluster.size() > 1 && cluster.at(1).isLowSurrogate()) {
        return codePointColumns(QChar::surrogateToUcs4(first, cluster.at(1)));
    }
    return codePointColumns(first.unicode());
}

ClusterType clusterType(const QString &cluster)
{
    if (cluster.at(0).isSpace()) {
        return ClusterType::Space;
    }
    return clusterColumns(cluster) == 1 ? ClusterType::Latin : ClusterType::Asian;
}

QStringList graphemeClusters(const QString &text)
{
    QStringList clusters;
    QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, text);
    qsizetype start = 0;
    for (qsizetype end = finder.toNextBoundary(); end != -1; end = finder.toNextBoundary()) {
        if (end > start) {
            clusters.append(text.mid(start, end - start));
        }
        start = end;
    }
    return clusters;
}

QString trimmedRight(QString text)
{
    while (!text.isEmpty() && text.back().isSpace()) {
        text.chop(1);
    }
    return text;
}

QString collapsedWhitespace(const QString &text)
{
    QString result;
    bool inWhitespace = false;
    for (QChar ch : text) {
        if (ch.isSpace()) {
            if (!inWhitespace) {
                result.append(QLatin1Char(' '));
                inWhitespace = true;
            }
        } else {
            result.append(ch);
            inWhitespace = false;
        }
    }
    return result.trimmed();
}

QStringList splitLines(const QString &text)
{
    QStringList lines;
    QString current;
    for (qsizetype i = 0; i < text.size(); ++i) {
        const QChar ch = text.at(i);
        if (ch == QLatin1Char('\r') || ch == QLatin1Char('\n')) {
            lines.append(current);
            current.clear();
            if (ch == QLatin1Char('\r') && i + 1 < text.size() && text.at(i + 1) == QLatin1Char('\n')) {
                ++i;
            }
        } else {
            current.append(ch);
        }
    }
    lines.append(current);
    return lines;
}

// Runs of Latin clusters form one token (a word); every space and every wide cluster is a token
// of its own, so lines may break between any two CJK characters.
QVector<QStringList> tokenize(const QStringList &clusters)
{
    QVector<QStringList> tokens;
    ClusterType lastType = ClusterType::Latin;
    for (const QString &cluster : clusters) {
        const ClusterType type = clusterType(cluster);
        if (tokens.isEmpty() || type != lastType || type != ClusterType::Latin) {
            tokens.append(QStringList());
        }
        tokens.last().append(cluster);
        lastType = type;
    }
    return tokens;
}

int columns(const QStringList &clusters)
{
    int total = 0;
    for (const QString &cluster : clusters) {
        total += clusterColumns(cluster);
    }
    return total;
}

QString wrapLine(const QString &line, int width)
{
    QStringList wrappedLines;
    QString lineBuffer;
    int currentWidth = 0;

    for (const QStringList &token : tokenize(graphemeClusters(line))) {
        const int tokenWidth = columns(token);
        if (currentWidth == 0 && token.size() == 1 && token.first() == QLatin1String(" ")) {
            continue;
        }

        if (currentWidth + tokenWidth <= width) {
            lineBuffer += token.join(QString());
            currentWidth += tokenWidth;
            if (currentWidth == width) {
                wrappedLines.append(trimmedRight(lineBuffer));
                lineBuffer.clear();
                currentWidth = 0;
            }
            continue;
        }

        if (currentWidth != 0) {
            wrappedLines.append(trimmedRight(lineBuffer));
        }

        // A token wider than the line is cut by columns, between clusters.
        lineBuffer.clear();
        currentWidth = 0;
        for (const QString &cluster : token) {
            const int clusterWidth = clusterColumns(cluster);
            if (currentWidth != 0 && currentWidth + clusterWidth > width) {
                wrappedLines.append(trimmedRight(lineBuffer));
                lineBuffer.clear();
                currentWidth = 0;
            }
            lineBuffer += cluster;
            currentWidth += clusterWidth;
        }
    }

    if (currentWidth != 0) {
        wrappedLines.append(trimmedRight(lineBuffer));
    }
    return wrappedLines.join(QLatin1Char('\n'));
}

} // namespace

int textColumns(const QString &text) { return columns(graphemeClusters(text)); }

QString wrapText(const QString &text, int maxColumns)
{
    const int width = qMax(maxColumns, 1);
    QStringList wrappedLines;
    for (const QString &rawLine : splitLines(text)) {
        const QString line = collapsedWhitespace(rawLine);
        wrappedLines.append(textColumns(line) > width ? wrapLine(line, width) : line);
    }
    return wrappedLines.join(QLatin1Char('\n'));
}

} // namespace FluentQt::Private
