#include "cchoicebubble.h"

#include <QtMath>
#include <QFontMetrics>

#include "Misc/Palette/cpaletteaction.h"

#include "Misc/Bubbles/cchoicemodel.h"
#include "cchoice.h"

CChoiceBubble::CChoiceBubble(const QPointF &pos, CPaletteAction *palette, const QFont &font, QGraphicsItem *parent)
    : CBubble(pos, palette, font, parent)
{
    m_type = Chronicler::ChoiceBubble;

    m_choices = new CChoiceModel(choiceList(), this);
    connect(m_choices, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(ModelUpdated()));
    connect(m_choices, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(ModelUpdated()));
    connect(m_choices, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(ModelUpdated()));
    connect(m_choices, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(ModelUpdated()));
    connect(m_choices, SIGNAL(modelReset()),
            this, SLOT(ModelUpdated()));

    AdjustMinSize();
    m_bounds = QRectF(-m_minSize.width() / 2, -m_minSize.height() / 2, m_minSize.width(), m_minSize.height());
    UpdatePolygon();

    setPalette(m_paletteAction);
}

void CChoiceBubble::setPalette(CPaletteAction *palette)
{
    CBubble::setPalette(palette);
    for(CChoice *choice : m_choices->choices())
        choice->setPalette(palette);
}

void CChoiceBubble::setFont(const QFont &font)
{
    CBubble::setFont(font);
    AdjustMinSize();
    UpdatePolygon();
}


// Links are not attached directly, but rather to the CChoice sub-bubbles.
void CChoiceBubble::AddLink(CConnection *)
{}
void CChoiceBubble::RemoveLink(CConnection *)
{}
void CChoiceBubble::RemoveLink(Chronicler::Anchor)
{}

QList<CConnection *> CChoiceBubble::links()
{
    QList<CConnection *> links;

    for(CChoice *choice : m_choices->choices())
        links.append(choice->links());

    return links;
}

CChoiceModel *CChoiceBubble::choices()
{
    return m_choices;
}

CChoiceBubble::choiceList CChoiceBubble::choiceBubbles()
{
    return m_choices->choices();
}

void CChoiceBubble::UpdatePolygon()
{
    m_bounds.setHeight(qMax(m_bounds.height(), m_minSize.height()));
    CBubble::UpdatePolygon();

    choiceList choices = m_choices->choices();

    qreal height = 0;
    if(choices.length() > 0)
        height = boundingRect().center().y() - (choices.length() * (choices[0]->boundingRect().height() + 2)) / 2;

    for(CChoice *choice : choices)
    {
        choice->setWidth(boundingRect().width() - 5);
        choice->setPos(m_bounds.x() + 2, height);
        height += choice->boundingRect().height() + 2;
    }
}

void CChoiceBubble::AdjustMinSize()
{
    choiceList choices = m_choices->choices();

    qreal height = 0;
    if(choices.length() > 0)
        height = (choices.length() * (choices[0]->boundingRect().height() + 2));

    height += QFontMetrics(m_font).height() * 2;

    m_minSize.setHeight(height);
}

void CChoiceBubble::ModelUpdated()
{
    AdjustMinSize();
    UpdatePolygon();
    emit PositionOrShapeChanged();
}


Chronicler::Anchor CChoiceBubble::OutputAnchorAtPosition(const QPointF &)
{
    return Chronicler::NoAnchor;
}

Chronicler::Anchor CChoiceBubble::InputAnchorAtPosition(const QPointF &pos)
{
    if(pos.y() > sceneBoundingRect().center().y())
        return Chronicler::SouthAnchor;

    return Chronicler::NorthAnchor;
}


QDataStream &CChoiceBubble::Deserialize(QDataStream &ds, const Chronicler::CVersion &version)
{
    CBubble::Deserialize(ds, version);

    qint64 num_choices;
    ds >> num_choices;

    for(int i = 0; i < num_choices; ++i)
    {
        CChoice *choice = new CChoice(m_paletteAction, m_font, this);
        ds >> *choice;
        m_choices->AddItem(choice);
    }

    setPalette(m_paletteAction);

    return ds;
}

QDataStream & CChoiceBubble::Serialize(QDataStream &ds) const
{
    CBubble::Serialize(ds);

    QList<CChoice *> tmp = m_choices->choices();

    ds << static_cast<qint64>(tmp.length());
    for(CChoice *choice : tmp)
        ds << *choice;

    return ds;
}


void CChoiceBubble::UpdateUID()
{
    CBubble::UpdateUID();

    for(CChoice *choice : m_choices->choices())
        choice->UpdateUID();
}
