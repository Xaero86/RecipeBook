#include "recipemodel.h"

#include <QFont>
#include <QColor>
#include <QMessageBox>

RecipeModel::RecipeModel(QObject *parent)
    : QAbstractItemModel{parent}
{}

void RecipeModel::refreshFromDirectory(const QDir &path)
{
    beginResetModel();
    _recipes.clear();
    endResetModel();

    QStringList recipeFiles = path.entryList(QStringList() << "*.rcp",QDir::Files);
    foreach(QString recipeFileName, recipeFiles) {
        Recipe* newRecipe = new Recipe(path, recipeFileName);
        beginInsertRows(QModelIndex(), _recipes.count(), _recipes.count());
        _recipes.append(QSharedPointer<Recipe>(newRecipe));
        endInsertRows();
    }
    updateCategorieList();
}

int RecipeModel::newRecipe(const QDir &path, const QString &recipeName, const QString &recipeFileName, const QString &recipeCategory)
{
    QString recipeFileNameCorrected = recipeFileName;
    QFileInfo file(path, recipeFileNameCorrected);

    if (file.completeSuffix() != "rcp") {
        recipeFileNameCorrected += ".rcp";
    }
    QFileInfo fileCorrected(path, recipeFileNameCorrected);

    if (!fileCorrected.exists()) {
        Recipe* newRecipe = new Recipe(path, recipeName, recipeFileNameCorrected, recipeCategory);
        beginInsertRows(QModelIndex(), _recipes.count(), _recipes.count());
        _recipes.append(QSharedPointer<Recipe>(newRecipe));
        endInsertRows();
        updateCategorieList();
        return _recipes.count() - 1;
    } else {
        QMessageBox::information(nullptr, tr("Invalid file name"), QString(tr("File %1 already exists")).arg(recipeFileNameCorrected));
        return -1;
    }
}

bool RecipeModel::saveAll(const QDir &path)
{
    bool saveAll = false;
    foreach(auto recipe, _recipes) {
        if (recipe->needSave()) {
            if (saveAll) {
                recipe->save(path);
            } else {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(nullptr, tr("Save recipe"), QString(tr("%1 recipe has been modified.")).arg(recipe->name()), QMessageBox::Save|QMessageBox::Ignore|QMessageBox::SaveAll|QMessageBox::Abort);
                if (reply == QMessageBox::Save) {
                    recipe->save(path);
                    updateRecipeInfo(recipe.data());
                } else if (reply == QMessageBox::SaveAll) {
                    saveAll = true;
                    recipe->save(path);
                } else if (reply == QMessageBox::Abort) {
                    return false;
                }
            }
        }
    }
    return true;
}

void RecipeModel::updateRecipeInfo(Recipe *recipe)
{
    if (recipe == nullptr) {
        return;
    }
    for (int r = 0; r < rowCount(); r++) {
        Recipe* currentRecipe = _recipes.at(r).data();
        if (recipe == currentRecipe) {
            emit dataChanged(index(r, 0), index(r, columnCount()-1));
            updateCategorieList();
            return;
        }
    }
}

void RecipeModel::removeRecipe(const QString &path, Recipe *recipe)
{
    if (recipe == nullptr) {
        return;
    }
    for (int r = 0; r < rowCount(); r++) {
        Recipe* currentRecipe = _recipes.at(r).data();
        if (recipe == currentRecipe) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(nullptr, tr("Remove recipe"), QString(tr("Definitely remove recipe %1 ?")).arg(recipe->name()), QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                QDir dir(path);
                dir.remove(recipe->fileName());
                beginRemoveRows(QModelIndex(), r, r);
                _recipes.removeAt(r);
                endRemoveRows();
                updateCategorieList();
            }
            return;
        }
    }
}

void RecipeModel::updateCategorieList()
{
    _categories.clear();
    foreach(auto recipe, _recipes) {
        _categories.append(recipe->categorie());
    }
    _categories.removeDuplicates();
    _categories.sort(Qt::CaseInsensitive);
}

QStringList RecipeModel::categories() const
{
    return _categories;
}

int RecipeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return _recipes.count();
}

int RecipeModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QModelIndex RecipeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    return createIndex(row, column, _recipes.at(row).data());
}

QModelIndex RecipeModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

QVariant RecipeModel::data(const QModelIndex &modelndex, int role) const
{
    if (!modelndex.isValid()) {
        return QVariant();
    }
    Recipe* recipe = static_cast<Recipe*>(modelndex.internalPointer());

    switch(role){
    case Qt::DisplayRole:
        if (recipe->name().isNull() || recipe->name().isEmpty()) {
            return recipe->fileName();
        } else {
            return recipe->name();
        }
        break;
    case Qt::ForegroundRole:
        if (!recipe->isValid()) {
            return QColor( Qt::red );
        }
        break;
    case Qt::FontRole:
        if (recipe->needSave()) {
            QFont font;
            font.setItalic(true);
            return font;
        }
        break;
    }
    return QVariant();
}

Qt::ItemFlags RecipeModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
