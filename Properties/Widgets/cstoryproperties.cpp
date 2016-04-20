#include "cstoryproperties.h"

#include <QLabel>

#include "Misc/ctextedit.h"
#include "Bubbles/cstorybubble.h"



CStoryProperties::CStoryProperties(QStringListModel *model, QWidget *parent)
    : CPropertiesWidget(model, parent), m_storyBubble(0), m_storyEdit(0)
{

    // Story Widget
    QLabel *lblStory = new QLabel(tr("Story:"), this);
    m_storyEdit = new CTextEdit(this, m_model);
    connect(m_storyEdit, SIGNAL(textChanged()), this, SLOT(StoryChanged()));

    // Story Layout
    QVBoxLayout *lStory = new QVBoxLayout();
    lStory->addWidget(lblStory);
    lStory->addWidget(m_storyEdit);

    m_layout->addLayout(lStory);

    setBubble(0);
}


void CStoryProperties::setBubble(CBubble *bbl)
{
    CPropertiesWidget::setBubble(bbl);

    m_storyBubble = qgraphicsitem_cast<CStoryBubble *>(bbl);
    if(m_storyBubble)
    {
        m_storyEdit->setText(m_storyBubble->getStory());
        m_storyEdit->setFont(m_storyBubble->getFont());
    }
    else
    {
        m_storyEdit->setText(tr(""));
    }
}


void CStoryProperties::setFont(const QFont &font)
{
    CPropertiesWidget::setFont(font);
    m_storyEdit->setFont(font);
}


void CStoryProperties::StoryChanged()
{
    if(m_storyBubble)
        m_storyBubble->setStory(m_storyEdit->toPlainText());
}
