#ifndef RECIPEBOOKCONFIG_H
#define RECIPEBOOKCONFIG_H

#include <QString>
#include <QDir>

class RecipeBookConfig
{
public:
    RecipeBookConfig();

    void save() const;

    QDir recipeDirectory() const;
    void setRecipeDirectory(const QString &path);

private:
    QDir       _recipeDirectory;
    bool       _canBeSaved = true;
};

#endif // RECIPEBOOKCONFIG_H
