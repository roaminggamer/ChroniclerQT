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

#include "Misc/cserializable.h"

#include "Misc/chronicler.h"
using Chronicler::CPalette;
using Chronicler::Anchor;
using Chronicler::BubbleType;
using Chronicler::t_uid;

class CPaletteAction;
class CConnection;
class CLink;


class CBubble : public QObject, public QGraphicsPolygonItem, public CSerializable
{
    Q_OBJECT

public:
    CBubble(const QPointF &pos, CPaletteAction *palette, const QFont &font = QFont(), QGraphicsItem *parent = 0);

    virtual ~CBubble();

    virtual void setLabel(QString label) { m_label = label; }
    QString getLabel() const { return m_label; }

    void setOrder(qint64 order) { m_order = order; }
    qint64 getOrder() const { return m_order; }

    void setLocked(bool locked) { m_locked = locked; }
    bool getLocked() const { return m_locked; }

    virtual void setFont(const QFont &font);
    QFont getFont() const { return m_font; }

    virtual void setPalette(CPaletteAction *palette);
    CPaletteAction *getPaletteAction();

    BubbleType getType() const { return m_type; }

    void setBounds(const QRectF &bounds);

    virtual CPaletteAction *getPaletteForAnchor(Anchor anchor);

    /// @brief if this bubble is contained inside another bubble, return that else return itself
    virtual CBubble * container();

    void AddConnection(CConnection *connection);
    void RemoveConnection(CConnection *connection);
    QList<CConnection *> connections();

    virtual void AddLink(CConnection *link) = 0;
    virtual void RemoveLink(CConnection *link) = 0;
    virtual void RemoveLink(Anchor anchor) = 0;
    virtual QList<CConnection *> links() = 0;

    virtual Anchor OutputAnchorAtPosition(const QPointF &pos);
    virtual Anchor InputAnchorAtPosition(const QPointF &pos);

    t_uid getUID();
    virtual void UpdateUID();
    t_uid GenerateUID() const;



protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *evt) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *evt) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *evt) override;

    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *evt) override;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);

    virtual void UpdatePolygon();

    virtual QDataStream &Deserialize(QDataStream &ds, const CVersion &version) override;
    virtual QDataStream &Serialize(QDataStream &ds) const override;

    // for backwards compatability
    static void AddUID(Chronicler::t_uid uid);

    t_uid m_UID;

    BubbleType m_type;

    QRectF m_bounds;
    QSizeF m_minSize;

    QList<CConnection *> m_connections;

    QString m_label;
    qint64 m_order;
    bool m_locked;

    QFont m_font;
    CPaletteAction *m_paletteAction;

    bool m_resize;
    QPointF m_offset;
    QRectF m_lastBounds;

    QPointF m_oldPos;

private:
    Anchor AnchorAtPosition(const QPointF &pos);

    // for backwards compatability
    static QList<t_uid> m_UIDs;
    
signals:
    void Selected(QGraphicsItem *item);
    void ConnectionsChanged(int);
    void PositionOrShapeChanged();
    void PositionChanged(const QPointF &oldPos, const QPointF &newPos);
    void ShapeChanged(const QRectF &oldRect, const QRectF &newRect);

    void PaletteChanged();

private slots:
    void UpdatePalette();

};

#endif // CBUBBLE_H
