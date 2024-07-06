#ifndef INGREDIENT_HIGHLIGHTER_H
#define INGREDIENT_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class IngredientHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    IngredientHighlighter(QObject *parent);

protected:
    void highlightBlock(const QString &text) override;

private:
    QTextCharFormat     _groupFormat;
    QTextCharFormat     _amountFormat;
    QTextCharFormat     _unitFormat;
    QTextCharFormat     _notesFormat;

    QList<QRegularExpression> _unitsRegex;
};

#endif // INGREDIENT_HIGHLIGHTER_H
