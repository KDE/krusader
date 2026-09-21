/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Max Howell <max.howell@methylblue.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BUILDER_H
#define BUILDER_H

#include "fileTree.h"
#include "radialMap.h" //Segment, defines

template<class T>
class Chain;

namespace RadialMap
{
class Map;

// temporary class that builds the Map signature

class Builder
{
public:
    Builder(Map *, const Directory *const, bool fast = false);

private:
    void findVisibleDepth(const Directory *const dir, const uint = 0);
    void setLimits(const uint &);
    bool build(const Directory *const, const uint = 0, uint = 0, const uint = 5760);

    Map *m_map;
    const Directory *const m_root;
    const FileSize m_minSize;
    uint *m_depth;
    Chain<Segment> *m_signature;
    FileSize *m_limits;
};
}

#endif
