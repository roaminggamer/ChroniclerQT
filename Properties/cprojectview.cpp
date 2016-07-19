#include "cprojectview.h"

#include <QFileInfo>

#include <QListView>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

#include <QMessageBox>

#include <QDockWidget>
#include <QToolBar>
#include <QTabWidget>

#include <QStatusBar>

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QFile>
#include <QSaveFile>
#include <QFileDialog>
#include <QTextStream>
#include <QAction>

#include <QStack>

#include <QSettings>

#include <QtAlgorithms>
#include <QtMath>

#include <QTimer>

#include "Bubbles/cchoice.h"
#include "Misc/cscenemodel.h"
#include "cgraphicsscene.h"
#include "cgraphicsview.h"
#include "chomepage.h"
#include "cmainwindow.h"

#include "Bubbles/cbubble.h"
#include "Bubbles/cstartbubble.h"
#include "Bubbles/cstorybubble.h"
#include "Bubbles/cchoicebubble.h"
#include "Misc/cchoicemodel.h"
#include "Bubbles/cactionbubble.h"
#include "Misc/cstringlistmodel.h"
#include "Bubbles/cconditionbubble.h"
#include "Connections/cconnection.h"


#include "Misc/choicescriptdata.h"

#include "Properties/cindentselectiondialog.h"

#include "Misc/cpalettebutton.h"

#include "csettingsview.h"

#include "Properties/cvariablesview.h"
#include "Misc/cvariablesmodel.h"

#include "Misc/chronicler.h"
using Chronicler::shared;


Q_DECLARE_METATYPE(QStringList)


CProjectView::CProjectView(QWidget *parent)
    : QWidget(parent), m_version(shared().ProgramVersion), m_path("")
{
    m_upButton = new QPushButton(QIcon(":/images/icn_up"), "");
    m_upButton->setToolTip("Move scene up");
    m_upButton->setEnabled(false);
    connect(m_upButton, SIGNAL(clicked(bool)), this, SLOT(MoveUp()));

    m_downButton = new QPushButton(QIcon(":/images/icn_down"), "");
    m_downButton->setToolTip("Move scene down");
    m_downButton->setEnabled(false);
    connect(m_downButton, SIGNAL(clicked(bool)), this, SLOT(MoveDown()));

    m_addButton = new QPushButton(QIcon(":/images/icn_add"), "");
    m_addButton->setToolTip("Add new scene");
    connect(m_addButton, SIGNAL(clicked(bool)), this, SLOT(AddItem()));

    m_removeButton = new QPushButton(QIcon(":/images/icn_trash"), "");
    m_removeButton->setToolTip("<qt>Delete scene.<br>WARNING: this cannot be undone!</qt>");
    m_removeButton->setEnabled(false);
    connect(m_removeButton, SIGNAL(clicked(bool)), this, SLOT(RemoveItem()));

    QVBoxLayout *vl_buttons = new QVBoxLayout();
    vl_buttons->addWidget(m_upButton);
    vl_buttons->addWidget(m_downButton);
    vl_buttons->addWidget(m_addButton);
    vl_buttons->addWidget(m_removeButton);
    vl_buttons->addStretch(1);

    m_sceneModel = new CSceneModel(this);

    m_modelView = new QListView();
    m_modelView->setAlternatingRowColors(true);
    m_modelView->setModel(m_sceneModel);
    m_modelView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    connect(m_modelView, SIGNAL(clicked(QModelIndex)), this, SLOT(SelectedChanged(QModelIndex)));
    connect(m_sceneModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(DataChanged(QModelIndex,QModelIndex)));

    QHBoxLayout *hl_viewButtons = new QHBoxLayout();
    hl_viewButtons->addWidget(m_modelView);
    hl_viewButtons->addLayout(vl_buttons);

    m_title = new QLineEdit();
    connect(m_title, SIGNAL(textChanged(QString)), this, SLOT(ProjectNameChanged()));

    m_author = new QLineEdit();

    QVBoxLayout *l_main = new QVBoxLayout(this);
    l_main->addWidget(new QLabel("Title"));
    l_main->addWidget(m_title);
    l_main->addWidget(new QLabel("Author"));
    l_main->addWidget(m_author);
    l_main->addWidget(new QLabel("Scenes"));
    l_main->addLayout(hl_viewButtons);

    m_autosave_num = 0;

    QTimer::singleShot(shared().settingsView->autosaveInterval(), this, SLOT(Autosave()));
}

