#include "recipe.h"

#include "definition.h"
#include "unit.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>

const QRegularExpression Recipe::INGREDIENT_GROUP_PATTERN = QRegularExpression(QStringLiteral("^\\s*=\\s*(?<title>.*)$"));
const QRegularExpression Recipe::INGREDIENT_AMOUNT_PATTERN = QRegularExpression(QStringLiteral("^[\\s-]*(?<amount>[0-9]+[,\\.\\/]?[0-9]*)\\s*(?<unit_type>.*)$"));
const QRegularExpression Recipe::INGREDIENT_NOTES_PATTERN = QRegularExpression(QStringLiteral("^\\s*notes?:\\s*(?<notes>.*)$"), QRegularExpression::CaseInsensitiveOption);
const QRegularExpression Recipe::STEP_GROUP_PATTERN = QRegularExpression(QStringLiteral("^\\s*=\\s*(?<title>.*)$"));
const QRegularExpression Recipe::STEP_START_PATTERN = QRegularExpression(QStringLiteral("^\\s*-\\s*(?<step>.*)$"));
const QRegularExpression Recipe::STEP_NOTES_PATTERN = QRegularExpression(QStringLiteral("^\\s*notes?:\\s*(?<notes>.*)$"), QRegularExpression::CaseInsensitiveOption);

Recipe::Recipe(const QDir &path, const QString &fileName) :
    _fileName(fileName)
{
    QFile file(path.filePath(_fileName));

    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray content = file.readAll();
            file.close();

            QJsonParseError jsonError;
            QJsonDocument doc = QJsonDocument::fromJson(QString(content).toUtf8(), &jsonError);

            if (jsonError.error != QJsonParseError::NoError) {
                _isValid = false;
                return;
            }

            if (!doc.isObject()) {
                _isValid = false;
                return;
            }

            QJsonObject recipe = doc.object();
            const QJsonValue name = recipe[FILE_RECIPE_NAME];
            _name = name.toString();
            const QJsonValue categorie = recipe[FILE_RECIPE_CATEGORIE];
            _category = categorie.toString();
            const QJsonValue preparationTime = recipe[FILE_RECIPE_PREPARATION_TIME];
            _preparationTimeMin = preparationTime.toInt();
            const QJsonValue cookTime = recipe[FILE_RECIPE_COOK_TIME];
            _cookTimeMin = cookTime.toInt();
            const QJsonValue description = recipe[FILE_RECIPE_DESCRIPTION];
            _description = description.toString();
            const QJsonValue notes = recipe[FILE_RECIPE_NOTES];
            _notes = notes.toString();

            const QJsonValue ingredientGroups = recipe[FILE_RECIPE_INGREDIENT_GROUPS];
            if (ingredientGroups.isArray()) {
                const QJsonArray ingredientGroupsArray = ingredientGroups.toArray();
                for (const QJsonValue &ingredientGroup : ingredientGroupsArray) {
                    QSharedPointer<IngredientGroup> newIngredientGroup = IngredientGroup::fromJson(ingredientGroup.toObject());
                    _ingredientGroups.append(newIngredientGroup);
                }
            }
            const QJsonValue stepGroups = recipe[FILE_RECIPE_STEP_GROUPS];
            if (stepGroups.isArray()) {
                const QJsonArray stepGroupsArray = stepGroups.toArray();
                for (const QJsonValue &stepGroup : stepGroupsArray) {
                    QSharedPointer<StepGroup> newStepGroup = StepGroup::fromJson(stepGroup.toObject());
                    _stepGroups.append(newStepGroup);
                }
            }
        } else {
            _isValid = false;
        }
    }
}

Recipe::Recipe(const QDir &path, const QString &name, const QString &fileName, const QString &recipeCategory) :
    _fileName(fileName), _name(name), _category(recipeCategory)
{
    save(path);
    _isEditing = true;
}

