#ifndef STEP_HIGHLIGHTER_H
#define STEP_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class StepHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    StepHighlighter(QObject *parent);

protected:
    void highlightBlock(const QString &text) override;

private:
    QTextCharFormat     _groupFormat;
    QTextCharFormat     _oddFormat;
    QTextCharFormat     _evenFormat;
};

#endif // STEP_HIGHLIGHTER_H
