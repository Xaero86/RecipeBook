#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "recipe.h"
#include "recipemodel.h"
#include "aggregateproxymodel.h"
#include "recipebookconfig.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class DisplayForm; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

protected slots:
    void configure();
    void reloadRecipeDirectory();
    void newRecipe();
    void toggleEditCurrent();
    void saveCurrent();
    void exportPdf();
    void removeCurrent();
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void editName(const QString &text);
    void editDescription();
    void editIngredients();
    void editSteps();
    void editNotes();
    void editCategorie();
    void editPreparationTime();
    void cookPrepTime();

private:
    class GroupByCategorie : public AggregateProxyModel
    {
    public:
        explicit GroupByCategorie(QObject *parent = nullptr) : AggregateProxyModel(parent) {}

        virtual QStringList indexPath(const QModelIndex &index) const override
        {
            const Recipe* recipe = static_cast<const Recipe*>(index.constInternalPointer());
            return recipe->categorie().split("/", Qt::SkipEmptyParts, Qt::CaseInsensitive);
        }
    };
    Ui::DisplayForm*   _displayUi;
    RecipeModel*       _recipeModel;
    GroupByCategorie*  _recipeGrouper;

    RecipeBookConfig   _config;

    QByteArray         _displayStyle;

    Recipe* currentRecipe();
    void displayRecipe(const Recipe *recipe);
};
#endif // MAIN_WINDOW_H
