#include "stephighlighter.h"

#include "recipe.h"

StepHighlighter::StepHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
{
    _groupFormat.setFontWeight(QFont::Bold);
    _oddFormat.setBackground(Qt::lightGray);
    _evenFormat.setBackground(Qt::white);
}

void StepHighlighter::highlightBlock(const QString &text)
{
    QRegularExpressionMatch matchGroup = Recipe::STEP_GROUP_PATTERN.match(text);
    if (matchGroup.hasMatch()) {
        setFormat(0, text.length(), _groupFormat);
        setCurrentBlockState(-1);
        return;
    }
    if (previousBlockState() == -1) {
        setCurrentBlockState(0);
    } else {
        QRegularExpressionMatch matchStep = Recipe::STEP_START_PATTERN.match(text);
        if (matchStep.hasMatch()) {
            if (previousBlockState() == 0) {
                setCurrentBlockState(1);
            } else {
                setCurrentBlockState(0);
            }
        } else {
            setCurrentBlockState(previousBlockState());
        }
    }

    if (currentBlockState() == 1) {
        setFormat(0, text.length(), _oddFormat);
    } else {
        setFormat(0, text.length(), _evenFormat);
    }

    QRegularExpressionMatch matchNotes = Recipe::STEP_NOTES_PATTERN.match(text);
    if (matchNotes.hasMatch()) {
        QTextCharFormat currFormat = format(0);
        currFormat.setFontItalic(true);
        setFormat(0, text.length(), currFormat);
    }
}
