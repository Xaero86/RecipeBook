#ifndef NEW_RECIPE_DIALOG_H
#define NEW_RECIPE_DIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>

class NewRecipeDialog : public QDialog
{
    Q_OBJECT

public:
    NewRecipeDialog(QWidget *parent, const QStringList &categories);
    virtual ~NewRecipeDialog() = default;

    QString getRecipeName() const;
    QString getRecipeFileName() const;
    QString getRecipeCategory() const;

private slots:
    void nameChanged();
    void toggleAutoFileName();

private:
    QLineEdit*         _leRecipeName;
    QLineEdit*         _leRecipeFileName;
    QComboBox*         _cbRecipeCategory;

    QLabel*            _lRecipeName;
    QLabel*            _lRecipeFileName;
    QCheckBox*         _cbAutoFileName;
    QLabel*            _lRecipeCategory;

    QDialogButtonBox*  _buttonBox;
};

#endif // NEW_RECIPE_DIALOG_H
