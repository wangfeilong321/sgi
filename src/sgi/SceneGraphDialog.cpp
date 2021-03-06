#include "stdafx.h"

#include "SceneGraphDialog.h"
#include <ui_SceneGraphDialog.h>

#include <iostream>
#include <QToolBar>
#include <QToolButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDesktopWidget>
#include <QTimer>

#include "sgi/plugins/SGIPluginInterface.h"
#include "sgi/plugins/SGIProxyItem.h"

#include "SGIPlugin.h"
#include "sgi/SGIItemInternal"

#include <sgi/plugins/ObjectTreeImpl>
#include <sgi/helpers/qt>
#include "ContextMenu.h"

#include <sgi/plugins/SGIHostItemInternal.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace sgi {

using namespace qt_helpers;

class SceneGraphDialog::ContextMenuCallback : public IContextMenuInfo
{
public:
    ContextMenuCallback(SceneGraphDialog * dialog)
        : _dialog(dialog) {}
public:
    virtual SGIItemBase * getView()
    {
        return _dialog->getView();
    }
    virtual void            triggerRepaint()
    {
        _dialog->triggerRepaint();
    }
    virtual bool            showSceneGraphDialog(SGIItemBase * item)
    {
        return _dialog->newInstance(item);
    }
    virtual bool            showObjectLoggerDialog(SGIItemBase * item)
    {
        return _dialog->showObjectLoggerDialog(item);
    }
private:
    SceneGraphDialog * _dialog;
};

class SceneGraphDialog::SceneGraphDialogImpl : public ISceneGraphDialog
{
public:
    SceneGraphDialogImpl(SceneGraphDialog * dialog)
        : _dialog(dialog) {}
    virtual                 ~SceneGraphDialogImpl() {}
    virtual QDialog *       getDialog() { return _dialog; }
    virtual IContextMenu *  toolsMenu() { return _dialog->toolsMenu(); }
    virtual void            setObject(SGIItemBase * item, ISceneGraphDialogInfo * info=NULL) { _dialog->setObject(item, info); }
    virtual void            setObject(const SGIHostItemBase * item, ISceneGraphDialogInfo * info=NULL) { _dialog->setObject(item, info); }
    virtual void            show() { emit _dialog->triggerShow(); }
    virtual void            hide() { emit _dialog->triggerHide(); }
    virtual bool            isVisible() { return _dialog->isVisible(); }
    virtual int             showModal() { return _dialog->exec(); }
    virtual IObjectTreeItem * selectedItem() { return _dialog->selectedItem(); }
    virtual IObjectTreeItem * rootItem() { return _dialog->rootItem(); }
    virtual void            setInfoText(const std::string & text) { return _dialog->setInfoText(text); }
    virtual ISceneGraphDialogInfo * getInfo() { return _dialog->_info; }

private:
    SceneGraphDialog * _dialog;
};

class SceneGraphDialog::ToolsMenuImpl : public ISceneGraphDialogToolsMenu
{
public:
    ToolsMenuImpl(SceneGraphDialog * dialog)
        : _dialog(dialog) {}
    virtual                     ~ToolsMenuImpl() {}
    virtual ISceneGraphDialog * getDialog() { return _dialog->dialogInterface(); }
    virtual IContextMenu *      toolsMenu() { return _dialog->toolsMenu(); }

private:
    SceneGraphDialog * _dialog;
};

