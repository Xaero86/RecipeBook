#ifndef CONFIG_DIALOG_H
#define CONFIG_DIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    ConfigDialog(QWidget *parent, const QString& recipeDirectory);
    virtual ~ConfigDialog() = default;

    QString getRecipeDirectory() const;

private slots:
    void selectDir();

private:
    QLineEdit*         _leRecipeDirectory;

    QLabel*            _lRecipeDirectory;

    QDialogButtonBox*  _buttonBox;
};

#endif // CONFIG_DIALOG_H
