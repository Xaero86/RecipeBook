#ifndef INGREDIENT_H
#define INGREDIENT_H

#include "unit.h"

#include <QString>
#include <QSharedPointer>
#include <QList>

class Ingredient
{
public:
    Ingredient(const QString &type, const QString &amount,  QSharedPointer<Unit> unit);

    QString toEdit() const;
    QString toDisplay() const;
    static QSharedPointer<Ingredient> fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

    void appendNotes(const QString &notes);

private:
    QString                             _type;
    QString                             _amount;
    QSharedPointer<Unit>                _unit;
    QString                             _notes;
    QVector<QSharedPointer<Ingredient>> _substitutions;
};

class IngredientGroup
{
public:
    IngredientGroup();
    IngredientGroup(const QString &title);

    QString toEdit(bool first) const;
    QString toDisplay(bool first) const;
    static QSharedPointer<IngredientGroup> fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

    void appendIngredient(const QString &type, const QString &amount = QString(), const QString &unit = QString());
    void appendNotes(const QString &notes);

private:
    QString                             _title;
    QVector<QSharedPointer<Ingredient>> _ingredients;
    QString                             _notes;

    QSharedPointer<Ingredient> currentIngredient();
};

#endif // INGREDIENT_H
