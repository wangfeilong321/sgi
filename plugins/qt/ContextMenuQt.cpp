#include "stdafx.h"
#include "ContextMenuQt.h"
#include <sgi/plugins/SGIHostItemQt.h>
#include "SGIItemQt"
#include "MenuActionQt.h"
#include <QMetaClassInfo>
#include <QWidget>
#include <sgi/helpers/string>
#include <sgi/helpers/qt>

namespace sgi {

namespace qt_plugin {

CONTEXT_MENU_POPULATE_IMPL_REGISTER(QObject)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(QWidget)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(QMetaObject)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(QPaintDevice)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(QImage)

using namespace sgi::qt_helpers;

bool contextMenuPopulateImpl<QObject>::populate(IContextMenuItem * menuItem)
{
    QObject * object = getObject<QObject,SGIItemQt>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        {
            menuItem->addSimpleAction(MenuActionObjectInfo, "Object info...", _item);
            menuItem->addSimpleAction(MenuActionDumpObjectInfo, "Dump object info", _item);
            menuItem->addSimpleAction(MenuActionDumpObjectTree, "Dump object tree", _item);

            SGIHostItemQtMeta metaObject(object->metaObject());
            if(metaObject.hasObject())
                menuItem->addMenu("Meta object", &metaObject); 

            SGIHostItemQt parent(object->parent());
            if(parent.hasObject())
                menuItem->addMenu("Parent", &parent);

            const QObjectList & children = object->children();
            if(!children.empty())
                menuItem->addMenu(helpers::str_plus_count("Childs", children.size()), cloneItem<SGIItemQt>(SGIItemTypeChilds));
            //menuItem->addMenu("Methods", cloneItem<SGIItemQt>(SGIItemTypeMethods));

            ret = true;
        }
        break;
    case SGIItemTypeChilds:
        {
            const QObjectList & children = object->children();
            for(QObjectList::const_iterator it = children.begin(); it != children.end(); it++)
            {
                SGIHostItemQt childItem(*it);
                menuItem->addMenu(std::string(), &childItem);
            }
            ret = true;
        }
        break;
    case SGIItemTypeMethods:
        {
            /*
            const QMetaObject * metaObject = object->metaObject();
            while(metaObject)
            {
                int methodOffset = metaObject->methodOffset();
                int methodCount = metaObject->methodCount();
                for (int i=methodOffset; i<methodCount; ++i)
                {
                    QMetaMethod method = metaObject->method(i);
                    menuItem->addSimpleAction(MenuActionObjectMethodInvoke, method.signature(), _item, new ReferencedDataMetaMethod(method));
                }
                metaObject = metaObject->superClass();
            }
            */
            ret = true;
        }
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<QWidget>::populate(IContextMenuItem * menuItem)
{
    QWidget * object = getObject<QWidget,SGIItemQtPaintDevice>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
/*
            if(object->parentWidget() != object->parent())
            {
                SGIHostItemQt parentWidget(object->parentWidget());
                if(parentWidget.hasObject())
                    menuItem->addMenu("ParentWidget", &parentWidget);
            }*/
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<QMetaObject>::populate(IContextMenuItem * menuItem)
{
    QMetaObject * object = getObject<QMetaObject,SGIItemQtMeta>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        {
            menuItem->addSimpleAction(MenuActionObjectInfo, "Object info...", _item);

            SGIHostItemQtMeta superClass(object->superClass());
            if(superClass.hasObject())
                menuItem->addMenu(helpers::str_plus_info("Super class", object->superClass()->className()), &superClass);
            ret = true;
        }
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<QPaintDevice>::populate(IContextMenuItem * menuItem)
{
    QPaintDevice * object = getObject<QPaintDevice,SGIItemQtPaintDevice>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        {
            menuItem->addSimpleAction(MenuActionObjectInfo, "Object info...", _item);
            ret = true;
        }
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<QImage>::populate(IContextMenuItem * menuItem)
{
    QImage * object = getObject<QImage,SGIItemQtPaintDevice>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionImagePreview, "Preview...", _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

} // namespace qt_plugin
} // namespace sgi
