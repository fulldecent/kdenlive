/***************************************************************************
                          effectslist.cpp  -  description
                             -------------------
    begin                : Sat Aug 10 2002
    copyright            : (C) 2002 by Jason Wood
    email                : jasonwood@blueyonder.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "effectslist.h"

#include <KDebug>
#include <KLocale>


EffectsList::EffectsList()
{
    m_baseElement = createElement("list");
    appendChild(m_baseElement);
}

EffectsList::~EffectsList()
{
}

QDomElement EffectsList::getEffectByName(const QString & name) const
{
    QString effectName;
    QDomNodeList effects = m_baseElement.childNodes();
    for (int i = 0; i < effects.count(); i++) {
        QDomElement effect =  effects.at(i).toElement();
        QDomElement namenode = effect.firstChildElement("name");
        if (!namenode.isNull()) effectName = i18n(namenode.text().toUtf8().data());
        if (name == effectName) {
            QDomNodeList params = effect.elementsByTagName("parameter");
            for (int i = 0; i < params.count(); i++) {
                QDomElement e = params.item(i).toElement();
                if (!e.hasAttribute("value"))
                    e.setAttribute("value", e.attribute("default"));
            }
            return effect;
        }
    }

    return QDomElement();
}

QDomElement EffectsList::getEffectByTag(const QString & tag, const QString & id) const
{
    QDomNodeList effects = m_baseElement.childNodes();
    for (int i = 0; i < effects.count(); i++) {
        QDomElement effect =  effects.at(i).toElement();
        if (!id.isEmpty()) {
            if (effect.attribute("id") == id) {
                QDomNodeList params = effect.elementsByTagName("parameter");
                for (int i = 0; i < params.count(); i++) {
                    QDomElement e = params.item(i).toElement();
                    if (!e.hasAttribute("value"))
                        e.setAttribute("value", e.attribute("default"));
                }
                return effect;
            }
        } else if (!tag.isEmpty()) {
            if (effect.attribute("tag") == tag) {
                QDomNodeList params = effect.elementsByTagName("parameter");
                for (int i = 0; i < params.count(); i++) {
                    QDomElement e = params.item(i).toElement();
                    if (!e.hasAttribute("value"))
                        e.setAttribute("value", e.attribute("default"));
                }
                return effect;
            }
        }
    }
    return QDomElement();
}

int EffectsList::hasEffect(const QString & tag, const QString & id) const
{
    QDomNodeList effects = m_baseElement.childNodes();
    for (int i = 0; i < effects.count(); i++) {
        QDomElement effect =  effects.at(i).toElement();
        if (!id.isEmpty()) {
            if (effect.attribute("id") == id) return i;
        } else if (!tag.isEmpty() && effect.attribute("tag") == tag) return i;
    }
    return -1;
}

QStringList EffectsList::effectIdInfo(const int ix) const
{
    QStringList info;
    QDomElement effect = m_baseElement.childNodes().at(ix).toElement();
    QDomElement namenode = effect.firstChildElement("name");
    info << i18n(namenode.text().toUtf8().data()) << effect.attribute("tag") << effect.attribute("id");
    return info;
}

QStringList EffectsList::effectNames()
{
    QStringList list;
    QDomNodeList effects = m_baseElement.childNodes();
    for (int i = 0; i < effects.count(); i++) {
        QDomElement effect =  effects.at(i).toElement();
        QDomElement namenode = effect.firstChildElement("name");
        if (!namenode.isNull()) list.append(i18n(namenode.text().toUtf8().data()));
    }
    return list;
}

QString EffectsList::getInfo(const QString & tag, const QString & id) const
{
    QString info;
    return getEffectInfo(getEffectByTag(tag, id));
}

QString EffectsList::getInfoFromIndex(const int ix) const
{
    QString info;
    return getEffectInfo(m_baseElement.childNodes().at(ix).toElement());
}

QString EffectsList::getEffectInfo(const QDomElement effect) const
{
    QString info;
    QDomElement namenode = effect.firstChildElement("description");
    if (!namenode.isNull())
        info = i18n(namenode.firstChild().nodeValue().simplified().toUtf8().data());

    namenode = effect.firstChildElement("author");
    if (!namenode.isNull())
        info.append("<br /><strong>" + i18n("Author:") + " </strong>" + i18n(namenode.text().toUtf8().data()));

    namenode = effect.firstChildElement("version");
    if (!namenode.isNull())
        info.append(QString(" (%1)").arg(namenode.text()));

    return info;
}

// static
bool EffectsList::hasKeyFrames(QDomElement effect)
{
    QDomNodeList params = effect.elementsByTagName("parameter");
    for (int i = 0; i < params.count(); i++) {
        QDomElement e = params.item(i).toElement();
        if (e.attribute("type") == "keyframe") return true;
    }
    return false;
}

// static
bool EffectsList::hasSimpleKeyFrames(QDomElement effect)
{
    QDomNodeList params = effect.elementsByTagName("parameter");
    for (int i = 0; i < params.count(); i++) {
        QDomElement e = params.item(i).toElement();
        if (e.attribute("type") == "simplekeyframe") return true;
    }
    return false;
}

// static
bool EffectsList::hasGeometryKeyFrames(QDomElement effect)
{
    QDomNodeList params = effect.elementsByTagName("parameter");
    for (int i = 0; i < params.count(); ++i) {
        QDomElement param = params.item(i).toElement();
        if (param.attribute("type") == "geometry" && !param.hasAttribute("fixed"))
            return true;
    }
    return false;
}

void EffectsList::clone(const EffectsList original)
{
    setContent(original.toString());
    m_baseElement = documentElement();
}

void EffectsList::clearList()
{
    while (!m_baseElement.firstChild().isNull())
        m_baseElement.removeChild(m_baseElement.firstChild());
}

// static
void EffectsList::setParameter(QDomElement effect, const QString &name, const QString &value)
{
    QDomNodeList params = effect.elementsByTagName("parameter");
    for (int i = 0; i < params.count(); i++) {
        QDomElement e = params.item(i).toElement();
        if (e.attribute("name") == name) {
            e.setAttribute("value", value);
            break;
        }
    }
}

// static
QString EffectsList::parameter(QDomElement effect, const QString &name)
{
    QDomNodeList params = effect.elementsByTagName("parameter");
    for (int i = 0; i < params.count(); i++) {
        QDomElement e = params.item(i).toElement();
        if (e.attribute("name") == name) {
            return e.attribute("value");
        }
    }
    return QString();
}

// static
void EffectsList::setProperty(QDomElement effect, const QString &name, const QString &value)
{
    QDomNodeList params = effect.elementsByTagName("property");
    // Update property if it already exists
    for (int i = 0; i < params.count(); i++) {
        QDomElement e = params.item(i).toElement();
        if (e.attribute("name") == name) {
            e.firstChild().setNodeValue(value);
            break;
        }
    }
}

// static
void EffectsList::renameProperty(QDomElement effect, const QString &oldName, const QString &newName)
{
    QDomNodeList params = effect.elementsByTagName("property");
    // Update property if it already exists
    for (int i = 0; i < params.count(); i++) {
        QDomElement e = params.item(i).toElement();
        if (e.attribute("name") == oldName) {
            e.setAttribute("name", newName);
            break;
        }
    }
}

// static
QString EffectsList::property(QDomElement effect, const QString &name)
{
    QDomNodeList params = effect.elementsByTagName("property");
    for (int i = 0; i < params.count(); i++) {
        QDomElement e = params.item(i).toElement();
        if (e.attribute("name") == name) {
            return e.firstChild().nodeValue();
        }
    }
    return QString();
}

// static
void EffectsList::removeProperty(QDomElement effect, const QString &name)
{
    QDomNodeList params = effect.elementsByTagName("property");
    for (int i = 0; i < params.count(); i++) {
        QDomElement e = params.item(i).toElement();
        if (e.attribute("name") == name) {
            effect.removeChild(params.item(i));
            break;
        }
    }
}

void EffectsList::append(QDomElement e)
{
    m_baseElement.appendChild(importNode(e, true));
}

int EffectsList::count() const
{
    return m_baseElement.childNodes().count();
}

bool EffectsList::isEmpty() const
{
    return m_baseElement.childNodes().count() == 0;
}

const QDomElement EffectsList::at(int ix) const
{
    QDomNodeList effects = m_baseElement.childNodes();
    if (ix < 0 || ix >= effects.count()) return QDomElement();
    return effects.at(ix).toElement();
}

void EffectsList::removeAt(int ix)
{
    QDomNodeList effects = m_baseElement.childNodes();
    if (ix < 0 || ix >= effects.count()) return;
    m_baseElement.removeChild(effects.at(ix));
}

QDomElement EffectsList::item(int ix)
{
    QDomNodeList effects = m_baseElement.childNodes();
    if (ix < 0 || ix >= effects.count()) return QDomElement();
    return effects.at(ix).toElement();
}

void EffectsList::insert(int ix, QDomElement effect)
{
    QDomNodeList effects = m_baseElement.childNodes();
    if (ix < 0) ix = 0;
    if (ix >= effects.count()) m_baseElement.appendChild(importNode(effect, true));
    else m_baseElement.insertBefore(importNode(effect, true), effects.at(ix));
}

void EffectsList::replace(int ix, QDomElement effect)
{
    QDomNodeList effects = m_baseElement.childNodes();
    if (ix < 0) ix = 0;
    if (ix < effects.count()) m_baseElement.removeChild(effects.at(ix));
    if (ix == effects.count()) m_baseElement.appendChild(importNode(effect, true));
    else m_baseElement.insertBefore(importNode(effect, true), effects.at(ix));
}