void Recipe::save(const QDir &path)
{
    if (!_isValid) {
        return;
    }
    QJsonObject recipe;

    if (!_name.isNull()) {
        recipe.insert(FILE_RECIPE_NAME, _name);
    }
    if (!_category.isNull()) {
        recipe.insert(FILE_RECIPE_CATEGORIE, _category);
    }
    if (_preparationTimeMin != 0) {
        recipe.insert(FILE_RECIPE_PREPARATION_TIME, _preparationTimeMin);
    }
    if (_cookTimeMin != 0) {
        recipe.insert(FILE_RECIPE_COOK_TIME, _cookTimeMin);
    }
    if (!_description.isNull()) {
        recipe.insert(FILE_RECIPE_DESCRIPTION, _description);
    }
    if (!_notes.isNull()) {
        recipe.insert(FILE_RECIPE_NOTES, _notes);
    }
    if (!_ingredientGroups.empty()) {
        QJsonArray ingredients;
        for (auto &ingredientGroup : _ingredientGroups) {
            ingredients.append(ingredientGroup->toJson());
        }
        recipe.insert(FILE_RECIPE_INGREDIENT_GROUPS, ingredients);
    }
    if (!_stepGroups.empty()) {
        QJsonArray steps;
        for (auto &stepGroup : _stepGroups) {
            steps.append(stepGroup->toJson());
        }
        recipe.insert(FILE_RECIPE_STEP_GROUPS, steps);
    }
    QJsonDocument doc;
    doc.setObject(recipe);

    QFile file(path.filePath(_fileName));
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
    _needSave = false;
}

bool Recipe::isValid() const
{
    return _isValid;
}

bool Recipe::needSave() const
{
    return _needSave;
}

bool Recipe::isEditing() const
{
    return _isEditing;
}

void Recipe::setEditing(bool status)
{
    _isEditing = status;
}

QString Recipe::fileName() const
{
    return _fileName;
}

void Recipe::setFileName(const QString &fileName)
{
    _fileName = fileName;
}

QString Recipe::name() const
{
    return _name;
}

void Recipe::setName(const QString &name)
{
    _name = name;
    _needSave = true;
}

QString Recipe::categorie() const
{
    return _category;
}

void Recipe::setCategorie(const QString &categorie)
{
    _category = categorie;
    _needSave = true;
}

int Recipe::preparationTimeMin() const
{
    return _preparationTimeMin;
}

void Recipe::setPreparationTimeMin(int time)
{
    _preparationTimeMin = time;
    _needSave = true;
}

int Recipe::cookTimeMin() const
{
    return _cookTimeMin;
}

void Recipe::setCookTimeMin(int time)
{
    _cookTimeMin = time;
    _needSave = true;
}

QString Recipe::description() const
{
    return _description;
}

void Recipe::setDescription(const QString &description)
{
    _description = description;
    _needSave = true;
}

QString Recipe::notes() const
{
    return _notes;
}

void Recipe::setNotes(const QString &notes)
{
    _notes = notes;
    _needSave = true;
}

QString Recipe::ingredientsToEdit() const
{
    bool first = true;
    QString result;
    foreach (auto &ingredient, _ingredientGroups) {
        result.append(ingredient->toEdit(first));
        first = false;
    }
    return result;
}

void Recipe::setIngredients(const QString &ingredients)
{
    QString input = ingredients;
    QString line;
    QTextStream stream(&input);

    _ingredientGroups.clear();

    while (stream.readLineInto(&line)) {
        QRegularExpressionMatch matchGroup = Recipe::INGREDIENT_GROUP_PATTERN.match(line);
        QRegularExpressionMatch matchAmount = Recipe::INGREDIENT_AMOUNT_PATTERN.match(line);
        QRegularExpressionMatch matchNotes = Recipe::INGREDIENT_NOTES_PATTERN.match(line);
        if (matchGroup.hasMatch()) {
            _ingredientGroups.push_back(QSharedPointer<IngredientGroup>(new IngredientGroup(matchGroup.captured(QStringLiteral("title")))));
        } else if (matchNotes.hasMatch()) {
            currentIngredientGroup()->appendNotes(matchNotes.captured(QStringLiteral("notes")));
        } else if (matchAmount.hasMatch()) {
            QString unitType = matchAmount.captured(QStringLiteral("unit_type"));
            QSharedPointer<Unit> unitFound;
            int matchLength = 0;
            for (auto unit = Unit::firstUnit(); unit != Unit::lastUnit(); ++unit) {
                matchLength = unit.value()->matchLength(unitType);
                if (matchLength > 0) {
                    unitFound = unit.value();
                    break;
                }
            }
            if (!unitFound.isNull()) {
                currentIngredientGroup()->appendIngredient(unitType.mid(matchLength, unitType.length()-matchLength).trimmed(), matchAmount.captured(QStringLiteral("amount")), unitFound->toString());
            } else {
                currentIngredientGroup()->appendIngredient(unitType, matchAmount.captured(QStringLiteral("amount")));
            }
        } else if (!line.trimmed().isEmpty()) {
            currentIngredientGroup()->appendIngredient(line);
        }
    }
    _needSave = true;
}

