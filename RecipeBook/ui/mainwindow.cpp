#include "mainwindow.h"

#include "ui_displayform.h"

#include "configdialog.h"
#include "newrecipedialog.h"
#include "ingredienthighlighter.h"
#include "stephighlighter.h"
#include "unit.h"

#include <QCloseEvent>
#include <QPrinter>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _displayUi(new Ui::DisplayForm)
{
    Unit::initilize();
    
    _recipeModel = new RecipeModel(this);
    _recipeGrouper = new GroupByCategorie(this);
    _recipeGrouper->setSourceModel(_recipeModel);

    _displayUi->setupUi(this);
    _displayUi->recipeListWidget->setModel(_recipeGrouper);

    new IngredientHighlighter(_displayUi->ingredientsTextEdit);
    new StepHighlighter(_displayUi->stepsTextEdit);

    QFile fStyle(":/styles/display.css");
    fStyle.open(QIODevice::ReadOnly|QIODevice::Text);
    _displayStyle = fStyle.readAll();
    fStyle.close();

    QTextDocument *doc = new QTextDocument(this);
    doc->setDefaultStyleSheet(_displayStyle);

    _displayUi->displayTextEdit->setDocument(doc);

    reloadRecipeDirectory();

    connect(_displayUi->actConfiguration, &QAction::triggered, this, &MainWindow::configure);
    connect(_displayUi->actReload, &QAction::triggered, this, &MainWindow::reloadRecipeDirectory);
    connect(_displayUi->actQuit, &QAction::triggered, this, &MainWindow::close);

    connect(_displayUi->actNewRecipe, &QAction::triggered, this, &MainWindow::newRecipe);
    connect(_displayUi->actEdit, &QAction::triggered, this, &MainWindow::toggleEditCurrent);
    connect(_displayUi->actSave, &QAction::triggered, this, &MainWindow::saveCurrent);
    connect(_displayUi->actExportPdf, &QAction::triggered, this, &MainWindow::exportPdf);
    connect(_displayUi->actRemove, &QAction::triggered, this, &MainWindow::removeCurrent);

    connect(_displayUi->recipeListWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::selectionChanged);

    connect(_displayUi->nameLineEdit, &QLineEdit::textEdited, this, &MainWindow::editName);
    connect(_displayUi->descriptionTextEdit, &QTextEdit::textChanged, this, &MainWindow::editDescription);
    connect(_displayUi->ingredientsTextEdit, &QTextEdit::textChanged, this, &MainWindow::editIngredients);
    connect(_displayUi->stepsTextEdit, &QTextEdit::textChanged, this, &MainWindow::editSteps);
    connect(_displayUi->notesTextEdit, &QTextEdit::textChanged, this, &MainWindow::editNotes);
    connect(_displayUi->categoryPushButton, &QPushButton::clicked, this, &MainWindow::editCategorie);
    connect(_displayUi->preparationTimeTimeEdit, &QDateTimeEdit::timeChanged, this, &MainWindow::editPreparationTime);
    connect(_displayUi->cookTimeTimeEdit, &QDateTimeEdit::timeChanged, this, &MainWindow::cookPrepTime);
}

