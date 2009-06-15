/*****************************************************************************
 * Copyright (C) 2006 Shie Erlich <erlich@users.sourceforge.net>             *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef TSTRING_H
#define TSTRING_H

#include <utility>
#include <cassert>

#include <QtCore/QString>
#include <QtCore/QList>

template <class T>
class TagString_t
{
    QString str;
    typedef QList<std::pair<uint, T> > taglist; // may change
    taglist tags;
public:
    TagString_t(const QString& s) : str(s) {}
    TagString_t() {}
    bool isSimple() const {
        return tags.empty();
    }
    const QString& string() const {
        return str;
    }
    unsigned length() const {
        return str.length();
    }
    TagString_t mid(unsigned idx, unsigned len = ~0) const;
    TagString_t left(unsigned) const;
    TagString_t right(unsigned) const;
    void insert(uint, const QString& s);
    int find(QChar c, int index = 0, bool cs = true) const {
        return str.indexOf(c, index, (cs ? Qt::CaseSensitive : Qt::CaseInsensitive));
    }
    TagString_t& operator+=(const TagString_t& s);
    typename taglist::const_iterator tagsBegin() const {
        return tags.begin();
    }
    typename taglist::const_iterator tagsEnd() const {
        return tags.end();
    }
    typename taglist::iterator tagsBegin() {
        return tags.begin();
    }
    typename taglist::iterator tagsEnd() {
        return tags.end();
    }
    void insertTag(uint pos, const T& t);
    void eraseTag(const typename taglist::iterator& which) {
        tags.erase(which);
    }
    /* void insert(uint idx,const QString&);
     void insert(uint idx,const char*);
     void addTag(uint);
     void remove(uint start,uint len);*/
};

template <class T>
TagString_t<T> TagString_t<T>::mid(unsigned idx, unsigned len) const
{
    TagString_t ret(str.mid(idx, len));
    unsigned max = idx + len;
    if (max < idx) max = ~0;
    for (typename taglist::const_iterator it = tags.begin(), end = tags.end();it != end;++it) {
        if ((*it).first >= idx && (*it).first < max)
            ret.tags.push_back(*it);
    }
    return ret;
}

template <class T>
TagString_t<T> TagString_t<T>::left(unsigned len) const
{
    TagString_t ret(str.left(len));
    for (typename taglist::const_iterator it = tags.begin(), end = tags.end();it != end;++it) {
        if ((*it).first < len)
            ret.tags.push_back(*it);
    }
    return ret;
}

template <class T>
TagString_t<T> TagString_t<T>::right(unsigned len) const
{
    TagString_t ret(str.right(len));
    for (typename taglist::const_iterator it = tags.begin(), end = tags.end();it != end;++it) {
        if ((*it).first >= str.length() - len)
            ret.tags.push_back(*it);
    }
    return ret;
}

template <class T>
void TagString_t<T>::insert(uint idx, const QString& s)
{
    str.insert(idx, s);
    const unsigned disp = s.length();
    for (typename taglist::iterator it = tags.begin(), end = tags.end();it != end;++it) {
        if ((*it).first >= idx)
            (*it).first += disp;
    }
}

template <class T>
TagString_t<T>& TagString_t<T>::operator+=(const TagString_t & s)
{
    str += s.str;
    const unsigned disp = length();
    for (typename taglist::const_iterator it = s.tags.begin(), end = s.tags.end();it != end;++it) {
        tags.push_back(make_pair((*it).first + disp, (*it).second));
    }
    return *this;
}

template <class T>
void TagString_t<T>::insertTag(uint pos, const T& t)
{
    assert(pos <= length());
    tags.push_back(make_pair(pos, t));
}

#endif