QSharedPointer<IngredientGroup> Recipe::currentIngredientGroup()
{
    if (_ingredientGroups.isEmpty()) {
        _ingredientGroups.push_back(QSharedPointer<IngredientGroup>(new IngredientGroup()));
    }
    return _ingredientGroups.back();
}

QString Recipe::stepsToEdit() const
{
    bool first = true;
    QString result;
    foreach (auto &step, _stepGroups) {
        result.append(step->toEdit(first));
        first = false;
    }
    return result;
}

void Recipe::setSteps(const QString &steps)
{
    QString input = steps;
    QString line;
    QTextStream stream(&input);

    bool notesPending = false;

    _stepGroups.clear();

    while (stream.readLineInto(&line)) {
        QRegularExpressionMatch matchGroup = Recipe::STEP_GROUP_PATTERN.match(line);
        QRegularExpressionMatch matchStep = Recipe::STEP_START_PATTERN.match(line);
        QRegularExpressionMatch matchNotes = Recipe::STEP_NOTES_PATTERN.match(line);
        if (matchGroup.hasMatch()) {
            _stepGroups.push_back(QSharedPointer<StepGroup>(new StepGroup(matchGroup.captured(QStringLiteral("title")))));
            notesPending = false;
        } else if (matchStep.hasMatch()) {
            currentStepGroup()->appendStep(matchStep.captured(QStringLiteral("step")), true);
            notesPending = false;
        } else if (matchNotes.hasMatch()) {
            currentStepGroup()->appendNotes(matchNotes.captured(QStringLiteral("notes")));
            notesPending = true;
        } else if (notesPending) {
            currentStepGroup()->appendNotes(line);
        } else {
            currentStepGroup()->appendStep(line);
        }
    }
    _needSave = true;
}

QSharedPointer<StepGroup> Recipe::currentStepGroup()
{
    if (_stepGroups.isEmpty()) {
        _stepGroups.push_back(QSharedPointer<StepGroup>(new StepGroup()));
    }
    return _stepGroups.last();
}

QString Recipe::toDisplay() const
{
    QString result;
    result.append(QStringLiteral("<html lang=\"fr\">"));
    result.append(QStringLiteral("<head>"));
    result.append(QStringLiteral("<meta charset=\"UTF-8\">"));
    result.append(QStringLiteral("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"));
    result.append(QStringLiteral("<title>%1</title>").arg(_name));
    result.append(QStringLiteral("<body>"));
    result.append(QStringLiteral("<h1>%1</h1>").arg(_name));

    result.append(QStringLiteral("<div id=\"information\">"));
    if (_preparationTimeMin > 0) {
        result.append(QStringLiteral("<div id=\"prepTime\">"));
        result.append(QStringLiteral("%1<span class=\"duration\">%2 %3</span>").arg(QCoreApplication::translate("Recipe", "Preparation time: ")).arg(_preparationTimeMin).arg(QCoreApplication::translate("Recipe", "min")));
        result.append(QStringLiteral("</div>"));
    }
    if (_cookTimeMin > 0) {
        result.append(QStringLiteral("<div id=\"cookTime\">"));
        result.append(QStringLiteral("%1<span class=\"duration\">%2 %3</span>").arg(QCoreApplication::translate("Recipe", "Cooking time: ")).arg(_cookTimeMin).arg(QCoreApplication::translate("Recipe", "min")));
        result.append(QStringLiteral("</div>"));
    }
    result.append(QStringLiteral("</div>"));
    if (!_description.isEmpty()) {
        QString description(_description);
        description.replace("\n", "<br>");
        result.append(description);
    }
    result.append(QStringLiteral("<h2>%1</h2>").arg(QCoreApplication::translate("Recipe", "Ingredients")));
    bool first = true;
    foreach (auto &ingredient, _ingredientGroups) {
        result.append(ingredient->toDisplay(first));
        first = false;
    }
    result.append(QStringLiteral("<h2>%1</h2>").arg(QCoreApplication::translate("Recipe", "Steps")));
    int currentIndex = 1;
    foreach (auto &step, _stepGroups) {
        result.append(step->toDisplay(currentIndex));
    }
    if (!_notes.isEmpty()) {
        result.append(QStringLiteral("<h2>%1</h2>").arg(QCoreApplication::translate("Recipe", "Notes")));
        QString notes(_notes);
        notes.replace("\n", "<br>");
        result.append(notes);
    }
    result.append(QStringLiteral("</body>"));
    result.append(QStringLiteral("</html>"));
    return result;
}
