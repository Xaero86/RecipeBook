#ifndef RECIPE_MODEL_H
#define RECIPE_MODEL_H

#include "recipe.h"

#include <QAbstractItemModel>
#include <QDir>

class RecipeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    RecipeModel(QObject *parent);
    virtual ~RecipeModel() = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QVariant data(const QModelIndex &modelndex, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &modelndex) const override;

    void refreshFromDirectory(const QDir &path);
    int newRecipe(const QDir &path, const QString &recipeName, const QString &recipeFileName, const QString &recipeCategory);
    bool saveAll(const QDir &path);
    void updateRecipeInfo(Recipe *recipe);
    void removeRecipe(const QString &path, Recipe *recipe);

    QStringList categories() const;

private:
    QVector<QSharedPointer<Recipe>>   _recipes;
    QStringList                       _categories;

    void updateCategorieList();
};

#endif // RECIPE_MODEL_H
