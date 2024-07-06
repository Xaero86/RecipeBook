#include "newrecipedialog.h"

#include <QVBoxLayout>
#include <QFormLayout>

NewRecipeDialog::NewRecipeDialog(QWidget *parent, const QStringList &categories)
    : QDialog(parent)
{
    setWindowTitle(tr("New recipe"));

    QFormLayout* formLayout = new QFormLayout;

    _lRecipeName = new QLabel(tr("Recipe name:"));
    _leRecipeName = new QLineEdit;
    _leRecipeName->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    _leRecipeName->setText("");

    formLayout->addRow(_lRecipeName, _leRecipeName);

    _lRecipeFileName = new QLabel(tr("Recipe file name:"));
    _leRecipeFileName = new QLineEdit;
    _leRecipeFileName->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    _leRecipeFileName->setText("");
    _leRecipeFileName->setEnabled(false);
    _cbAutoFileName = new QCheckBox(tr("auto"));
    _cbAutoFileName->setChecked(true);

    QHBoxLayout* fileNameLayout = new QHBoxLayout;
    fileNameLayout->setContentsMargins(QMargins());
    fileNameLayout->addWidget(_lRecipeFileName);
    fileNameLayout->addWidget(_cbAutoFileName);
    QWidget* fileNameWidget = new QWidget;
    fileNameWidget->setLayout(fileNameLayout);

    formLayout->addRow(fileNameWidget, _leRecipeFileName);

    _lRecipeCategory = new QLabel(tr("Recipe category:"));
    _cbRecipeCategory = new QComboBox;
    _cbRecipeCategory->setEditable(true);
    _cbRecipeCategory->clear();
    _cbRecipeCategory->addItems(categories);

    formLayout->addRow(_lRecipeCategory, _cbRecipeCategory);

    _buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    formLayout->addRow(_buttonBox);
    setLayout(formLayout);
    formLayout->setSizeConstraint(QLayout::SetFixedSize);

    connect(_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(_leRecipeName, &QLineEdit::textEdited, this, &NewRecipeDialog::nameChanged);
    connect(_cbAutoFileName, &QCheckBox::clicked, this, &NewRecipeDialog::toggleAutoFileName);
}

QString NewRecipeDialog::getRecipeName() const
{
    return _leRecipeName->text();
}

QString NewRecipeDialog::getRecipeFileName() const
{
    return _leRecipeFileName->text();
}

QString NewRecipeDialog::getRecipeCategory() const
{
    return _cbRecipeCategory->currentText();
}

void NewRecipeDialog::nameChanged()
{
    if (_cbAutoFileName->isChecked()) {
        _leRecipeFileName->setText(_leRecipeName->text());
    }
}

void NewRecipeDialog::toggleAutoFileName()
{
    if (_cbAutoFileName->isChecked()) {
        _leRecipeFileName->setText(_leRecipeName->text());
        _leRecipeFileName->setEnabled(false);
    } else {
        _leRecipeFileName->setEnabled(true);
    }
}
