#include "unit.h"

QMap<QString, QSharedPointer<Unit>> Unit::allUnits;

Unit::Unit(const QString& name, const QString& regex) : _name(name) , _reference(), _ratio()
{
    if (regex.isNull()) {
        _regex = QRegularExpression(QStringLiteral("^%1\\b").arg(_name), QRegularExpression::CaseInsensitiveOption);
    } else {
        _regex = QRegularExpression(QStringLiteral("^%1\\b").arg(regex), QRegularExpression::CaseInsensitiveOption);
    }
}

void Unit::setReference(Unit *reference, float ratio)
{
    _reference = reference;
    _ratio = ratio;
}

void Unit::initilize()
{
    allUnits["g"]   = QSharedPointer<Unit>(new Unit("g"));
    allUnits["kg"]  = QSharedPointer<Unit>(new Unit("kg"));
    allUnits["kg"]->setReference(allUnits["g"].data(), 1000.0f);
    allUnits["hg"]  = QSharedPointer<Unit>(new Unit("hg"));
    allUnits["hg"]->setReference(allUnits["g"].data(), 100.0f);
    allUnits["dag"] = QSharedPointer<Unit>(new Unit("dag"));
    allUnits["dag"]->setReference(allUnits["g"].data(), 10.0f);
    allUnits["l"]   = QSharedPointer<Unit>(new Unit("l"));
    allUnits["dl"]  = QSharedPointer<Unit>(new Unit("dl"));
    allUnits["dl"]->setReference(allUnits["l"].data(), 0.1f);
    allUnits["cl"]  = QSharedPointer<Unit>(new Unit("cl"));
    allUnits["cl"]->setReference(allUnits["l"].data(), 0.01f);
    allUnits["ml"]  = QSharedPointer<Unit>(new Unit("ml"));
    allUnits["ml"]->setReference(allUnits["l"].data(), 0.001f);
    //allUnits["pincées"]  = QSharedPointer<Unit>(new Unit("pincées", "pincées?"));
}

QString Unit::toString() const
{
    return _name;
}

QSharedPointer<Unit> Unit::fromString(const QString& name)
{
    return allUnits.value(name, QSharedPointer<Unit>());
}

int Unit::matchLength(const QString& text) const
{
    QRegularExpressionMatch matchIterator = _regex.match(text);
    return matchIterator.capturedLength();
}

QMap<QString, QSharedPointer<Unit>>::const_iterator Unit::firstUnit()
{
    return allUnits.constBegin();
}

QMap<QString, QSharedPointer<Unit>>::const_iterator Unit::lastUnit()
{
    return allUnits.constEnd();
}