void CProjectView::SaveProject()
{
    if(m_sceneModel->rowCount() > 0)
    {
        if(!m_path.length())
            SaveProjectAs();
        else
        {
            // Create necessary directories
            QString dir = QFileInfo(m_path).absolutePath();

            if(!QDir(dir + "/backups").exists())
                QDir().mkdir(dir + "/backups");

            if(!QDir(dir + "/scenes").exists())
                QDir().mkdir(dir + "/scenes");

            // Update order before saving...
            ExportChoiceScript();

            QSaveFile file(m_path);
            SaveToFile(file);

            while(!file.commit())
            {
                QMessageBox msgBox;
                msgBox.setText("Error writing to file!");
                msgBox.setInformativeText("Ensure that the file exists and that you have enough disk space.");
                msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Retry | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Retry);
                int ret = msgBox.exec();

                if(ret == QMessageBox::Save)
                {
                    SaveProjectAs();
                    return;
                }
                else if(ret == QMessageBox::Cancel)
                    return;
            }

            QStringList recent_files = shared().settingsView->settings()->value("Homepage/RecentFiles").value<QStringList>();
            if(!recent_files.contains(m_path))
            {
                recent_files.insert(0, m_path);
                if(recent_files.length() > shared().settingsView->maxRecentFiles())
                    recent_files.removeLast();
            }

            shared().settingsView->settings()->setValue("Homepage/RecentFiles", QVariant::fromValue(recent_files));

            // show status message for 10 seconds
            shared().statusBar->showMessage("Successfully saved to " + m_path + "...", 30000);
        }
    }
}

void CProjectView::SaveProjectAs()
{
    QString dir = QFileInfo(m_path).absolutePath();
    QFileDialog dialog(this, "Save As", dir, "chronx (*.chronx)");
    dialog.setDefaultSuffix("chronx");
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);

    if(dialog.exec())
    {
        m_path = dialog.selectedFiles().first();
        shared().dock->setWindowTitle(m_path);
        SaveProject();
    }
}

void CProjectView::Autosave()
{
    if(shared().settingsView->maxAutosaves() > 0 && m_sceneModel->rowCount() > 0 && m_path.length())
    {
        QString filename = QFileInfo(m_path).absolutePath() + "/backups/" + QFileInfo(m_path).completeBaseName() +
                ".backup" + QString::number((m_autosave_num = (m_autosave_num % shared().settingsView->maxAutosaves()) + 1)) + ".chronx";
        QSaveFile file(filename);
        SaveToFile(file);
        file.commit();
        // show status message for 30 seconds
        shared().statusBar->showMessage("Autosave complete...", 30000);
    }

    QTimer::singleShot(shared().settingsView->autosaveInterval(), this, SLOT(Autosave()));
}

void CProjectView::SaveToFile(QSaveFile &file)
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);

    ds << shared().ProgramVersion << m_title->text() << m_author->text() << *(shared().paletteButton) << m_sceneModel->rowCount();
    for(CGraphicsView *view : m_sceneModel->views())
        ds << *(view->cScene());

    ds << *(shared().variablesView);

    file.open(QIODevice::WriteOnly);
    file.write(ba);
}

void CProjectView::OpenProject(QString filepath)
{
    CloseProject();

    QFile file(filepath);
    while(!file.open(QIODevice::ReadOnly))
    {
        QString dir = QFileInfo(filepath).absolutePath();
        file.setFileName(QFileDialog::getOpenFileName(this, "Open", dir, "chronx (*.chronx)"));
        if(!file.fileName().length())
            return;
    }

    m_path = file.fileName();

    // Load data from the file
    QDataStream ds(file.readAll());

    int num_scenes;
    QString project_title;
    QString project_author;

    ds >> m_version;
    if(m_version == "0.8.1.0")
        ds >> project_title >> num_scenes;
    else if(m_version == "0.8.6.0")
        ds >> project_title >> *(shared().paletteButton) >> num_scenes;
    else
        ds >> project_title >> project_author >> *(shared().paletteButton) >> num_scenes;

    m_title->setText(project_title);
    m_author->setText(project_author);

    // load other scenes
    for(int i = 0; i < num_scenes; ++i)
    {
        QString name;
        ds >> name;
        CGraphicsView *view = new CGraphicsView(new CGraphicsScene(false, name), this);
        view->hide();
        m_sceneModel->AddItem(view);
        ds >> *(m_sceneModel->views().last()->cScene());
    }

    ds >> *(shared().variablesView);

    shared().dock->show();
    shared().pointerToolBar->show();
    shared().dock->setWindowTitle(m_path);//(QFileInfo(m_path).fileName());
    shared().sceneTabs->removeTab(shared().sceneTabs->indexOf(shared().homepage));

    // grab the startup view
    CGraphicsView *view = m_sceneModel->views().first();
    shared().sceneTabs->addTab(view, view->cScene()->name());
    shared().sceneTabs->setCurrentWidget(view);
    view->centerOn(view->cScene()->startBubble()->scenePos());

    // show status message for 30 seconds
    shared().statusBar->showMessage("Successfully opened " + m_path, 30000);
}



