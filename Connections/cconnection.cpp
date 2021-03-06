#include "cconnection.h"

#include <QPointF>
#include <QtMath>

#include "cline.h"
#include "Bubbles/cbubble.h"

#include "Properties/cprojectview.h"
#include "cgraphicsview.h"
#include "cgraphicsscene.h"

#include "Misc/chronicler.h"
using Chronicler::shared;

CConnection::CConnection(QGraphicsScene *scn)
    : CConnection(0, 0, Anchor::SouthAnchor, Anchor::NorthAnchor, scn)
{}

CConnection::CConnection(CBubble *from, CBubble *to, Anchor anc_from, Anchor anc_to, QGraphicsScene *scn)
    : m_from(0), m_to(0), m_fromUID(0), m_toUID(0), m_connected(false)
{
    m_line = new CLine(QPointF(), QPointF(), anc_from, anc_to);
    m_line->setZValue(-999999);
    scn->addItem(m_line);

    setFrom(from);
    setTo(to);
}

CConnection::~CConnection()
{
    setFrom(0);
    setTo(0);

    delete m_line;
}

CBubble *CConnection::from() const
{
    return m_from;
}

void CConnection::setFrom(CBubble *from)
{
    if(m_from)
    {
        disconnect(m_from, SIGNAL(PositionOrShapeChanged()), this, SLOT(UpdatePosition()));
        disconnect(m_from, SIGNAL(PaletteChanged()), this, SLOT(FromPaletteChanged()));
        m_from->RemoveLink(this);
    }

    m_from = from;
    if(m_from)
    {
        m_from->AddLink(this);
        connect(m_from, SIGNAL(PositionOrShapeChanged()), this, SLOT(UpdatePosition()));
        connect(m_from, SIGNAL(PaletteChanged()), this, SLOT(FromPaletteChanged()));

        m_line->setPalette(m_from->getPaletteForAnchor(startAnchor()));
    }

    UpdatePosition();
}

CBubble *CConnection::to() const
{
    return m_to;
}

void CConnection::setTo(CBubble *to)
{
    if(m_to)
    {
        disconnect(m_to, SIGNAL(PositionOrShapeChanged()), this, SLOT(UpdatePosition()));
        m_to->RemoveConnection(this);
    }

    m_to = to;
    if(m_to)
    {
        m_to->AddConnection(this);
        connect(m_to, SIGNAL(PositionOrShapeChanged()), this, SLOT(UpdatePosition()));
    }

    UpdatePosition();
}

CPaletteAction *CConnection::getPalette()
{
    return m_line->getPalette();
}

void CConnection::UpdatePosition()
{
    if(m_from)
    {
        qreal angle = (startAnchor() * M_PI * 0.5);
        QPointF pos = m_from->sceneBoundingRect().center();
        QSizeF size = m_from->sceneBoundingRect().size();
        m_line->setStart(pos + QPointF(size.width() * 0.5 * qCos(angle),
                                       size.height() * 0.5 * qSin(angle)));
    }
    if(m_to)
    {
        qreal angle = (endAnchor() * M_PI * 0.5);
        QPointF pos = m_to->sceneBoundingRect().center();
        QSizeF size = m_to->sceneBoundingRect().size();
        m_line->setEnd(pos + QPointF(size.width() * 0.5 * qCos(angle),
                                     size.height() * 0.5 * qSin(angle)));
    }
}

void CConnection::FromPaletteChanged()
{
    m_line->setPalette(m_from->getPaletteForAnchor(startAnchor()));
}

QRectF CConnection::boundingRect() const
{
    return m_line->boundingRect();
}

void CConnection::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{}

Anchor CConnection::startAnchor() const
{
    return m_line->startAnchor();
}

void CConnection::setStartAnchor(Chronicler::Anchor anchor)
{
    m_line->setStartAnchor(anchor);
}

Anchor CConnection::endAnchor() const
{
    return m_line->endAnchor();
}

void CConnection::setEndAnchor(Chronicler::Anchor anchor)
{
    m_line->setEndAnchor(anchor);
}

/**
 * @brief Connects this object with the two bubbles containing the
 *        UID's read during deserialization
 *
 *        To be called only after all bubbles have been instantiated
 */
void CConnection::ConnectToUIDs(bool paste)
{
    CBubble *from, *to;

    if(paste)
    {
        from = shared().projectView->BubbleWithUID(m_fromUID, m_line->scene());
        to = shared().projectView->BubbleWithUID(m_toUID, m_line->scene());
    }
    else
    {
        from = shared().projectView->BubbleWithUID(m_fromUID);
        to = shared().projectView->BubbleWithUID(m_toUID);
    }

    if(from && to)
    {
        setFrom(from);
        setTo(to);
        m_connected = true;
    }
    else
    {
        if(from)
            from->RemoveLink(startAnchor());
        else if(m_line->scene())
        {
            static_cast<CGraphicsScene *>(m_line->scene())->RemoveConnection(this);
            deleteLater();
        }
    }
}

/**
 * @brief Deserializes this object in the format specified by version
 * @param stream The QDataStream to extract the data from
 * @param version The version of the program the data was created with
 * @return The modified datastream for chaining
 */
QDataStream & CConnection::Deserialize(QDataStream &stream, const Chronicler::CVersion &version)
{
    Q_UNUSED(version)

    qint32 start, end;

    stream >> start
            >> end
            >> m_fromUID
            >> m_toUID;

    m_line->setStartAnchor(Anchor(start));
    m_line->setEndAnchor(Anchor(end));
    m_connected = false;

    return stream;
}

/**
 * @brief Serializes this object with the current program version
 * @param stream The QDataStream to insert the data into
 * @return The modified datastream for chaining
 */
QDataStream & CConnection::Serialize(QDataStream &stream) const
{
    stream << static_cast<qint32>(m_line->startAnchor())
           << static_cast<qint32>(m_line->endAnchor())
           << m_from->GenerateUID()
           << m_to->GenerateUID();

    return stream;
}

CLine *CConnection::getLine() const
{
    return m_line;
}

bool CConnection::isConnected() const
{
    return m_connected;
}

