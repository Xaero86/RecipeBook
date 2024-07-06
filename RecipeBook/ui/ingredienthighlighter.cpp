#include "ingredienthighlighter.h"

#include "recipe.h"
#include "unit.h"

IngredientHighlighter::IngredientHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
{
    _groupFormat.setFontWeight(QFont::Bold);
    _amountFormat.setForeground(Qt::red);
    _unitFormat.setForeground(Qt::green);
    _notesFormat.setFontItalic(true);
}

void IngredientHighlighter::highlightBlock(const QString &text)
{
    QRegularExpressionMatch matchGroup = Recipe::INGREDIENT_GROUP_PATTERN.match(text);
    if (matchGroup.hasMatch()) {
        setFormat(0, text.length(), _groupFormat);
        return;
    }

    QRegularExpressionMatch matchNotes = Recipe::INGREDIENT_NOTES_PATTERN.match(text);
    if (matchNotes.hasMatch()) {
        setFormat(0, text.length(), _notesFormat);
    }

    QRegularExpressionMatch matchAmount = Recipe::INGREDIENT_AMOUNT_PATTERN.match(text);
    if (matchAmount.hasMatch()) {
        setFormat(matchAmount.capturedStart(QStringLiteral("amount")), matchAmount.capturedLength(QStringLiteral("amount")), _amountFormat);

        int startUnit = matchAmount.capturedStart(QStringLiteral("unit_type"));
        for (auto unit = Unit::firstUnit(); unit != Unit::lastUnit(); ++unit) {
            int matchLength = unit.value()->matchLength(matchAmount.captured(QStringLiteral("unit_type")));
            if (matchLength > 0) {
                setFormat(startUnit, matchLength, _unitFormat);
                return;
            }
        }
    }
}
