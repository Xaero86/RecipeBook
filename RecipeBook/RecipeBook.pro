QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    data/aggregateproxymodel.cpp \
    data/ingredient.cpp \
	data/recipemodel.cpp \
    data/step.cpp \
    data/unit.cpp \
    main.cpp \
    data/recipe.cpp \
	data/recipebookconfig.cpp \
    ui/configdialog.cpp \
    ui/ingredienthighlighter.cpp \
	ui/mainwindow.cpp \
    ui/newrecipedialog.cpp \
    ui/stephighlighter.cpp

HEADERS += \
    data/aggregateproxymodel.h \
    data/definition.h \
    data/ingredient.h \
    data/recipe.h \
    data/recipebookconfig.h \
	data/recipemodel.h \
    data/step.h \
    data/unit.h \
    ui/configdialog.h \
    ui/ingredienthighlighter.h \
    ui/mainwindow.h \
    ui/newrecipedialog.h \
    ui/stephighlighter.h

INCLUDEPATH += data ui

FORMS += \
    form/displayform.ui

RESOURCES += \
    RecipeBook.qrc

# translation
TRANSLATIONS += languages/recipebook_fr_FR.ts \
                languages/recipebook_en_EN.ts

CONFIG += lrelease
CONFIG += embed_translations
