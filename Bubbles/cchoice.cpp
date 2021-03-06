#include "cchoice.h"

#include <QFontMetrics>
#include <QPainterPath>

#include "Misc/ctextitem.h"

#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

#include "Misc/Palette/cpaletteaction.h"

#include "Connections/cconnection.h"
#include "cgraphicsscene.h"

CChoice::CChoice(CPaletteAction *palette, const QFont &font, QGraphicsItem *parent, const QString &choice)
    : CSingleLinkBubble(QPointF(), palette, font, parent)
{
    m_type = Chronicler::ChoiceItemBubble;

    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);

    connect(qgraphicsitem_cast<CBubble *>(parent), SIGNAL(PositionOrShapeChanged()), this, SIGNAL(PositionOrShapeChanged()));

    m_choice = new CTextItem(choice, m_bounds, this);
//    m_choice->SetStyle(Qt::AlignCenter);

    AdjustMinSize();
    m_bounds = QRectF(0, 0, m_minSize.width(), m_minSize.height());
    UpdatePolygon();

    setPalette(m_paletteAction);
}

void CChoice::setPalette(CPaletteAction *palette)
{
    CSingleLinkBubble::setPalette(palette);
    m_choice->setColor(palette->getPalette().font);
}

void CChoice::setFont(const QFont &font)
{
    CSingleLinkBubble::setFont(font);
    m_choice->setFont(font);
    AdjustMinSize();
    UpdatePolygon();
}

void CChoice::setChoice(const QString &choice)
{
    m_choice->setText(choice);
}

QString CChoice::text() const
{
    return m_choice->Text();
}

void CChoice::setWidth(const qreal &width)
{
    m_bounds.setWidth(width);

    UpdatePolygon();
}

void CChoice::UpdatePolygon()
{
    m_choice->Resize(m_bounds);

    QPainterPath path;
    path.addRect(m_bounds);

    setPolygon(path.toFillPolygon());
}

void CChoice::AdjustMinSize()
{
    QFontMetrics fm(m_font);
    m_minSize.setHeight(fm.height() + 8);
}


void CChoice::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    event->ignore();
}

void CChoice::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    CSingleLinkBubble::mousePressEvent(event);
    event->ignore();
}

void CChoice::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    CSingleLinkBubble::mouseMoveEvent(event);
    event->ignore();
}

void CChoice::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    CSingleLinkBubble::hoverMoveEvent(event);
    event->ignore();
}

void CChoice::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    CSingleLinkBubble::mouseReleaseEvent(event);
    event->ignore();
}

Chronicler::Anchor CChoice::OutputAnchorAtPosition(const QPointF &pos)
{
    if(pos.x() < sceneBoundingRect().center().x())
        return Chronicler::WestAnchor;

    return Chronicler::EastAnchor;
}

// Inputs are applied to the CChoiceBubble that contains this choice.
Chronicler::Anchor CChoice::InputAnchorAtPosition(const QPointF &)
{
    return Chronicler::NoAnchor;
}

CBubble *CChoice::container()
{
    return dynamic_cast<CBubble *>(parentItem());
}

QDataStream & CChoice::Deserialize(QDataStream &ds, const Chronicler::CVersion &version)
{
    Q_UNUSED(version)

    QString choice;
    bool linked;

    ds >> m_UID >> choice >> linked;

    if(linked)
    {
        m_link = dynamic_cast<CGraphicsScene *>(scene())->AddConnection();
        ds >> *m_link;
    }

    m_choice->setText(choice);

    setPalette(m_paletteAction);

    return ds;
}

QDataStream & CChoice::Serialize(QDataStream &ds) const
{
    ds << GenerateUID() << m_choice->Text() << bool(m_link);

    if(m_link)
        ds << *m_link;

    return ds;
}


