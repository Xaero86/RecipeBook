#include "ingredient.h"

#include "definition.h"

#include <QJsonObject>
#include <QJsonArray>

Ingredient::Ingredient(const QString &type, const QString &amount, QSharedPointer<Unit> unit) : _type(type), _amount(amount), _unit(unit), _notes()
{}

QString Ingredient::toEdit() const
{
    QString result;
    if (!_amount.isEmpty()) {
        result.append(_amount).append(" ");
        if (!_unit.isNull()) {
            result.append(_unit->toString()).append(" ");
        }
    }
    result.append(_type).append("\n");
    if (!_notes.isEmpty()) {
        result.append("Notes: ").append(_notes).append("\n");
    }
    return result;
}

QString Ingredient::toDisplay() const
{
    QString result = "<li>";
    if (!_amount.isEmpty()) {
        result.append("<span class=\"quantity\">");
        result.append(_amount).append(" ");
        if (!_unit.isNull()) {
            result.append(_unit->toString()).append(" ");
        }
        result.append("</span>");
    }
    result.append(_type).append("</li>\n");
    if (!_notes.isEmpty()) {
        result.append(QString("<span class=\"notes\">Notes: %1</span>").arg(_notes)).append("\n");
    }
    return result;
}

QSharedPointer<Ingredient> Ingredient::fromJson(const QJsonObject &json)
{
    QString type = json[FILE_RECIPE_INGREDIENT_TYPE].toString();
    QString amount = json[FILE_RECIPE_INGREDIENT_AMOUNT].toString();
    QString unit = json[FILE_RECIPE_INGREDIENT_UNIT].toString();
    QSharedPointer<Ingredient> result = QSharedPointer<Ingredient>(new Ingredient(type, amount, Unit::fromString(unit)));

    if (json[FILE_RECIPE_INGREDIENT_NOTES].isString()) {
        result->_notes = json[FILE_RECIPE_INGREDIENT_NOTES].toString();
    }

    const QJsonValue substitutions = json[FILE_RECIPE_INGREDIENT_NOTES];
    if (substitutions.isArray()) {
        const QJsonArray substitutionsArray = substitutions.toArray();
        for (const QJsonValue &substitution : substitutionsArray) {
            QSharedPointer<Ingredient> newIngredient = Ingredient::fromJson(substitution.toObject());
            result->_substitutions.append(newIngredient);
        }
    }
    return result;
}

QJsonObject Ingredient::toJson() const
{
    QJsonObject result;
    result[FILE_RECIPE_INGREDIENT_TYPE] = _type;
    if (!_amount.isNull()) {
        result[FILE_RECIPE_INGREDIENT_AMOUNT] = _amount;
    }
    if (!_unit.isNull()) {
        result[FILE_RECIPE_INGREDIENT_UNIT] = _unit->toString();
    }
    if (!_notes.isNull()) {
        result[FILE_RECIPE_INGREDIENT_NOTES] = _notes;
    }
    if (!_substitutions.isEmpty()) {
        QJsonArray substitutions;
        foreach (auto &substitution, _substitutions) {
            substitutions.append(substitution->toJson());
        }
        result.insert(FILE_RECIPE_INGREDIENT_SUBS, substitutions);
    }
    return result;
}

void Ingredient::appendNotes(const QString &notes)
{
    if (_notes.isEmpty()) {
        _notes = notes;
    } else {
        _notes.append("\n").append(notes);
    }
}

IngredientGroup::IngredientGroup() : _title(), _notes()
{}

IngredientGroup::IngredientGroup(const QString &title) : _title(title), _notes()
{}

QString IngredientGroup::toEdit(bool first) const
{
    QString result;

    if (!first || !_title.isEmpty()) {
        result = QString(" = ").append(_title).append("\n");
    }
    if (!_notes.isEmpty()) {
        result.append("Notes: ").append(_notes).append("\n");
    }
    foreach (auto &ingredient, _ingredients) {
        result.append(ingredient->toEdit());
    }
    return result;
}

QString IngredientGroup::toDisplay(bool first) const
{
    QString result;

    if (!first || !_title.isEmpty()) {
        result = QString("<p><span class=\"indGroupTitle\">%1</span></p>").arg(_title);
    }
    if (!_notes.isEmpty()) {
        result.append(QString("<span class=\"notes\">Notes: %1</span>").arg(_notes)).append("\n");
    }
    result.append("<ul>");
    foreach (auto &ingredient, _ingredients) {
        result.append(ingredient->toDisplay());
    }
    result.append("</ul>");
    return result;
}

QSharedPointer<IngredientGroup> IngredientGroup::fromJson(const QJsonObject &json)
{
    QSharedPointer<IngredientGroup> result(new IngredientGroup());
    result->_title = json[FILE_RECIPE_IG_TITLE].toString();
    result->_notes = json[FILE_RECIPE_IG_NOTES].toString();

    const QJsonValue ingredients = json[FILE_RECIPE_IG_INGREDIENTS];
    if (ingredients.isArray()) {
        const QJsonArray ingredientsArray = ingredients.toArray();
        for (const QJsonValue &ingredient : ingredientsArray) {
            QSharedPointer<Ingredient> newIngredient = Ingredient::fromJson(ingredient.toObject());
            result->_ingredients.append(newIngredient);
        }
    }
    return result;
}

QJsonObject IngredientGroup::toJson() const
{
    QJsonObject result;
    if (!_title.isNull()) {
        result[FILE_RECIPE_IG_TITLE] = _title;
    }
    if (!_notes.isNull()) {
        result[FILE_RECIPE_SG_NOTES] = _notes;
    }
    if (!_ingredients.isEmpty()) {
        QJsonArray ingredients;
        foreach (auto &ingredient, _ingredients) {
            ingredients.append(ingredient->toJson());
        }
        result.insert(FILE_RECIPE_IG_INGREDIENTS, ingredients);
    }
    return result;
}

void IngredientGroup::appendIngredient(const QString &type, const QString &amount, const QString &unit)
{
    _ingredients.push_back(QSharedPointer<Ingredient>(new Ingredient(type, amount, Unit::fromString(unit))));
}

void IngredientGroup::appendNotes(const QString &notes)
{
    if (!currentIngredient().isNull()) {
        currentIngredient()->appendNotes(notes);
    } else {
        if (_notes.isEmpty()) {
            _notes = notes;
        } else {
            _notes.append("\n").append(notes);
        }
    }
}

QSharedPointer<Ingredient> IngredientGroup::currentIngredient()
{
    if (_ingredients.isEmpty()) {
        return QSharedPointer<Ingredient>();
    }
    return _ingredients.last();
}