void CProjectView::ImportChoiceScript(const QString &filepath)
{
    CloseProject();

    QString dir = QFileInfo(filepath).absolutePath();
    QFile file(filepath);
    while(!file.open(QIODevice::ReadOnly))
    {
        file.setFileName(QFileDialog::getOpenFileName(this, "Open", dir, "startup (startup.txt)"));
        if(!file.fileName().length())
            return; // quit if user hits cancel
    }

    CIndentSelectionDialog dialog(this);
    dialog.exec();

    // TODO: ask for indent type
    CSIndent csindent = dialog.getIndent();
    ChoiceScriptData csdata(file, csindent);
    m_sceneModel->setViews(csdata.getViews());
    shared().variablesView->model()->setVariables(csdata.getVariables());
    m_title->setText(csdata.getTitle());
    m_author->setText(csdata.getAuthor());

    file.close();

    SaveProjectAs();
    OpenProject(m_path);
}

void CProjectView::NewProject()
{
    CloseProject();

    m_title->setText("New Project");

    shared().dock->show();
    shared().pointerToolBar->show();
    shared().dock->setWindowTitle(m_path);
    shared().sceneTabs->removeTab(shared().sceneTabs->indexOf(shared().homepage));

    CGraphicsView *view = new CGraphicsView(new CGraphicsScene(true, "startup"), this);
    m_sceneModel->AddItem(view);

    shared().sceneTabs->addTab(view, view->cScene()->name());
    shared().sceneTabs->setCurrentWidget(view);
    view->centerOn(view->cScene()->startBubble()->scenePos());
}

void CProjectView::CloseProject()
{
    m_title->setText("");
    m_path = "";

    for(CGraphicsView *view : m_sceneModel->views())
    {
        QList<CBubble *> bubbles = view->cScene()->bubbles();

        for(CBubble *bbl : bubbles)
            delete bbl;
    }

    m_sceneModel->Reset();
    shared().variablesView->Reset();

    shared().dock->hide();
    shared().pointerToolBar->hide();
    shared().showHomepageAction->trigger();
}


bool SortByOrderAscending(CBubble *first, CBubble *second)
{
    return first->getOrder() < second->getOrder();
}

void CProjectView::ExportChoiceScript()
{
    QString project_folder = QFileInfo(m_path).absolutePath();
    for(CGraphicsView *view : m_sceneModel->views())
    {
        QFile file(project_folder + "/scenes/" + view->cScene()->name() + ".txt");

        QString cs;
        if(view->cScene()->name() == "startup")
        {
            cs += "*title " + m_title->text() + "\n";
            cs += "*author " + m_author->text() + "\n\n";

            cs += "*scene_list\n";
            for(CGraphicsView *v : m_sceneModel->views())
                cs += "    " + v->cScene()->name() + "\n";

            cs += "\n";

            for(const CVariable &v : shared().variablesView->model()->variables())
            {
                if(v.scene() == Q_NULLPTR)
                    cs += "*create " + v.name() + " " + v.data();
            }
        }

        for(const CVariable &v : shared().variablesView->model()->variables())
        {
            if(v.scene() == view->cScene())
                cs += "*temp " + v.name() + " " + v.data();
        }

        QList<CConnection *> processed_links;
        CalculateOrder(view->cScene()->startBubble()->link(), processed_links, 0);

        QList<CBubble *> bubbles = view->cScene()->bubbles();
        qSort(bubbles.begin(), bubbles.end(), SortByOrderAscending);

        QList<CBubble *> processed;


        for(CBubble *bbl : bubbles)
            cs += BubbleToChoiceScript(bubbles, processed, 0, bbl);

        file.open(QIODevice::WriteOnly);
        file.write(cs.toStdString().c_str());
        file.close();
    }
}

