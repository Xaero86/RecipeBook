#ifndef UNIT_H
#define UNIT_H

#include <QString>
#include <QMap>
#include <QSharedPointer>
#include <QRegularExpression>

class Unit
{
public:
    static void initilize();

    QString toString() const;
    static QSharedPointer<Unit> fromString(const QString& name);
    int matchLength(const QString& text) const;

    static QMap<QString, QSharedPointer<Unit>>::const_iterator firstUnit();
    static QMap<QString, QSharedPointer<Unit>>::const_iterator lastUnit();

private:
    Unit(const QString& name, const QString& regex = QString());
    Unit(const Unit&) = delete;             // Copy ctor
    Unit(Unit&&) = delete;                  // Move ctor
    Unit& operator=(const Unit&) = delete;  // Copy assignment
    Unit& operator=(Unit&&) = delete;       // Move assignment

    void setReference(Unit *reference, float ratio);

    static QMap<QString, QSharedPointer<Unit>> allUnits;

    QString             _name;
    QRegularExpression  _regex;
    Unit*               _reference;
    float               _ratio;
};

#endif // UNIT_H
