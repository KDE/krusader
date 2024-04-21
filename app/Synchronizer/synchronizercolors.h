/*
    SPDX-FileCopyrightText: 2016-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SYNCHRONIZERCOLORS_H
#define SYNCHRONIZERCOLORS_H

#include <KColorUtils>

#define DECLARE_SYNCHRONIZER_BACKGROUND_DEFAULTS                                                                                                               \
    QColor SYNCHRONIZER_BACKGROUND_DEFAULTS[] = {QGuiApplication::palette().color(QPalette::Active, QPalette::Base),                                           \
                                                 QGuiApplication::palette().color(QPalette::Active, QPalette::Base),                                           \
                                                 QGuiApplication::palette().color(QPalette::Active, QPalette::Base),                                           \
                                                 QGuiApplication::palette().color(QPalette::Active, QPalette::Base),                                           \
                                                 KColorUtils::tint(QGuiApplication::palette().color(QPalette::Active, QPalette::Base), Qt::red, 0.5)}

#define DECLARE_SYNCHRONIZER_FOREGROUND_DEFAULTS                                                                                                               \
    QColor SYNCHRONIZER_FOREGROUND_DEFAULTS[] = {                                                                                                              \
        QGuiApplication::palette().color(QPalette::Active, QPalette::Text),                                                                                    \
        KColorUtils::tint(QGuiApplication::palette().color(QPalette::Active, QPalette::Text), Qt::red, 0.7),                                                   \
        KColorUtils::tint(QGuiApplication::palette().color(QPalette::Active, QPalette::Text), QColor(0, 115, 207), 0.7),                                       \
        KColorUtils::tint(QGuiApplication::palette().color(QPalette::Active, QPalette::Text), Qt::green, 0.5),                                                 \
        QGuiApplication::palette().color(QPalette::Active, QPalette::Text)}

#endif /* __SYNCHRONIZERCOLORS_H__ */