QList<CGraphicsView *> CProjectView::getViews()
{
    return m_sceneModel->views();
}

CBubble *CProjectView::BubbleWithUID(uint uid)
{
    CBubble *bubble = 0;

    for(CGraphicsView *view : m_sceneModel->views())
    {
        for(QGraphicsItem *item : view->items())
        {
            if((bubble = dynamic_cast<CBubble *>(item)) && (bubble->getUID() == uid))
                return bubble;
        }
    }

    return 0;
}

const QString CProjectView::getVersion() const
{
    return m_version;
}

CSceneModel *CProjectView::model()
{
    return m_sceneModel;
}

QString CProjectView::BubbleToChoiceScript(const QList<CBubble *> &bubbles, QList<CBubble *> &processed, int indent_level, CBubble *bubble)
{
    QString cs;

    if(!processed.contains(bubble))
    {
        processed.prepend(bubble);

        QString indent_str = "    ";
        QString indent;

        // generate label...
        if(LabelNeeded(bubble, bubbles))
            cs +=  "\n\n*label " + MakeLabel(bubble, bubbles) + "\n";
        else
            indent = QString(indent_str).repeated(indent_level);

        // ------------ Start bubble ------------
        if(bubble->getType() == Chronicler::Start)
        {
            CStartBubble *start = dynamic_cast<CStartBubble *>(bubble);
            if(start->link())
                cs += BubbleToChoiceScript(bubbles, processed, indent_level, start->link()->to());
            else
                cs += indent + "*finish\n";
        }

        // ------------ Story bubble ------------
        if(bubble->getType() == Chronicler::Story)
        {
            CStoryBubble *story = dynamic_cast<CStoryBubble *>(bubble);

            cs += indent + story->getStory().replace("\n", "\n" + indent) + "\n";

            if(story->link())
            {
                if(LabelNeeded(story->link()->to(), bubbles))
                    cs += indent + "*goto " + MakeLabel(story->link()->to(), bubbles);
                //                if(story->link()->to()->getType() == Chronicler::Story)
                //                    cs += indent + "*page_break";
                else
                    cs += "\n" + BubbleToChoiceScript(bubbles, processed, indent_level, story->link()->to());
            }
            else
                cs += indent + "*finish";
        }

        // ------------ Choice bubble ------------
        else if(bubble->getType() == Chronicler::Choice)
        {
            CChoiceBubble *choice_bubble = dynamic_cast<CChoiceBubble *>(bubble);

            cs += indent + "*choice";

            // increase indent level
            indent = QString(indent_str).repeated(++indent_level);

            for(CChoice *choice : choice_bubble->choiceBubbles())
            {
                // TODO add section for conditional choices
                cs += "\n" + indent + "#" + choice->choice() + "\n";

                if(choice->link() && LabelNeeded(choice->link()->to(), bubbles))
                    cs += indent + indent_str + "*goto " + MakeLabel(choice->link()->to(), bubbles);
                else if(choice->link())
                    cs += "\n" + BubbleToChoiceScript(bubbles, processed, indent_level + 1, choice->link()->to());
                else
                    cs += indent + indent_str + "*finish";

                cs += "\n";
            }
        }

        // ------------ Action bubble ------------
        else if(bubble->getType() == Chronicler::Action)
        {
            CActionBubble *action = dynamic_cast<CActionBubble *>(bubble);

            cs += indent + action->actionString().replace("\n", "\n" + indent) + "\n";

            if(action->link() && LabelNeeded(action->link()->to(), bubbles))
                cs += indent + "*goto " + MakeLabel(action->link()->to(), bubbles);
            else if(action->link())
                cs += "\n" + BubbleToChoiceScript(bubbles, processed, indent_level, action->link()->to());
            else
                cs += indent + "*finish";
        }

        // ------------ Condition bubble ------------
        else if(bubble->getType() == Chronicler::Condition)
        {
            CConditionBubble *cb = dynamic_cast<CConditionBubble *>(bubble);

            cs += indent + "*if(" + cb->getCondition() + ")\n";

            // true
            if(cb->trueLink())
            {
                if(LabelNeeded(cb->trueLink()->to(), bubbles))
                    cs += indent + indent_str + "*goto " + MakeLabel(cb->trueLink()->to(), bubbles);
                else
                    cs += BubbleToChoiceScript(bubbles, processed, indent_level + 1, cb->trueLink()->to());
            }
            else
                cs += indent + indent_str + "*finish";

            cs += "\n" + indent + "*else\n";

            // false
            if(cb->falseLink())
            {
                if(LabelNeeded(cb->falseLink()->to(), bubbles))
                    cs += indent + indent_str + "*goto " + MakeLabel(cb->falseLink()->to(), bubbles);
                else
                    cs += BubbleToChoiceScript(bubbles, processed, indent_level + 1, cb->falseLink()->to());
            }
            else
                cs += indent + indent_str + "*finish";
        }
    }

    return cs;
}

