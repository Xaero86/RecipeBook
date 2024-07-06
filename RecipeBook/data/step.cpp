#include "step.h"

#include "definition.h"

#include <QJsonObject>
#include <QJsonArray>

Step::Step(const QString &step) : _step(step), _notes()
{}

QString Step::toEdit() const
{
    QString result = QString(" - ").append(_step).append("\n");
    if (!_notes.isEmpty()) {
        result.append("Notes: ").append(_notes).append("\n");
    }
    return result;
}

QString Step::toDisplay() const
{
    QString result = QString("<li>%1</li>").arg(_step).append("\n");
    if (!_notes.isEmpty()) {
        result.append(QString("<span class=\"notes\">Notes: %1</span>").arg(_notes)).append("\n");
    }
    return result;
}

QSharedPointer<Step> Step::fromJson(const QJsonObject &json)
{
    QSharedPointer<Step> result(new Step(json[FILE_RECIPE_STEP].toString()));
    result->_notes = json[FILE_RECIPE_STEP_NOTES].toString();

    return result;
}

QJsonObject Step::toJson() const
{
    QJsonObject result;
    result[FILE_RECIPE_STEP] = _step;
    if (!_notes.isNull()) {
        result[FILE_RECIPE_STEP_NOTES] = _notes;
    }
    return result;
}

void Step::appendStep(const QString &step)
{
    if (_step.isEmpty()) {
        _step = step;
    } else {
        _step.append("\n").append(step);
    }
}

void Step::appendNotes(const QString &notes)
{
    if (_notes.isEmpty()) {
        _notes = notes;
    } else {
        _notes.append("\n").append(notes);
    }
}

StepGroup::StepGroup() : _title(), _notes()
{}

StepGroup::StepGroup(const QString &title) : _title(title), _notes()
{}

QString StepGroup::toEdit(bool first) const
{
    QString result;

    if (!first || !_title.isEmpty()) {
        result = QString(" = ").append(_title).append("\n");
    }
    if (!_notes.isEmpty()) {
        result.append("Notes: ").append(_notes).append("\n");
    }
    foreach (auto &step, _steps) {
        result.append(step->toEdit());
    }
    return result;
}

QString StepGroup::toDisplay(int &currentIndex) const
{
    QString result;

    if ((currentIndex > 0) || !_title.isEmpty()) {
        result = QString("<p><span class=\"indStepTitle\">%1</span></p>").arg(_title);
    }
    if (!_notes.isEmpty()) {
        result.append(QString("<span class=\"notes\">Notes: %1</span>").arg(_notes)).append("\n");
    }
    result.append(QString("<ol start=\"%1\">").arg(currentIndex));
    foreach (auto &step, _steps) {
        result.append(step->toDisplay());
    }
    result.append("</ol>");
    currentIndex += _steps.count();
    return result;
}

QSharedPointer<StepGroup> StepGroup::fromJson(const QJsonObject &json)
{
    QSharedPointer<StepGroup> result(new StepGroup(json[FILE_RECIPE_SG_TITLE].toString()));
    result->_notes = json[FILE_RECIPE_STEP_NOTES].toString();

    const QJsonValue steps = json[FILE_RECIPE_SG_STEPS];
    if (steps.isArray()) {
        const QJsonArray stepsArray = steps.toArray();
        for (const QJsonValue &step : stepsArray) {
            QSharedPointer<Step> newStep = Step::fromJson(step.toObject());
            result->_steps.append(newStep);
        }
    }
    return result;
}

QJsonObject StepGroup::toJson() const
{
    QJsonObject result;
    if (!_title.isNull()) {
        result[FILE_RECIPE_SG_TITLE] = _title;
    }
    if (!_notes.isNull()) {
        result[FILE_RECIPE_SG_NOTES] = _notes;
    }
    if (!_steps.isEmpty()) {
        QJsonArray steps;
        foreach (auto &step, _steps) {
            steps.append(step->toJson());
        }
        result.insert(FILE_RECIPE_SG_STEPS, steps);
    }
    return result;
}

void StepGroup::appendStep(const QString &step, bool newStep)
{
    if (!currentStep().isNull() && !newStep) {
        currentStep()->appendStep(step);
    } else {
        _steps.push_back(QSharedPointer<Step>(new Step(step)));
    }
}

void StepGroup::appendNotes(const QString &notes)
{
    if (!currentStep().isNull()) {
        currentStep()->appendNotes(notes);
    } else {
        if (_notes.isEmpty()) {
            _notes = notes;
        } else {
            _notes.append("\n").append(notes);
        }
    }
}

QSharedPointer<Step> StepGroup::currentStep()
{
    if (_steps.isEmpty()) {
        return QSharedPointer<Step>();
    }
    return _steps.last();
}