MainWindow::~MainWindow()
{
    delete _displayUi;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool accept = _recipeModel->saveAll(_config.recipeDirectory());
    if (accept) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::configure()
{
    bool accept = _recipeModel->saveAll(_config.recipeDirectory());
    if (!accept) {
        return;
    }
    ConfigDialog* configDialog = new ConfigDialog(this, _config.recipeDirectory().absolutePath());
    int result = configDialog->exec();
    if (result == QDialog::Accepted) {
        _config.setRecipeDirectory(configDialog->getRecipeDirectory());
        reloadRecipeDirectory();
        _config.save();
    }
}

void MainWindow::reloadRecipeDirectory()
{
    _recipeModel->refreshFromDirectory(_config.recipeDirectory());
    _displayUi->recipeListWidget->expandAll();
    _displayUi->recipeWidget->setCurrentIndex(2);
    _displayUi->errorMsgLabel->setText(QString(tr("Select a recipe")));
    _displayUi->actEdit->setChecked(false);
    _displayUi->actEdit->setEnabled(false);
    _displayUi->actSave->setEnabled(false);
    _displayUi->menuExport->setEnabled(false);
    _displayUi->actRemove->setEnabled(false);
}

void MainWindow::newRecipe()
{
    NewRecipeDialog* newRecipeDialog = new NewRecipeDialog(this, _recipeModel->categories());
    int result = newRecipeDialog->exec();
    if (result == QDialog::Accepted) {
        int recipeAddedIndex = _recipeModel->newRecipe(_config.recipeDirectory(), newRecipeDialog->getRecipeName(), newRecipeDialog->getRecipeFileName(), newRecipeDialog->getRecipeCategory());
        if (recipeAddedIndex != -1) {
            QModelIndex grNewIndex = _recipeGrouper->mapFromSource(_recipeModel->index(recipeAddedIndex, 0));
            _displayUi->recipeListWidget->selectionModel()->clearSelection();
            _displayUi->recipeListWidget->selectionModel()->select(grNewIndex, QItemSelectionModel::Select);
        }
    }
}

void MainWindow::toggleEditCurrent()
{
    Recipe* selectedRecipe = currentRecipe();
    if ((selectedRecipe == nullptr) || !selectedRecipe->isValid()) {
        return;
    }
    if (selectedRecipe->isEditing()) {
        selectedRecipe->setEditing(false);
    } else {
        selectedRecipe->setEditing(true);
    }
    displayRecipe(selectedRecipe);
}

void MainWindow::saveCurrent()
{
    Recipe* selectedRecipe = currentRecipe();
    if ((selectedRecipe == nullptr) || !selectedRecipe->isValid()) {
        return;
    }
    selectedRecipe->save(_config.recipeDirectory());
    _recipeModel->updateRecipeInfo(selectedRecipe);
}

void MainWindow::exportPdf()
{
    Recipe* selectedRecipe = currentRecipe();
    if ((selectedRecipe == nullptr) || !selectedRecipe->isValid()) {
        return;
    }
    QFileInfo recipeFile(_config.recipeDirectory().absolutePath(), selectedRecipe->fileName());
    QFileInfo defaultPdfFile(_config.recipeDirectory().absolutePath(), recipeFile.completeBaseName() + ".pdf");

    QString pdfFile = QFileDialog::getSaveFileName(this, tr("Export"), defaultPdfFile.absoluteFilePath(), tr("PDF (*.pdf)"));
    if (pdfFile.isEmpty()) {
        return;
    }
    QTextDocument document;
    document.setDefaultStyleSheet(_displayStyle);
    document.setHtml(selectedRecipe->toDisplay());

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize::A4);
    printer.setOutputFileName(pdfFile);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15));

    document.print(&printer);

    QFile file(pdfFile+".html");
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        stream << selectedRecipe->toDisplay();
    }
}

void MainWindow::removeCurrent()
{
    Recipe* selectedRecipe = currentRecipe();
    if (selectedRecipe == nullptr) {
        return;
    }
    _displayUi->recipeListWidget->selectionModel()->clear();
    _recipeModel->removeRecipe(_config.recipeDirectory().absolutePath(), selectedRecipe);
}

void MainWindow::selectionChanged(const QItemSelection &, const QItemSelection &)
{
    Recipe* selectedRecipe = currentRecipe();
    if (selectedRecipe == nullptr) {
        _displayUi->recipeWidget->setCurrentIndex(2);
        _displayUi->errorMsgLabel->setText(QString(tr("Select a recipe")));
        _displayUi->actEdit->setChecked(false);
        _displayUi->actEdit->setEnabled(false);
        _displayUi->actSave->setEnabled(false);
        _displayUi->menuExport->setEnabled(false);
        _displayUi->actRemove->setEnabled(false);
        return;
    }
    displayRecipe(selectedRecipe);
}

void MainWindow::editName(const QString &text)
{
    Recipe* selectedRecipe = currentRecipe();
    if ((selectedRecipe == nullptr) || !selectedRecipe->isValid()) {
        return;
    }
    selectedRecipe->setName(text);
    _recipeModel->updateRecipeInfo(selectedRecipe);
}

void MainWindow::editDescription()
{
    Recipe* selectedRecipe = currentRecipe();
    if ((selectedRecipe == nullptr) || !selectedRecipe->isValid()) {
        return;
    }
    QString description = _displayUi->descriptionTextEdit->toPlainText();
    if (selectedRecipe->description() != description) {
        selectedRecipe->setDescription(description);
        _recipeModel->updateRecipeInfo(selectedRecipe);
    }
}

void MainWindow::editIngredients()
{
    Recipe* selectedRecipe = currentRecipe();
    if ((selectedRecipe == nullptr) || !selectedRecipe->isValid()) {
        return;
    }
    QString ingredients = _displayUi->ingredientsTextEdit->toPlainText();
    if (selectedRecipe->ingredientsToEdit() != ingredients) {
        selectedRecipe->setIngredients(ingredients);
        _recipeModel->updateRecipeInfo(selectedRecipe);
    }
}

void MainWindow::editSteps()
{
    Recipe* selectedRecipe = currentRecipe();
    if ((selectedRecipe == nullptr) || !selectedRecipe->isValid()) {
        return;
    }
    QString steps = _displayUi->stepsTextEdit->toPlainText();
    if (selectedRecipe->stepsToEdit() != steps) {
        selectedRecipe->setSteps(steps);
        _recipeModel->updateRecipeInfo(selectedRecipe);
    }
}

