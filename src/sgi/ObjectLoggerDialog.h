#pragma once

#include <QDialog>
#include "sgi/plugins/ObjectLoggerDialog"
#include <queue>

class QTimer;
class QSpinBox;
QT_BEGIN_NAMESPACE
class Ui_ObjectLoggerDialog;
class QTreeWidgetItem;
class QToolBar;
QT_END_NAMESPACE

namespace sgi {

class SGIPluginHostInterface;
class ObjectTreeItem;
class IObjectLogger;

class ObjectLoggerDialog : public QDialog
{
	Q_OBJECT

public:
                        ObjectLoggerDialog(SGIItemBase * item, IObjectLoggerDialogInfo * info=NULL, QWidget *parent = 0, Qt::WindowFlags f = 0);
                        ObjectLoggerDialog(IObjectLogger * logger, IObjectLoggerDialogInfo * info=NULL, QWidget *parent = 0, Qt::WindowFlags f = 0);
	virtual				~ObjectLoggerDialog();

private:
                            ObjectLoggerDialog(const ObjectLoggerDialog & rhs);
    ObjectLoggerDialog &    operator=(const ObjectLoggerDialog & rhs);

public:
    IObjectLoggerDialog *   dialogInterface() { return _interface; }

    bool                    addItem(SGIItemBase * item, bool alsoChilds=true);
    bool                    addItems(const SGIItemBasePtrPath & path);
    bool                    removeItem(SGIItemBase * item);
    bool                    removeItems(const SGIItemBasePtrPath & path);

    bool                    addLogItem(SGIDataItemBase * item);
    bool                    removeLogItem(SGIDataItemBase * item, bool first);

public slots:
    void                    onItemExpanded(QTreeWidgetItem * item);
    void                    onItemCollapsed(QTreeWidgetItem * item);
    void                    onItemClicked(QTreeWidgetItem * item, int column);
    void                    onItemActivated(QTreeWidgetItem * item, int column);
    void                    onItemContextMenu(QPoint pt);
    void                    reload();
    void                    reloadTree();
    void                    reloadLog();
    void                    onObjectChanged();

protected slots:
    void                    refreshTimeChanged(int n);
    void                    refreshTimerExpired();
    void                    updateLog();

signals:
    void                    triggerOnObjectChanged();
    void                    triggerShow();
    void                    triggerHide();
    void                    triggerUpdateLog();

protected:
    class ContextMenuCallback;
    class SceneGraphDialogInfo;
    class ObjectLoggerDialogImpl;

protected:
    bool                    buildTree(ObjectTreeItem * treeItem, SGIItemBase * item);

    void                    triggerRepaint();
    SGIItemBase *           getView();
    bool                    showSceneGraphDialog(SGIItemBase * item);
    bool                    showSceneGraphDialog(const SGIHostItemBase * item);

    bool                    newInstance(SGIItemBase * item);
    bool                    newInstance(const SGIHostItemBase * item);

private:
    void                    init();

    class QueuedOperation;
    class OperationQueue;

private:
    Ui_ObjectLoggerDialog *             ui;
    SGIPluginHostInterface *            _hostInterface;
    IObjectLoggerDialogPtr              _interface;
    IObjectLoggerPtr                    _logger;
    SGIItemBasePtr                      _item;
    IObjectLoggerDialogInfoPtr          _info;
    QToolBar *                          _toolBar;
    QAction *                           _actionReload;
    QSpinBox *                          _spinBoxRefreshTime;
    QTimer *                            _refreshTimer;
    osg::ref_ptr<IContextMenu>          _contextMenu;
    osg::ref_ptr<ContextMenuCallback>   _contextMenuCallback;
    osg::ref_ptr<SceneGraphDialogInfo>  _sceneGraphDialogInfo;
    OperationQueue *                    _queuedOperations;
};

} // namespace sgi
