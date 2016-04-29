#ifndef CBUBBLE_H
#define CBUBBLE_H


#include <QObject>
#include <QGraphicsPolygonItem>

#include <QFont>

QT_BEGIN_NAMESPACE
class QGraphicsItem;
class QGraphicsSceneMouseEvent;
class QStyleOptionGraphicsItem;
class QPolygonF;
QT_END_NAMESPACE

#include "Misc/chronicler.h"
using Chronicler::CPalette;
using Chronicler::BubbleType;

class CConnection;
class CLink;

class CBubble : public QObject, public QGraphicsPolygonItem
{
    Q_OBJECT

public:

    CBubble(QGraphicsItem *parent);
    CBubble(QMenu *contextMenu, const QPointF &pos, const CPalette &palette, const QFont &font = QFont(), QGraphicsItem *parent = 0);


    virtual void setLabel(QString label) { m_label = label; }
    QString getLabel() const { return m_label; }

    void setOrder(int order) { m_order = order; }
    int getOrder() const { return m_order; }

    void setLocked(bool locked) { m_locked = locked; }
    bool getLocked() const { return m_locked; }

    virtual void setFont(const QFont &font);
    QFont getFont() const { return m_font; }

    virtual void setPalette(const CPalette & palette);
    const CPalette & getPalette() const { return m_palette; }

    BubbleType getType() const { return m_type; }

    void AddConnection(CConnection *connection);
    void RemoveConnection(CConnection *connection);

    virtual void AddLink(CConnection *link, int position) = 0;
    virtual void RemoveLink(CConnection *link) = 0;

    virtual QList<CConnection *> links() { return {}; }

protected:
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *evt);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *evt);
    
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);

    virtual void UpdatePolygon() = 0;


    BubbleType m_type;

    QMenu *m_contextMenu;

    QRectF m_bounds;
    QSizeF m_minSize;

    QList<CConnection *> m_connections;

    QString m_label;
    int m_order;
    bool m_locked;

    QFont m_font;
    CPalette m_palette;
    
signals:
    void Selected(QGraphicsItem *item);
    void ConnectionsChanged(int);
};

#endif // CBUBBLE_H
