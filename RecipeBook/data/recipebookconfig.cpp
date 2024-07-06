#include "recipebookconfig.h"

#include "definition.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

RecipeBookConfig::RecipeBookConfig()
{
    QFile file(CONFIG_FILE_NAME);

    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray content = file.readAll();
            file.close();

            QJsonParseError jsonError;
            QJsonDocument doc = QJsonDocument::fromJson(QString(content).toUtf8(), &jsonError);

            if (jsonError.error != QJsonParseError::NoError) {
                _canBeSaved = false;
                return;
            }

            if (!doc.isObject()) {
                _canBeSaved = false;
                return;
            }

            QJsonObject root = doc.object();
            const QJsonValue recipePath = root[FILE_CONFIG_PATH];
            if (!recipePath.isString()) {
                return;
            }
            _recipeDirectory = recipePath.toString();
        } else {
            _canBeSaved = false;
        }
    }
}

void RecipeBookConfig::save() const
{
    if (!_canBeSaved) {
        return;
    }
    QJsonObject root;
    root.insert(FILE_CONFIG_PATH, _recipeDirectory.absolutePath());

    QJsonDocument doc;
    doc.setObject(root);

    QFile file(CONFIG_FILE_NAME);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

QDir RecipeBookConfig::recipeDirectory() const
{
    return _recipeDirectory;
}

void RecipeBookConfig::setRecipeDirectory(const QString &path)
{
    _recipeDirectory = path;
}