bool CProjectView::LabelNeeded(CBubble *bubble, const QList<CBubble *> &bubbles)
{
    QList<CConnection *> connections = bubble->connections();

    // figure out if we need a label or not
    bool make_label = connections.length() > 1 || bubble->getLocked();
    if(!make_label && connections.length() > 0)
    {
        // if the only connecting bubble hasn't been processed yet...
        CBubble *from = connections.first()->from()->container();
        int diff = (bubbles.indexOf(from) - bubbles.indexOf(bubble));
        if((diff > 1) && !dynamic_cast<CChoiceBubble *>(from))
            make_label = true;
    }

    return make_label;
}

QString CProjectView::MakeLabel(CBubble *bubble, const QList<CBubble *> &bubbles)
{
    QString label = bubble->getLabel().replace(" ", "_");
    if(!label.length())
        label = "bubble_" + QString::number(bubble->GenerateUID());
    else
    {
        for(CBubble *b : bubbles)
        {
            if(b != bubble && LabelNeeded(b, bubbles) && b->getLabel() == bubble->getLabel())
            {
                label += "_" + QString::number(bubble->GenerateUID());
                break;
            }
        }
    }

    return label;
}

void CProjectView::CalculateOrder(CConnection *connection, QList<CConnection *> &processed, qint64 order)
{
    if(connection && !processed.contains(connection))
    {
        // update processed
        processed.prepend(connection);

        // calculate new order for current bubble
        qint64 new_order = order;
        if(connection->to()->getLocked())
            new_order = connection->to()->getOrder();
        else
            connection->to()->setOrder(new_order);

        // recurse
        for(CConnection *c : connection->to()->links())
            CalculateOrder(c, processed, new_order + 1);
    }
}




void CProjectView::SelectedChanged(QModelIndex current)
{
    if(current.row() >= 0 && current.row() < m_modelView->model()->rowCount())
    {
        m_upButton->setEnabled(true);
        m_downButton->setEnabled(true);
        m_removeButton->setEnabled(true);

        CGraphicsView *view = m_sceneModel->views()[current.row()];

        if(shared().sceneTabs->indexOf(view) == -1)
            shared().sceneTabs->addTab(view, view->cScene()->name());
        shared().sceneTabs->setCurrentWidget(view);
    }
    else
    {
        m_upButton->setEnabled(false);
        m_downButton->setEnabled(false);
        m_removeButton->setEnabled(false);
    }
}

void CProjectView::DataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // TODO ensure that each scene has a unique name
    for(int i = topLeft.row(); i <= bottomRight.row(); ++i)
    {
        CGraphicsView *view = m_sceneModel->views()[i];
        int tab = shared().sceneTabs->indexOf(view);
        if(tab != -1)
            shared().sceneTabs->setTabText(tab, view->cScene()->name());
    }
}

void CProjectView::ProjectNameChanged()
{
    shared().mainWindow->setWindowTitle("Chronicler " + shared().ProgramVersion + " - " + m_title->text());
}

void CProjectView::MoveUp()
{
    m_sceneModel->MoveUp(m_modelView->currentIndex().row());
}

void CProjectView::MoveDown()
{
    m_sceneModel->MoveDown(m_modelView->currentIndex().row());
}

void CProjectView::AddItem()
{
    m_sceneModel->AddItem(new CGraphicsView(new CGraphicsScene(true, "scene " + QString().setNum(m_sceneModel->rowCount())), this));
    m_modelView->edit(QModelIndex(m_sceneModel->index(m_sceneModel->rowCount() - 1, 0)));
}

void CProjectView::RemoveItem()
{
    if(m_modelView->currentIndex().row() > 0)
        m_sceneModel->RemoveItem(m_modelView->currentIndex().row());
}
