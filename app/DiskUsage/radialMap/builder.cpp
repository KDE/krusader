/*
    SPDX-FileCopyrightText: 2003-2004 Max Howell <max.howell@methylblue.com>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "builder.h"
#include "Config.h"
#include "widget.h"

// QtCore
#include <QLocale>

#include <KLocalizedString>

//**** REMOVE NEED FOR the +1 with MAX_RING_DEPTH uses
//**** add some angle bounds checking (possibly in Segment ctor? can I delete in a ctor?)
//**** this class is a mess

RadialMap::Builder::Builder(RadialMap::Map *m, const Directory *const d, bool fast)
    : m_map(m)
    , m_root(d)
    , m_minSize(static_cast<FileSize>(static_cast<long double>(d->size() * 3) / (PI * m->height() - m->MAP_2MARGIN)))
    , m_depth(&m->m_visibleDepth)
{
    m_signature = new Chain<Segment>[*m_depth + 1];

    if (!fast) { //|| *m_depth == 0 ) //depth 0 is special case usability-wise //**** WHY?!
        // determine depth rather than use old one
        findVisibleDepth(d); // sets m_depth
    }

    m_map->setRingBreadth();
    setLimits(m_map->m_ringBreadth);
    build(d);

    m_map->m_signature = m_signature;

    delete[] m_limits;
}

void RadialMap::Builder::findVisibleDepth(const Directory *const dir, const unsigned int depth)
{
    //**** because I don't use the same minimumSize criteria as in the visual function
    //     this can lead to incorrect visual representation
    //**** BUT, you can't set those limits until you know m_depth!

    //**** also this function doesn't check to see if anything is actually visible
    //     it just assumes that when it reaches a new level everything in it is visible
    //     automatically. This isn't right especially as there might be no files in the
    //     dir provided to this function!

    static uint stopDepth = 0;

    if (dir == m_root) {
        stopDepth = *m_depth;
        *m_depth = 0;
    }

    if (*m_depth < depth)
        *m_depth = depth;
    if (*m_depth >= stopDepth)
        return;

    for (ConstIterator<File> it = dir->constIterator(); it != dir->end(); ++it)
        if ((*it)->isDir() && (*it)->size() > m_minSize)
            findVisibleDepth(dynamic_cast<const Directory *>(*it), depth + 1); // if no files greater than min size the depth is still recorded
}

void RadialMap::Builder::setLimits(const uint &b) // b = breadth?
{
    long double size3 = m_root->size() * 3;
    long double pi2B = PI * 2 * b;

    m_limits = new FileSize[*m_depth + 1]; // FIXME delete!

    for (unsigned int d = 0; d <= *m_depth; ++d)
        m_limits[d] = static_cast<FileSize>(size3 / (pi2B * (d + 1))); // min is angle that gives 3px outer diameter for that depth
}

//**** segments currently overlap at edges (i.e. end of first is start of next)
bool RadialMap::Builder::build(const Directory *const dir, const unsigned int depth, unsigned int a_start, const unsigned int a_end)
{
    // first iteration: dir == m_root

    if (dir->fileCount() == 0) // we do fileCount rather than size to avoid chance of divide by zero later
        return false;

    FileSize hiddenSize = 0;
    uint hiddenFileCount = 0;

    for (ConstIterator<File> it = dir->constIterator(); it != dir->end(); ++it) {
        if ((*it)->size() > m_limits[depth]) {
            auto a_len = (unsigned int)(5760 * ((double)(*it)->size() / (double)m_root->size()));

            auto *s = new Segment(*it, a_start, a_len);

            (m_signature + depth)->append(s);

            if ((*it)->isDir()) {
                if (depth != *m_depth) {
                    // recurse
                    s->m_hasHiddenChildren = build(dynamic_cast<const Directory *>(*it), depth + 1, a_start, a_start + a_len);
                } else
                    s->m_hasHiddenChildren = true;
            }

            a_start += a_len; //**** should we add 1?

        } else {
            hiddenSize += (*it)->size();

            if ((*it)->isDir()) //**** considered virtual, but dir wouldn't count itself!
                hiddenFileCount += dynamic_cast<const Directory *>(*it)->fileCount(); // need to add one to count the dir as well

            ++hiddenFileCount;
        }
    }

    if (hiddenFileCount == dir->fileCount() && !Config::showSmallFiles)

        return true;

    else if ((Config::showSmallFiles && hiddenSize > m_limits[depth]) || (depth == 0 && (hiddenSize > dir->size() / 8)) /*|| > size() * 0.75*/) {
        // append a segment for unrepresented space - a "fake" segment

        const QString s = i18np("%1 file: ~ %2", "%1 files: ~ %2", QLocale().toString(hiddenFileCount), File::humanReadableSize(hiddenSize / hiddenFileCount));
        (m_signature + depth)->append(new Segment(new File(s, hiddenSize), a_start, a_end - a_start, true));
    }

    return false;
}
