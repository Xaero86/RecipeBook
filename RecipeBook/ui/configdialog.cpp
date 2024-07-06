#include "configdialog.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>

ConfigDialog::ConfigDialog(QWidget *parent, const QString &recipeDirectory)
    : QDialog(parent)
{
    setWindowTitle("Configuration");

    _lRecipeDirectory = new QLabel(tr("Recipe directory:"));
    _leRecipeDirectory = new QLineEdit;
    _leRecipeDirectory->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    _leRecipeDirectory->setMinimumWidth(200);
    _leRecipeDirectory->setText(recipeDirectory);
    QPushButton *pbSelect = new QPushButton(tr("Select"));

    QHBoxLayout* hLayoutRecipeDirectory = new QHBoxLayout;
    hLayoutRecipeDirectory->addWidget(_lRecipeDirectory);
    hLayoutRecipeDirectory->addWidget(_leRecipeDirectory);
    hLayoutRecipeDirectory->addWidget(pbSelect);

    _buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->addLayout(hLayoutRecipeDirectory);
    vLayout->addWidget(_buttonBox);
    setLayout(vLayout);
    vLayout->setSizeConstraint(QLayout::SetFixedSize);

    connect(_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(pbSelect, &QPushButton::clicked, this, &ConfigDialog::selectDir);
}

QString ConfigDialog::getRecipeDirectory() const
{
    return _leRecipeDirectory->text();
}

void ConfigDialog::selectDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory"), _leRecipeDirectory->text());
    if (!dir.isEmpty()) {
        _leRecipeDirectory->setText(dir);
    }
}
