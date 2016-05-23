#ifndef CPROJECTVIEW_H
#define CPROJECTVIEW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QListView;
class QPushButton;
class QMenu;
class QLineEdit;
QT_END_NAMESPACE

#include <QModelIndex>

class CGraphicsView;
class CSceneModel;
class CBubble;

class CProjectView : public QWidget
{
    Q_OBJECT

public:
    static const QString currentVersion;

    CProjectView(QWidget *parent = 0);



    QList<CGraphicsView *> views();

    CBubble *BubbleWithUID(uint uid);

private:
    void CreateBubbles();

    QLineEdit *m_name;

    // view/model
    QListView  *m_modelView;
    CSceneModel *m_sceneModel;

    // list buttons
    QPushButton *m_upButton;
    QPushButton *m_downButton;
    QPushButton *m_addButton;
    QPushButton *m_removeButton;

    // project data
    QString m_version;
//    QString m_name;

    // for quicksave
    QString m_path;

    // TODO
    // member variable for project history

signals:
    sceneSelected(CGraphicsView *);

public slots:
    void SaveProject();
    void SaveProjectAs();
    void OpenProject(const QString &filepath = "");
    void ImportProject();
    void NewProject();
    void CloseProject();

private slots:
    void SelectedChanged(QModelIndex current);
    void DataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void ProjectNameChanged();

    void MoveUp();
    void MoveDown();
    void AddItem();
    void RemoveItem();

};

#endif // CPROJECTVIEW_H
