/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include <QGraphicsScene>

QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
class QMenu;
class QPointF;
class QGraphicsLineItem;
class QFont;
class QGraphicsTextItem;
class QColor;
class QWheelEvent;
QT_END_NAMESPACE

class CBubble;
class CLink;
class CLine;

#include "Misc/chronicler.h"
using Chronicler::CPalette;
using Chronicler::BubbleType;

//! [0]
class CGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum Mode { InsertItem, InsertLine, InsertText, Cursor, InsertStory, InsertCondition, InsertChoice };

    explicit CGraphicsScene(QMenu *itemMenu, QObject *parent = 0);

    void setFont(const QFont &font);
    QFont getFont() const { return m_font; }

    void setPalette(const CPalette &palette);
//    QColor textColor() const { return m_textColor; }
//    QColor itemColor() const { return m_itemColor; }
//    QColor lineColor() const { return m_lineColor; }
//    void setLineColor(const QColor &color);
//    void setTextColor(const QColor &color);
//    void setItemColor(const QColor &color);


    bool isRubberBandSelecting() const { return m_rubberBand; }

public slots:
    void setMode(Mode mode);
    
private slots:
//    void linkClicked(CLink *link);

signals:
    void itemInserted(CBubble *item);
    void itemSelected(QGraphicsItem *item);
    void leftPressed();
    void leftReleased();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

private:
    void AddBubble(BubbleType type, const QPointF &pos);

    QMenu *m_itemMenu;
    Mode m_mode;
    QPointF m_startPoint;
    CLine *m_line;
    QFont m_font;
    CPalette m_palette;
//    QColor m_textColor;
//    QColor m_itemColor;
//    QColor m_lineColor;
    bool m_rubberBand;
};





#endif // DIAGRAMSCENE_H