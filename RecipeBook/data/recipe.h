#ifndef RECIPE_H
#define RECIPE_H

#include <QString>
#include <QDir>
#include <QRegularExpression>

#include "step.h"
#include "ingredient.h"

class Recipe
{
public:
    Recipe(const QDir &path, const QString &fileName);
    Recipe(const QDir &path, const QString &name, const QString &fileName, const QString &recipeCategory);

    void save(const QDir &path);

    bool isValid() const;
    bool needSave() const;
    bool isEditing() const;
    void setEditing(bool status);

    QString fileName() const;
    void setFileName(const QString &fileName);
    QString name() const;
    void setName(const QString &name);
    QString categorie() const;
    void setCategorie(const QString &categorie);
    int preparationTimeMin() const;
    void setPreparationTimeMin(int time);
    int cookTimeMin() const;
    void setCookTimeMin(int time);
    QString description() const;
    void setDescription(const QString &description);
    QString notes() const;
    void setNotes(const QString &notes);
    QString ingredientsToEdit() const;
    void setIngredients(const QString &ingredients);
    QString stepsToEdit() const;
    void setSteps(const QString &steps);
    QString toDisplay() const;

    static const QRegularExpression INGREDIENT_GROUP_PATTERN;
    static const QRegularExpression INGREDIENT_AMOUNT_PATTERN;
    static const QRegularExpression INGREDIENT_NOTES_PATTERN;
    static const QRegularExpression STEP_GROUP_PATTERN;
    static const QRegularExpression STEP_START_PATTERN;
    static const QRegularExpression STEP_NOTES_PATTERN;

private:
    QSharedPointer<IngredientGroup> currentIngredientGroup();
    QSharedPointer<StepGroup> currentStepGroup();

    bool      _isValid = true;
    bool      _needSave = false;
    bool      _isEditing = false;
    QString   _fileName;
    QString   _name;
    QString   _category;
    int       _preparationTimeMin;
    int       _cookTimeMin;
    QString   _description;
    QString   _notes;
    QVector<QSharedPointer<IngredientGroup>> _ingredientGroups;
    QVector<QSharedPointer<StepGroup>>       _stepGroups;
};

#endif // RECIPE_H
