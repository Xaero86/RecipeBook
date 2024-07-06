#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTranslator translator;
    QString locale = QLocale::system().name();
    Q_UNUSED(translator.load(QStringLiteral("recipebook_") + locale, ":i18n"));

    app.installTranslator(&translator);
    MainWindow w;
    w.show();
    return app.exec();
}
