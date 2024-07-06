#ifndef STEP_H
#define STEP_H

#include <QString>
#include <QSharedPointer>
#include <QList>

class Step
{
public:
    Step(const QString &step);

    QString toEdit() const;
    QString toDisplay() const;
    static QSharedPointer<Step> fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

    void appendStep(const QString &step);
    void appendNotes(const QString &notes);

private:
    QString       _step;
    QString       _notes;
};

class StepGroup
{
public:
    StepGroup();
    StepGroup(const QString &title);

    QString toEdit(bool first) const;
    QString toDisplay(int &currentIndex) const;
    static QSharedPointer<StepGroup> fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

    void appendStep(const QString &step, bool newStep = false);
    void appendNotes(const QString &notes);

private:
    QString                       _title;
    QVector<QSharedPointer<Step>> _steps;
    QString                       _notes;

    QSharedPointer<Step> currentStep();
};

#endif // STEP_H