void MainWindow::editNotes()
{
    Recipe* selectedRecipe = currentRecipe();
    if ((selectedRecipe == nullptr) || !selectedRecipe->isValid()) {
        return;
    }
    QString notes = _displayUi->notesTextEdit->toPlainText();
    if (selectedRecipe->notes() != notes) {
        selectedRecipe->setNotes(notes);
        _recipeModel->updateRecipeInfo(selectedRecipe);
    }
}

void MainWindow::editCategorie()
{
    Recipe* selectedRecipe = currentRecipe();
    if ((selectedRecipe == nullptr) || !selectedRecipe->isValid()) {
        return;
    }
    QString categorie = _displayUi->categoryComboBox->currentText();
    if (selectedRecipe->categorie() != categorie) {
        selectedRecipe->setCategorie(categorie);
        _recipeModel->updateRecipeInfo(selectedRecipe);
    }
}

void MainWindow::editPreparationTime()
{
    Recipe* selectedRecipe = currentRecipe();
    if ((selectedRecipe == nullptr) || !selectedRecipe->isValid()) {
        return;
    }
    QTime preparationTime = _displayUi->preparationTimeTimeEdit->time();
    int preparationTimeMin = preparationTime.minute() + preparationTime.hour() * 60;
    if (selectedRecipe->preparationTimeMin() != preparationTimeMin) {
        selectedRecipe->setPreparationTimeMin(preparationTimeMin);
        _recipeModel->updateRecipeInfo(selectedRecipe);
    }
}

void MainWindow::cookPrepTime()
{
    Recipe* selectedRecipe = currentRecipe();
    if ((selectedRecipe == nullptr) || !selectedRecipe->isValid()) {
        return;
    }
    QTime cookTime = _displayUi->cookTimeTimeEdit->time();
    int cookTimeMin = cookTime.minute() + cookTime.hour() * 60;
    if (selectedRecipe->cookTimeMin() != cookTimeMin) {
        selectedRecipe->setCookTimeMin(cookTimeMin);
        _recipeModel->updateRecipeInfo(selectedRecipe);
    }
}

Recipe* MainWindow::currentRecipe()
{
    QModelIndexList selectedIndexes = _displayUi->recipeListWidget->selectionModel()->selectedIndexes();

    if (selectedIndexes.isEmpty()) {
        return nullptr;
    }
    QModelIndex selectedIndex = _recipeGrouper->mapToSource(selectedIndexes.at(0));
    if (selectedIndex.isValid()) {
        return static_cast<Recipe*>(selectedIndex.internalPointer());
    }
    return nullptr;
}

void MainWindow::displayRecipe(const Recipe *recipe)
{
    if (!recipe->isValid()) {
        _displayUi->recipeWidget->setCurrentIndex(2);
        _displayUi->errorMsgLabel->setText(QString(tr("Error in file %1")).arg(recipe->fileName()));
        _displayUi->actEdit->setChecked(false);
        _displayUi->actEdit->setEnabled(false);
        _displayUi->actSave->setEnabled(false);
        _displayUi->menuExport->setEnabled(false);
        _displayUi->actRemove->setEnabled(true);
        return;
    }

    _displayUi->actEdit->setEnabled(true);
    _displayUi->actSave->setEnabled(true);
    _displayUi->menuExport->setEnabled(true);
    _displayUi->actRemove->setEnabled(true);

    if (recipe->isEditing()) {
        _displayUi->recipeWidget->setCurrentIndex(1);
        _displayUi->actEdit->setChecked(true);
        _displayUi->nameLineEdit->setText(recipe->name());
        _displayUi->descriptionTextEdit->setText(recipe->description());
        _displayUi->ingredientsTextEdit->setText(recipe->ingredientsToEdit());
        _displayUi->stepsTextEdit->setText(recipe->stepsToEdit());
        _displayUi->notesTextEdit->setText(recipe->notes());
        _displayUi->categoryComboBox->blockSignals(true);
        _displayUi->categoryComboBox->clear();
        _displayUi->categoryComboBox->addItems(_recipeModel->categories());
        _displayUi->categoryComboBox->blockSignals(false);
        _displayUi->categoryComboBox->setEditText(recipe->categorie());
        _displayUi->preparationTimeTimeEdit->setTime(QTime(0, recipe->preparationTimeMin()));
        _displayUi->cookTimeTimeEdit->setTime(QTime(0, recipe->cookTimeMin()));
    } else {
        _displayUi->recipeWidget->setCurrentIndex(0);
        _displayUi->actEdit->setChecked(false);
        _displayUi->displayTextEdit->setText(recipe->toDisplay());
    }
}