SceneGraphDialog::SceneGraphDialog(SGIItemBase * item, ISceneGraphDialogInfo * info, QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
	, ui(NULL)
    , _interface(new SceneGraphDialogImpl(this))
    , _item(item)
	, _info(info)
    , _toolBar(NULL)
    , _actionReload(NULL)
    , _actionReloadSelected(NULL)
    , _actionItemPrevious(NULL)
    , _actionItemNext(NULL)
    , _contextMenu()
    , _contextMenuCallback()
    , _spinBoxRefreshTime(NULL)
    , _refreshTimer(NULL)
    , _toolsMenu()
    , _toolsMenuInterface(new ToolsMenuImpl(this))
    , _itemToolsMenu()
    , _firstShow(true)
{
    init();
}

SceneGraphDialog::~SceneGraphDialog()
{
	if (ui)
	{
		delete ui;
		ui = NULL;
	}
	_interface = NULL;
}

void SceneGraphDialog::init()
{
    ui = new Ui_SceneGraphDialog;
    ui->setupUi( this );

    Qt::WindowFlags flags =this->windowFlags()
        | Qt::WindowMinimizeButtonHint
        | Qt::WindowMaximizeButtonHint
        | Qt::WindowCloseButtonHint;
    this->setWindowFlags(flags);

    ObjectTreeItem::s_hostInterface = SGIPlugins::instance()->hostInterface();
    
    _toolBar = new QToolBar;
    QVBoxLayout * mainLayout = (QVBoxLayout *)this->layout();
    mainLayout->insertWidget(0, _toolBar);

    _comboBoxPath = new QComboBox(_toolBar);
    connect(_comboBoxPath, SIGNAL(activated(int)), this, SLOT(selectItemFromPath(int)));

    _actionReloadSelected = new QAction(tr("Reload"), this);
    connect(_actionReloadSelected, SIGNAL(triggered()), this, SLOT(reloadSelectedItem()));

    _actionReload = new QAction(tr("Reload All"), this);
    connect(_actionReload, SIGNAL(triggered()), this, SLOT(reload()));

    _actionItemPrevious = new QAction(tr("Previous"), this);
    connect(_actionItemPrevious, SIGNAL(triggered()), this, SLOT(itemPrevious()));
    _actionItemNext = new QAction(tr("Next"), this);
    connect(_actionItemNext, SIGNAL(triggered()), this, SLOT(itemNext()));

    _spinBoxRefreshTime = new QSpinBox(_toolBar);
    _spinBoxRefreshTime->setMinimum(0);
    _spinBoxRefreshTime->setMaximum(600);
    _spinBoxRefreshTime->setPrefix("Refresh ");
    _spinBoxRefreshTime->setSuffix("s");
    connect(_spinBoxRefreshTime, SIGNAL(valueChanged(int)), this, SLOT(refreshTimeChanged(int)));

    _toolBar->addAction(_actionReloadSelected);
    _toolBar->addAction(_actionReload);
    _toolBar->addAction(_actionItemPrevious);
    _toolBar->addWidget(_comboBoxPath);
    _toolBar->addAction(_actionItemNext);
    _toolBar->addWidget(_spinBoxRefreshTime);

    SGIHostItemInternal hostItemSelf(_interface);
    SGIPlugins::instance()->generateItem(_itemSelf, &hostItemSelf);

    SGIHostItemInternal hostItemToolsMenu(_toolsMenuInterface);
    SGIPlugins::instance()->generateItem(_itemToolsMenu, &hostItemToolsMenu);

    _toolsMenu = new ContextMenu(false, this);
    _toolsMenu->setObject(_itemToolsMenu);

    QToolButton * toolsMenuButton = new QToolButton(this);
    toolsMenuButton->setMenu(_toolsMenu);
    toolsMenuButton->setText(tr("Tools"));
    toolsMenuButton->setPopupMode(QToolButton::MenuButtonPopup);

    _toolBar->addWidget(toolsMenuButton);

    QTreeWidgetItem * root = ui->treeWidget->invisibleRootItem();
    QtSGIItem nodeDataRoot(SGIItemTypeTreeRoot, NULL, true);
    // set dummy data into the second column (type)
    root->setData(0, Qt::UserRole, QVariant::fromValue(nodeDataRoot));
    _rootTreeItem = new ObjectTreeItem(root);

    QObject::connect(this, SIGNAL(triggerOnObjectChanged()), this, SLOT(onObjectChanged()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(triggerShow()), this, SLOT(showBesideParent()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(triggerHide()), this, SLOT(hide()), Qt::QueuedConnection);

    updatePathComboBox();
    reload();
}

void SceneGraphDialog::showBesideParent()
{
    QWidget::show();
    if(_firstShow)
    {
        _firstShow = false;

        QDesktopWidget * dw = QApplication::desktop();
        QWidget * parent = parentWidget();
        if(parent)
        {
            int numScreens = dw->screenCount();
            int parentScreen = dw->screenNumber(parent);
            int currentScreen = dw->screenNumber(this);

            if(parentScreen == currentScreen)
            {
                int targetScreen = (currentScreen + 1) % numScreens;
                if(targetScreen != currentScreen)
                {
                    QRect geom = frameGeometry();
                    QRect currentScreenRect = dw->screenGeometry(currentScreen);
                    QRect targetScreenRect = dw->screenGeometry(targetScreen);
                    QPoint currentTopLeft = parent->mapToGlobal(geom.topLeft());
                    QPoint currentBottomRight = parent->mapToGlobal(geom.bottomRight());
                    QPoint screenOffset = currentTopLeft - currentScreenRect.topLeft();
                    QPoint targetTopLeft = targetScreenRect.topLeft() + screenOffset;
                    QPoint targetBottomRight(targetTopLeft.x() + geom.width(), targetTopLeft.y() + geom.height());
                    if (targetScreenRect.contains(targetTopLeft))
                    {
                        targetTopLeft = parent->mapFromGlobal(targetTopLeft);
                        move(targetTopLeft);
                    }
                }
            }
        }
    }
}

void SceneGraphDialog::onObjectChanged()
{
    updatePathComboBox();
    selectItemInPathBox();
    reload();
}

IContextMenu * SceneGraphDialog::toolsMenu()
{
    return _toolsMenu->menuInterface();
}

void SceneGraphDialog::setObject(const SGIHostItemBase * hostitem, ISceneGraphDialogInfo * info)
{
    SGIItemBasePtr item;
    if(SGIPlugins::instance()->generateItem(item, hostitem))
        setObject(item.get(), info);
}

void SceneGraphDialog::setObject(SGIItemBase * item, ISceneGraphDialogInfo * info)
{
    _item = item;
	if(info)
		_info = info;
    emit triggerOnObjectChanged();
}

void SceneGraphDialog::updatePathComboBox()
{
    _comboBoxPath->blockSignals(true);
    _comboBoxPath->clear();
    // first get the current path
    if(_item.valid())
        SGIPlugins::instance()->getObjectPath(_itemPath, _item);
    else
        _itemPath.clear();

    int currentItemIndex = -1;

    if(_itemPath.empty())
    {
        if(_item.valid())
        {
            std::string objectName;
            SGIPlugins::instance()->getObjectName(objectName, _item, true);
            QtSGIItem data(_item.get());
            _comboBoxPath->addItem(fromLocal8Bit(objectName), QVariant::fromValue(data));
            currentItemIndex = 0;
        }
    }
    else
    {
        for(SGIItemBasePtrPath::const_iterator it = _itemPath.begin(); it != _itemPath.end(); it++)
        {
            const SGIItemBasePtr & item = *it;
            std::string objectName;
            SGIPlugins::instance()->getObjectName(objectName, item, true);
			QtSGIItem data(item.get());
            _comboBoxPath->addItem(fromLocal8Bit(objectName), QVariant::fromValue(data));
            if(*item.get() == *_item.get())
                currentItemIndex = _comboBoxPath->count() - 1;
        }
    }
    int numItems = _comboBoxPath->count();
    _comboBoxPath->setEnabled(_itemPath.empty() == false);
    // trigger the drop-down box to adjust to the new content
    _comboBoxPath->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    _comboBoxPath->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    _comboBoxPath->setCurrentIndex(currentItemIndex);
    _comboBoxPath->blockSignals(false);
    _actionItemPrevious->setEnabled(currentItemIndex > 0);
    _actionItemNext->setEnabled(currentItemIndex + 1 < numItems);
}

void SceneGraphDialog::reload()
{
    setCursor(Qt::WaitCursor);

    if(_item.valid())
    {
        std::string displayName;
        SGIPlugins::instance()->getObjectDisplayName(displayName, _item);
        setWindowTitle(tr("Information about %1").arg(fromLocal8Bit(displayName)));
    }
    else
    {
        setWindowTitle(tr("No Information available"));
    }

    ui->treeWidget->blockSignals(true);
	ui->treeWidget->clear();
	QList<int> panes_sizes;
	int total_width ;
	QLayout * currentLayout = ui->verticalLayout;
	total_width = this->width() - ui->verticalLayout->margin();
	const int tree_width = 3 * total_width / 5;
	const int textbox_width = 2 * total_width / 5;
	panes_sizes.append(tree_width);
	panes_sizes.append(textbox_width);
	ui->splitter->setSizes(panes_sizes);

	total_width = tree_width - 32;
	ui->treeWidget->setColumnWidth(0, 3 * total_width / 4);
	ui->treeWidget->setColumnWidth(1, total_width / 4);

    ObjectTreeItem * viewNodeItem = NULL;

    buildRootTree(static_cast<ObjectTreeItem *>(_rootTreeItem.get()));

	ObjectTreeItem * treeItem = NULL;
    if(_item.valid())
    {
        treeItem = static_cast<ObjectTreeItem *>(_rootTreeItem->addChild(std::string(), _item.get()));
    }

	if(treeItem)
	{
		treeItem->setSelected(true);
		onItemActivated(treeItem->treeItem(), 0);
        _selectedTreeItem = treeItem;
	}
    else if(viewNodeItem)
    {
        viewNodeItem->setSelected(true);
        onItemActivated(viewNodeItem->treeItem(), 0);
        _selectedTreeItem = viewNodeItem;
    }
    else
        _selectedTreeItem = _rootTreeItem;
    ui->treeWidget->blockSignals(false);

    QTreeWidgetItem * currentItem = ui->treeWidget->currentItem();
    if(currentItem)
        ui->treeWidget->expandItem(currentItem);
    setCursor(Qt::ArrowCursor);
}

void SceneGraphDialog::itemNext()
{
    if(!_itemPath.empty())
    {
        for(SGIItemBasePtrPath::const_iterator it = _itemPath.begin(); it != _itemPath.end(); it++)
        {
            if(*_item.get() == *(*it))
            {
                it++;
                if(it != _itemPath.end())
                    _item = *it;
                selectItemInPathBox();
                reload();
                break;
            }
        }
    }
}

void SceneGraphDialog::itemPrevious()
{
    if(!_itemPath.empty())
    {
        SGIItemBasePtr prev;
        for(SGIItemBasePtrPath::const_iterator it = _itemPath.begin(); it != _itemPath.end(); it++)
        {
            if(*_item.get() == *(*it))
            {
                if(prev)
                {
                    _item = prev;
                    selectItemInPathBox();
                    reload();
                }
                break;
            }
            else
                prev =*it;
        }
    }
}

void SceneGraphDialog::reloadSelectedItem()
{
    QTreeWidgetItem * item = _selectedTreeItem.valid()?((ObjectTreeItem*)_selectedTreeItem.get())->treeItem():NULL;
    if(item)
    {
        QtSGIItem itemData = item->data(0, Qt::UserRole).value<QtSGIItem>();
        if(itemData.hasItem())
        {
            // we are going to re-populate the item with new data,
            // so first remove the old dummy child item.
            QList<QTreeWidgetItem *> children = item->takeChildren();
            Q_FOREACH(QTreeWidgetItem * child, children)
            {
                delete child;
            }
        }
        ObjectTreeItem treeItem(item);
        buildTree(&treeItem, itemData.item());
    }
}

void SceneGraphDialog::onItemExpanded(QTreeWidgetItem * item)
{
    QtSGIItem itemData = item->data(0, Qt::UserRole).value<QtSGIItem>();
	if(!itemData.isPopulated() && itemData.hasItem())
    {
        // we are going to re-populate the item with new data,
        // so first remove the old dummy child item.
        QList<QTreeWidgetItem *> children = item->takeChildren();
        Q_FOREACH(QTreeWidgetItem * child, children)
        {
            delete child;
        }
        ObjectTreeItem treeItem(item);
        buildTree(&treeItem, itemData.item());
	}
}

void SceneGraphDialog::onItemCollapsed(QTreeWidgetItem * item)
{
    QtSGIItem itemData = item->data(0, Qt::UserRole).value<QtSGIItem>();
    //setNodeInfo(itemData.item());
}

void SceneGraphDialog::onItemClicked(QTreeWidgetItem * item, int column)
{
    QtSGIItem itemData = item->data(0, Qt::UserRole).value<QtSGIItem>();
    setNodeInfo(itemData.item());
}

void SceneGraphDialog::onItemActivated(QTreeWidgetItem * item, int column)
{
    QtSGIItem itemData = item->data(0, Qt::UserRole).value<QtSGIItem>();
    setNodeInfo(itemData.item());
}

void SceneGraphDialog::onItemSelectionChanged()
{
    QTreeWidgetItem * item = ui->treeWidget->currentItem();
    if(item)
    {
        QtSGIItem itemData = item->data(0, Qt::UserRole).value<QtSGIItem>();
        _selectedTreeItem = new ObjectTreeItem(item);
        setNodeInfo(itemData.item());
    }
    else
    {
        _selectedTreeItem = NULL;
        setNodeInfo(NULL);
    }
}

bool SceneGraphDialog::buildTree(ObjectTreeItem * treeItem, SGIItemBase * item)
{
    bool ret = SGIPlugins::instance()->objectTreeBuildTree(treeItem, item);
    // in any case mark the item as populated and add the interal item as well.
    // We ignore the result from any plugin here, because some plugin be not be
    // functioning as expected or no plugin might be available for this item. The
    // internal item is useful especially in these cases to investigate why the
    // call to build tree failed.
    InternalItemData internalItemData(item);;
    SGIHostItemOsg hostItemInternal(new ReferencedInternalItemData(internalItemData));
    treeItem->addChild("Internal", &hostItemInternal);

    QTreeWidgetItem * treeItemQt = treeItem->treeItem();
    QtSGIItem itemData = treeItemQt->data(0, Qt::UserRole).value<QtSGIItem>();
    itemData.markAsPopulated();
    treeItemQt->setData(0, Qt::UserRole, QVariant::fromValue(itemData));

    // but return the result of the buildTree call to the user
    return ret;
}

bool SceneGraphDialog::buildRootTree(ObjectTreeItem * treeItem)
{
    bool ret = false;
    if(_itemSelf.valid())
        ret = SGIPlugins::instance()->objectTreeBuildRootTree(treeItem, _itemSelf);
    return ret;
}

void SceneGraphDialog::setNodeInfo(const SGIItemBase * item)
{
    std::ostringstream os;
    if(item)
        SGIPlugins::instance()->writePrettyHTML(os, item);
    else
    {
        os << "<b>item is <i>NULL</i></b>";
    }
    ui->textEdit->blockSignals(true);
    ui->textEdit->setHtml(fromLocal8Bit(os.str()));
    ui->textEdit->blockSignals(false);
}

void SceneGraphDialog::onItemContextMenu(QPoint pt)
{
	QTreeWidgetItem * item = ui->treeWidget->itemAt (pt);
	QtSGIItem itemData;
    if (item)
    {
        itemData = item->data(0, Qt::UserRole).value<QtSGIItem>();

        QMenu * contextMenu = NULL;
        if (!_contextMenuCallback)
            _contextMenuCallback = new ContextMenuCallback(this);

        IContextMenu * objectMenu = NULL;
        if (_info)
            objectMenu = _info->contextMenu(this, itemData.item(), _contextMenuCallback);
        if (!objectMenu)
        {
            if (_contextMenu)
            {
                _contextMenu->setObject(itemData.item(), _contextMenuCallback);
                objectMenu = _contextMenu;
            }
            else
            {
                objectMenu = SGIPlugins::instance()->createContextMenu(this, itemData.item(), _contextMenuCallback);
            }
        }

        if (objectMenu)
            contextMenu = objectMenu->getMenu();

        _contextMenu = objectMenu;

        if (contextMenu)
        {
            pt.ry() += ui->treeWidget->header()->height();
            QPoint globalPos = ui->treeWidget->mapToGlobal(pt);
            contextMenu->popup(globalPos);
        }
    }
}

SGIItemBase * SceneGraphDialog::getView()
{
    if(_info)
        return _info->getView();
    else
        return NULL;
}

void SceneGraphDialog::triggerRepaint()
{
    if(_info)
        _info->triggerRepaint();
}

bool SceneGraphDialog::newInstance(SGIItemBase * item)
{
    bool ret;
    // only open a new instance when the object is different
    if(_item != item && *_item.get() != *item)
    {
        ISceneGraphDialog * dlg = SGIPlugins::instance()->showSceneGraphDialog(parentWidget(), item, _info);
        if(dlg)
            dlg->show();
        ret = (dlg != NULL);
    }
    else
    {
        // keep the current dialog open, but return success
        ret = true;
    }
    return ret;
}

bool SceneGraphDialog::newInstance(const SGIHostItemBase * hostitem)
{
    bool ret;
    osg::ref_ptr<SGIItemBase> item;
    if(SGIPlugins::instance()->generateItem(item, hostitem))
        ret = newInstance(item.get());
    else
        ret = false;
    return ret;
}


bool SceneGraphDialog::showObjectLoggerDialog(SGIItemBase * item)
{
    return _info->showObjectLoggerDialog(item);
}

bool SceneGraphDialog::showObjectLoggerDialog(const SGIHostItemBase * hostitem)
{
    bool ret;
    osg::ref_ptr<SGIItemBase> item;
    if(SGIPlugins::instance()->generateItem(item, hostitem))
        ret = showObjectLoggerDialog(item.get());
    else
        ret = false;
    return ret;
}

void SceneGraphDialog::selectItemInPathBox()
{
    int selectIndex = -1;
    _comboBoxPath->blockSignals(true);
    int numItems = _comboBoxPath->count();
    for(int i = 0; selectIndex < 0 && i < numItems; i++)
    {
		QtSGIItem data = _comboBoxPath->itemData(i).value<QtSGIItem>();
        if(*data.item() == *_item.get())
        {
            selectIndex = i;
            break;
        }
    }
    _actionItemPrevious->setEnabled(selectIndex > 0);
    _actionItemNext->setEnabled(selectIndex + 1 < numItems);
    _comboBoxPath->setCurrentIndex(selectIndex);
    _comboBoxPath->blockSignals(false);
}

void SceneGraphDialog::selectItemFromPath(int index)
{
	QtSGIItem data = _comboBoxPath->itemData(index).value<QtSGIItem>();
	_item = data.item();
    _actionItemPrevious->setEnabled(index > 0);
    _actionItemNext->setEnabled(index + 1 < _comboBoxPath->count());
    reload();
}

void SceneGraphDialog::refreshTimeChanged ( int n )
{
    if(!_refreshTimer)
    {
        _refreshTimer = new QTimer(this);
        connect(_refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTimerExpired()));
    }
    if(n > 0)
        _refreshTimer->start(n * 1000);
    else
        _refreshTimer->stop();
}

void SceneGraphDialog::refreshTimerExpired()
{
    QTreeWidgetItem * item = ui->treeWidget->currentItem();
    if(item)
    {
        QtSGIItem itemData = item->data(0, Qt::UserRole).value<QtSGIItem>();
        setNodeInfo(itemData.item());
    }
}

IObjectTreeItem * SceneGraphDialog::selectedItem()
{
    return _selectedTreeItem.get();
}

IObjectTreeItem * SceneGraphDialog::rootItem()
{
    return _rootTreeItem.get();
}

void SceneGraphDialog::setInfoText(const std::string & text)
{
    ui->textEdit->blockSignals(true);
    ui->textEdit->setHtml(fromLocal8Bit(text));
    ui->textEdit->blockSignals(false);
}

} // namespace sgi
