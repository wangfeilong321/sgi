#include "stdafx.h"
#include "ContextMenuOSG.h"
#include <sgi/helpers/osg>
#include "SGIItemOsg"

#include <sgi/plugins/ContextMenu>
#include <sgi/plugins/SceneGraphDialog>
#include <sgi/plugins/ObjectLoggerDialog>
#include <sgi/SGIItemInternal>
#include <sgi/plugins/SGIHostItemQt.h>
#include <sgi/helpers/string>

#include <osg/Version>
#include <osg/PagedLOD>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/AutoTransform>
#include <osg/CameraView>
#include <osg/ProxyNode>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/ClipNode>
#include <osg/Depth>
#include <osg/Material>
#include <osg/LineWidth>
#include <osg/LineStipple>
#include <osg/io_utils>

#include <osgDB/Registry>

#include <osgViewer/View>
#include <osgQt/GraphicsWindowQt>
#include <osgText/Text>
#include <osgAnimation/AnimationManagerBase>

#include "MenuActionOSG.h"
#include "stateset_helpers.h"
#include "osg_accessor.h"
#include "DrawableHelper.h"
#include "osganimation_accessor.h"

namespace sgi {

class SGIItemOsg;

namespace osg_plugin {

CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Referenced)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Object)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Node)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Group)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::StateSet)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Geode)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Transform)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::MatrixTransform)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::PositionAttitudeTransform)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::AutoTransform)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Billboard)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::CameraView)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Camera)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::View)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::ProxyNode)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::LOD)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::PagedLOD)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Drawable)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::ShapeDrawable)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Geometry)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Sphere)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Box)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Cone)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Cylinder)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Capsule)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::InfinitePlane)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::TriangleMesh)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::HeightField)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::CompositeShape)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Image)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::StateAttribute)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Texture)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Texture2D)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Texture3D)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::ClipNode)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::GraphicsContext)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Shader)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Program)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Depth)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Uniform)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::BufferData)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Array)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Material)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::LineWidth)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::LineStipple)

CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgDB::Registry)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgDB::ImagePager)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgDB::DatabasePager)

CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgGA::GUIEventHandler)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgGA::GUIEventAdapter)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgGA::CameraManipulator)

CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgViewer::View)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgViewer::Scene)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgViewer::GraphicsWindow)

CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgQt::GraphicsWindowQt)

CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgText::TextBase)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgText::Text)

CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgAnimation::AnimationManagerBase)

CONTEXT_MENU_POPULATE_IMPL_REGISTER(sgi::ISceneGraphDialogToolsMenu)

using namespace sgi::osg_helpers;

namespace {
    static void createStateAttributeMenu(unsigned actionId, osg::StateAttribute * attr, IContextMenuItem * menuItem,
                                             const std::string & name, SGIItemBase * item, const osg::StateAttribute::OverrideValue value)
    {
        StateAttributeModeValue currentMode = getStateAttributeModeFromOverrideValue(value);
        IContextMenuItem * childMenu = menuItem->addModeMenu(actionId, name, item, currentMode, attr);

        childMenu->addModeAction("Unspecified", StateAttributeModeValueUnspecified);
        childMenu->addModeAction("Off", StateAttributeModeValueOff);
        childMenu->addModeAction("Override On", StateAttributeModeValueOverrideOn);
        childMenu->addModeAction("Protected", StateAttributeModeValueProtected);
    }
    static void createUniformMenu(unsigned actionId, osg::Uniform * uniform, IContextMenuItem * menuItem,
                                             const std::string & name, SGIItemBase * item, const osg::StateAttribute::OverrideValue value)
    {
        StateAttributeModeValue currentMode = getStateAttributeModeFromOverrideValue(value);
        IContextMenuItem * childMenu = menuItem->addModeMenu(actionId, name, item, currentMode, uniform);

        childMenu->addModeAction("Unspecified", StateAttributeModeValueUnspecified);
        childMenu->addModeAction("Off", StateAttributeModeValueOff);
        childMenu->addModeAction("Override On", StateAttributeModeValueOverrideOn);
        childMenu->addModeAction("Protected", StateAttributeModeValueProtected);
    }

    static void createStateAttributeModeMenu(unsigned actionId, osg::StateAttribute::GLMode mode, IContextMenuItem * menuItem,
                                             const std::string & name, SGIItemBase * item, const StateAttributeModeValue value)
    {
        IContextMenuItem * childMenu = menuItem->addModeMenu(actionId, name, item, value, new ReferencedDataInt(mode));

        childMenu->addModeAction("Unspecified", StateAttributeModeValueUnspecified);
        childMenu->addModeAction("Off", StateAttributeModeValueOff);
        childMenu->addModeAction("On", StateAttributeModeValueOn);
        childMenu->addModeAction("Off (override)", StateAttributeModeValueOverrideOff);
        childMenu->addModeAction("On (override)", StateAttributeModeValueOverrideOn);
    }
    static void addStateSetModeMenu(IContextMenuItem * menuItem, osg::StateAttribute::GLMode mode, SGIItemBase * item)
    {
        std::string name = sgi::castToEnumValueString<sgi::osg_helpers::GLConstant>(mode);
        StateAttributeModeValue currentMode = getStateAttributeModeFromObject(static_cast<osg::Object*>(static_cast<SGIItemOsg*>(item)->object()), mode);
        createStateAttributeModeMenu(MenuActionStateSetMode, mode, menuItem, name, item, currentMode);
    }
}

extern std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const osg::StateAttribute::Type & t);

bool contextMenuPopulateImpl<osg::Referenced>::populate(IContextMenuItem * menuItem)
{
    osg::Referenced * object = static_cast<osg::Referenced*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        menuItem->addSimpleAction(MenuActionObjectInfo, "Object info...", _item);
        ret = true;
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Object>::populate(IContextMenuItem * menuItem)
{
    osg::Object * object = static_cast<osg::Object*>(item<SGIItemOsg>()->object());

    bool ret;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionObjectSetName, "Set name...", _item);
            menuItem->addSimpleAction(MenuActionObjectSave, "Save as...", _item);
            IContextMenuItem * manipulateMenu = menuItem->getOrCreateMenu("Manipulate");
            if(manipulateMenu)
            {
                IContextMenuItem * dataVarianceMenu = manipulateMenu->addModeMenu(MenuActionObjectDataVariance, "Data variance", _item, (int)object->getDataVariance());
                if(dataVarianceMenu)
                {
                    dataVarianceMenu->addModeAction("Unspecified", osg::Object::UNSPECIFIED);
                    dataVarianceMenu->addModeAction("Static", osg::Object::STATIC);
                    dataVarianceMenu->addModeAction("Dynamic", osg::Object::DYNAMIC);
                }
            }
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Node>::populate(IContextMenuItem * menuItem)
{
    osg::Node * object = static_cast<osg::Node*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            IContextMenuItem * manipulateMenu = menuItem->getOrCreateMenu("Manipulate");
            if(manipulateMenu)
            {
                unsigned numUpdateTraversal = object->getNumChildrenRequiringUpdateTraversal();
                unsigned numEventTraversal = object->getNumChildrenRequiringEventTraversal();

                manipulateMenu->addSimpleAction(MenuActionNodeMask, helpers::str_plus_hex("Node mask", object->getNodeMask()), _item);

                manipulateMenu->addSimpleAction(MenuActionNodeNumUpdateTraversal, helpers::str_plus_count("Num update traversal", numUpdateTraversal), _item);
                manipulateMenu->addSimpleAction(MenuActionNodeNumEventTraversal, helpers::str_plus_count("Num event traversal", numEventTraversal), _item);

                manipulateMenu->addSimpleAction(MenuActionNodeRecomputeBound, "Recompute bound", _item);
                manipulateMenu->addBoolAction(MenuActionNodeCullingActive, "Culling Active", _item, object->getCullingActive());
                manipulateMenu->addSimpleAction(MenuActionNodeStripTextures, "Strip textures", _item);
            }

            menuItem->addMenu("Node visitor...", cloneItem<SGIItemOsg>(SGIItemTypeObjectLogger));

            IContextMenuItem * lookAtMenu = menuItem->getOrCreateMenu("Look at");
            if(lookAtMenu)
            {
                lookAtMenu->addSimpleAction(MenuActionNodeLookAt, "Direct", _item, new ReferencedDataInt(NodeLookAtDirect));
                lookAtMenu->addSimpleAction(MenuActionNodeLookAt, "From behind", _item, new ReferencedDataInt(NodeLookAtFromBehind));
                lookAtMenu->addSimpleAction(MenuActionNodeLookAt, "Top view", _item, new ReferencedDataInt(NodeLookAtTopView));
            }

            SGIHostItemOsg eventCallback(object->getEventCallback());
            if(eventCallback.hasObject())
                menuItem->addMenu("EventCallback", &eventCallback);

            SGIHostItemOsg udpateCallback(object->getUpdateCallback());
            if(udpateCallback.hasObject())
                menuItem->addMenu("UpdateCallback", &udpateCallback);

            SGIHostItemOsg cullCallback(object->getCullCallback());
            if(cullCallback.hasObject())
                menuItem->addMenu("CullCallback", &cullCallback);

            SGIHostItemOsg stateSet(object->getStateSet());
            if(stateSet.hasObject())
                menuItem->addMenu("StateSet", &stateSet);
            else
                manipulateMenu->addSimpleAction(MenuActionNodeCreateStateSet, "Create StateSet", _item);
        }
        break;
    case SGIItemTypeObjectLogger:
        {
            IObjectLoggerPtr logger;
            SGIItemBasePtr root = _item->rootBase();
            _hostInterface->getObjectLogger(logger, root);
            menuItem->addBoolAction(MenuActionObjectLoggerActive, "Active", _item, (logger.valid())?logger->isItemPresent(root):false);
            if(logger.valid())
            {
                IObjectLoggerDialogPtr dialog = logger->getDialog();
                menuItem->addBoolAction(MenuActionObjectLoggerVisible, "Visible", _item, (dialog.valid())?dialog->isVisible():false);
            }
            ret = true;
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Group>::populate(IContextMenuItem * menuItem)
{
    osg::Group * object = static_cast<osg::Group*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            unsigned numChildren = object->getNumChildren();
            if(numChildren)
                menuItem->addMenu(helpers::str_plus_count("Childs", numChildren), cloneItem<SGIItemOsg>(SGIItemTypeChilds));

            IContextMenuItem * manipulateMenu = menuItem->getOrCreateMenu("Manipulate");
            if(manipulateMenu)
            {
                IContextMenuItem * addChildMenu = manipulateMenu->addModeMenu(MenuActionGroupAddChild, "Add child", _item, MenuActionAddChildInvalid);
                if(addChildMenu)
                {
                    addChildMenu->addModeAction("Axis", MenuActionAddChildAxis);
                    addChildMenu->addModeAction("Billboard axis", MenuActionAddChildBillboardAxis);
                    addChildMenu->addModeAction("Bounding box", MenuActionAddChildBoundingBox);
                }
            }
        }
        break;
    case SGIItemTypeChilds:
        {
            unsigned numChildren = object->getNumChildren();
            for(unsigned i = 0; i < numChildren; i++)
            {
                osg::Node * child = object->getChild(i);
                SGIHostItemOsg childItem(child);
                menuItem->addMenu(std::string(), &childItem);
            }
            ret = true;
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::StateSet>::populate(IContextMenuItem * menuItem)
{
    osg::StateSet * object = static_cast<osg::StateSet*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            IContextMenuItem * manipulateMenu = menuItem->getOrCreateMenu("Manipulate");
            if(manipulateMenu)
            {
                osg::StateSet::RenderingHint renderingHint = (osg::StateSet::RenderingHint)object->getRenderingHint();
                IContextMenuItem * renderingHintMenu = manipulateMenu->addModeMenu(MenuActionStateSetRenderHint, "RenderingHint", _item, renderingHint);
                renderingHintMenu->addModeAction("DEFAULT_BIN", osg::StateSet::DEFAULT_BIN);
                renderingHintMenu->addModeAction("OPAQUE_BIN", osg::StateSet::OPAQUE_BIN);
                renderingHintMenu->addModeAction("TRANSPARENT_BIN", osg::StateSet::TRANSPARENT_BIN);
            }

            manipulateMenu->addSimpleAction(MenuActionStateSetRenderBinNumber, helpers::str_plus_info("RenderBinNum", object->getBinNumber()), _item);
            manipulateMenu->addSimpleAction(MenuActionStateSetRenderBinName, helpers::str_plus_info("RenderBinName", object->getBinName()), _item);

            osg::StateSet::RenderBinMode renderBinMode = (osg::StateSet::RenderBinMode)object->getRenderBinMode();
            IContextMenuItem * renderBinModeMenu = menuItem->addModeMenu(MenuActionStateSetRenderBinMode, "RenderBinMode", _item, renderBinMode);
            if(renderBinModeMenu)
            {
                renderBinModeMenu->addModeAction("Inherit", osg::StateSet::INHERIT_RENDERBIN_DETAILS);
                renderBinModeMenu->addModeAction("Use", osg::StateSet::USE_RENDERBIN_DETAILS);
                renderBinModeMenu->addModeAction("Override", osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);
            }

            const osg::StateSet::ModeList & modelist = object->getModeList();
            menuItem->addMenu(helpers::str_plus_count("Mode list", modelist.size()), cloneItem<SGIItemOsg>(SGIItemTypeStateSetModeList, ~0u));

            const osg::StateSet::AttributeList & attributelist = object->getAttributeList();
            menuItem->addMenu(helpers::str_plus_count("Attribute list", attributelist.size()), cloneItem<SGIItemOsg>(SGIItemTypeStateSetAttributeList, ~0u));

            menuItem->addMenu(helpers::str_plus_count("Texture mode lists", object->getNumTextureModeLists()), cloneItem<SGIItemOsg>(SGIItemTypeStateSetTextureModeList, ~0u));

            menuItem->addMenu(helpers::str_plus_count("Texture attribute lists", object->getNumTextureAttributeLists()), cloneItem<SGIItemOsg>(SGIItemTypeStateSetTextureAttributeLists, ~0u));

            const osg::StateSet::UniformList & uniformlist = object->getUniformList();
            menuItem->addMenu(helpers::str_plus_count("Uniform list", uniformlist.size()), cloneItem<SGIItemOsg>(SGIItemTypeStateSetUniformList, ~0u));
        }
        break;
    case SGIItemTypeStateSetTextureAttributeLists:
        {
            unsigned texturecount = 0;
            const osg::StateSet::TextureAttributeList & textureAttributes = object->getTextureAttributeList();
            for(osg::StateSet::TextureAttributeList::const_iterator it = textureAttributes.begin(); it != textureAttributes.end(); it++, texturecount++)
            {
                const osg::StateSet::AttributeList & attributes = *it;
                unsigned attrcount = 0;
                for(osg::StateSet::AttributeList::const_iterator it2 = attributes.begin(); it2 != attributes.end(); it2++, attrcount++)
                {
                    const osg::StateAttribute::TypeMemberPair & typememberpair = it2->first;
                    const osg::StateAttribute::Type & type = typememberpair.first;
                    const unsigned int & member = typememberpair.second;
                    const osg::StateSet::RefAttributePair & attrpair = it2->second;
                    const osg::ref_ptr<osg::StateAttribute> & attr = attrpair.first;
                    const osg::StateAttribute::OverrideValue & value = attrpair.second;

                    std::string name = helpers::str_plus_number("Texture", texturecount) + "/" + helpers::str_plus_number("Attribute", attrcount);
                    createStateAttributeMenu(MenuActionStateSetTextureAttributeList, attr.get(), menuItem, name, _item, value);
                }
            }
            ret = true;
        }
        break;
    case SGIItemTypeStateSetTextureModeList:
        {
            ret = true;
        }
        break;
    case SGIItemTypeStateSetModeList:
        {
            addStateSetModeMenu(menuItem, GL_LIGHTING, _item);
            addStateSetModeMenu(menuItem, GL_BLEND, _item);
            addStateSetModeMenu(menuItem, GL_CULL_FACE, _item);
            addStateSetModeMenu(menuItem, GL_DEPTH_TEST, _item);
            addStateSetModeMenu(menuItem, GL_NORMALIZE, _item);
            addStateSetModeMenu(menuItem, GL_RESCALE_NORMAL, _item);
            addStateSetModeMenu(menuItem, GL_POINT_SMOOTH, _item);
            addStateSetModeMenu(menuItem, GL_LINE_SMOOTH, _item);
            addStateSetModeMenu(menuItem, GL_ALPHA_TEST, _item);
            addStateSetModeMenu(menuItem, GL_CLIP_PLANE0, _item);
            addStateSetModeMenu(menuItem, GL_CLIP_PLANE1, _item);
            addStateSetModeMenu(menuItem, GL_CLIP_PLANE2, _item);
            ret = true;
        }
        break;
    case SGIItemTypeStateSetAttributeList:
        {
            IContextMenuItem * addAttrMenu = menuItem->addModeMenu(MenuActionStateSetAddAttribute, "Add", _item, (osg::StateAttribute::Type)-1);
            if(addAttrMenu)
            {
                addAttrMenu->addModeAction("Program", osg::StateAttribute::PROGRAM);
                addAttrMenu->addModeAction("View port", osg::StateAttribute::VIEWPORT);
                addAttrMenu->addModeAction("ClipPlane", osg::StateAttribute::CLIPPLANE);
                addAttrMenu->addModeAction("Depth", osg::StateAttribute::DEPTH);
                addAttrMenu->addModeAction("Material", osg::StateAttribute::MATERIAL);
            }
            unsigned childNo = 0;
            const osg::StateSet::AttributeList & list = object->getAttributeList();
            if(!list.empty())
                menuItem->addSeparator();

            for(osg::StateSet::AttributeList::const_iterator it = list.begin(); it != list.end(); it++, childNo++)
            {
                const osg::StateAttribute::TypeMemberPair & typepair = it->first;
                const osg::StateAttribute::Type & type = typepair.first;
                const unsigned & member = typepair.second;
                const osg::StateSet::RefAttributePair & attrpair = it->second;
                const osg::ref_ptr<osg::StateAttribute> & attr = attrpair.first;
                const osg::StateAttribute::OverrideValue & value = attrpair.second;
                SGIHostItemOsg attrItem(attr.get());
                std::stringstream ss;
                ss << childNo << ':' << type << ' ' << attr->getName();
                createStateAttributeMenu(MenuActionStateSetAttributeValue, attr.get(), menuItem, ss.str(), _item, value);
            }

            ret = true;
        }
        break;
    case SGIItemTypeStateSetUniformList:
        {
            IContextMenuItem * addUniformMenu = menuItem->addModeMenu(MenuActionStateSetAddUniform, "Add", _item, (osg::Uniform::Type)-1);
            if(addUniformMenu)
            {
                addUniformMenu->addModeAction("Bool", osg::Uniform::BOOL);
                addUniformMenu->addModeAction("Integer", osg::Uniform::INT);
                addUniformMenu->addModeAction("Float", osg::Uniform::FLOAT);
                addUniformMenu->addModeAction("Double", osg::Uniform::DOUBLE);
                addUniformMenu->addModeAction("Vec2f", osg::Uniform::FLOAT_VEC2);
                addUniformMenu->addModeAction("Vec3f", osg::Uniform::FLOAT_VEC3);
                addUniformMenu->addModeAction("Vec4f", osg::Uniform::FLOAT_VEC4);
                addUniformMenu->addModeAction("Vec2d", osg::Uniform::DOUBLE_VEC2);
                addUniformMenu->addModeAction("Vec3d", osg::Uniform::DOUBLE_VEC3);
                addUniformMenu->addModeAction("Vec4d", osg::Uniform::DOUBLE_VEC4);
            }

            unsigned childNo = 0;
            const osg::StateSet::UniformList & list = object->getUniformList();
            if(!list.empty())
                menuItem->addSeparator();

            for(osg::StateSet::UniformList::const_iterator it = list.begin(); it != list.end(); it++, childNo++)
            {
                const std::string & name = it->first;
                const osg::StateSet::RefUniformPair & unipair = it->second;
                const osg::ref_ptr<osg::Uniform> & uniform = unipair.first;
                const osg::StateAttribute::OverrideValue & value = unipair.second;
                SGIHostItemOsg uniformItem(uniform.get());
                std::stringstream ss;
                ss << childNo << ':' << osg::Uniform::getTypename(uniform->getType()) << ' ' << uniform->getName();
                createUniformMenu(MenuActionStateSetUniformValue, uniform.get(), menuItem, ss.str(), _item, value);
            }

            ret = true;
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Program>::populate(IContextMenuItem * menuItem)
{
    osg::Program * object = getObject<osg::Program,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            IContextMenuItem * addShaderMenu = menuItem->addModeMenu(MenuActionProgramAddShader, "Add shader", _item, osg::Shader::UNDEFINED);
            if(addShaderMenu)
            {
                addShaderMenu->addModeAction("Vertex", osg::Shader::VERTEX);
                addShaderMenu->addModeAction("Fragment", osg::Shader::FRAGMENT);
                addShaderMenu->addModeAction("Geometry", osg::Shader::GEOMETRY);
                addShaderMenu->addModeAction("Tesselation control", osg::Shader::TESSCONTROL);
                addShaderMenu->addModeAction("Tesselation evaluation", osg::Shader::TESSEVALUATION);
                addShaderMenu->addModeAction("Compute", osg::Shader::COMPUTE);
            }
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Depth>::populate(IContextMenuItem * menuItem)
{
    osg::Depth * object = getObject<osg::Depth,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            IContextMenuItem * functionMenu = menuItem->addModeMenu(MenuActionDepthFunction, "Function", _item, object->getFunction());
            if(functionMenu)
            {
                functionMenu->addModeAction("GL_NEVER", osg::Depth::NEVER);
                functionMenu->addModeAction("GL_LESS", osg::Depth::LESS);
                functionMenu->addModeAction("GL_EQUAL", osg::Depth::EQUAL);
                functionMenu->addModeAction("GL_LEQUAL", osg::Depth::LEQUAL);
                functionMenu->addModeAction("GL_GREATER", osg::Depth::GREATER);
                functionMenu->addModeAction("GL_NOTEQUAL", osg::Depth::NOTEQUAL);
                functionMenu->addModeAction("GL_GEQUAL", osg::Depth::GEQUAL);
                functionMenu->addModeAction("GL_ALWAYS", osg::Depth::ALWAYS);
            }
            menuItem->addBoolAction(MenuActionDepthWriteMask, "Write mask", _item, object->getWriteMask());
            menuItem->addSimpleAction(MenuActionDepthSetZNear, helpers::str_plus_info("ZNear", object->getZNear()), _item);
            menuItem->addSimpleAction(MenuActionDepthSetZFar, helpers::str_plus_info("ZFar", object->getZFar()), _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Uniform>::populate(IContextMenuItem * menuItem)
{
    osg::Uniform * object = getObject<osg::Uniform,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            std::stringstream ss;
            ss << "Edit " << osg::Uniform::getTypename(object->getType());
            if(object->getType() == osg::Uniform::BOOL)
            {
                bool value;
                object->get(value);
                menuItem->addBoolAction(MenuActionUniformEdit, ss.str(), _item, value);
            }
            else
            {
                menuItem->addSimpleAction(MenuActionUniformEdit, ss.str(), _item);
            }
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::BufferData>::populate(IContextMenuItem * menuItem)
{
    osg::BufferData * object = getObject<osg::BufferData,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            IContextMenuItem * manipulateMenu = menuItem->getOrCreateMenu("Manipulate");
            if(manipulateMenu)
            {
                manipulateMenu->addSimpleAction(MenuActionBufferDirty, "Dirty", _item);
            }
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Array>::populate(IContextMenuItem * menuItem)
{
    osg::Array * object = getObject<osg::Array,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            IContextMenuItem * manipulateMenu = menuItem->getOrCreateMenu("Manipulate");
            if(manipulateMenu)
            {
                manipulateMenu->addSimpleAction(MenuActionArrayDataEdit, "Modify data...", _item);
            }
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

#define contextMenuPopulateImpl_material_per_face_property(__id) \
    menuItem->addSimpleAction(MenuActionMaterial##__id, helpers::str_plus_info(#__id " Front", object->get##__id(osg::Material::FRONT)), _item, new ReferencedDataInt(osg::Material::FRONT)); \
    menuItem->addSimpleAction(MenuActionMaterial##__id, helpers::str_plus_info(#__id " Back", object->get##__id(osg::Material::BACK)), _item, new ReferencedDataInt(osg::Material::BACK)); \
    menuItem->addSimpleAction(MenuActionMaterial##__id, helpers::str_plus_info(#__id " Front+Back", object->get##__id(osg::Material::FRONT_AND_BACK)), _item, new ReferencedDataInt(osg::Material::FRONT_AND_BACK)); \
    (void(0))

bool contextMenuPopulateImpl<osg::Material>::populate(IContextMenuItem * menuItem)
{
    osg::Material * object = getObject<osg::Material,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            IContextMenuItem * colorModeMenu = menuItem->addModeMenu(MenuActionMaterialColorMode, "Color mode", _item, object->getColorMode());
            if(colorModeMenu)
            {
                colorModeMenu->addModeAction("AMBIENT", osg::Material::AMBIENT);
                colorModeMenu->addModeAction("DIFFUSE", osg::Material::DIFFUSE);
                colorModeMenu->addModeAction("SPECULAR", osg::Material::SPECULAR);
                colorModeMenu->addModeAction("EMISSION", osg::Material::EMISSION);
                colorModeMenu->addModeAction("AMBIENT_AND_DIFFUSE", osg::Material::AMBIENT_AND_DIFFUSE);
                colorModeMenu->addModeAction("OFF", osg::Material::OFF);
            }
            contextMenuPopulateImpl_material_per_face_property(Ambient);
            contextMenuPopulateImpl_material_per_face_property(Diffuse);
            contextMenuPopulateImpl_material_per_face_property(Specular);
            contextMenuPopulateImpl_material_per_face_property(Emission);
            contextMenuPopulateImpl_material_per_face_property(Shininess);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::LineWidth>::populate(IContextMenuItem * menuItem)
{
    osg::LineWidth * object = getObject<osg::LineWidth,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionLineWidthSet, helpers::str_plus_info("Width", object->getWidth()), _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::LineStipple>::populate(IContextMenuItem * menuItem)
{
    osg::LineStipple * object = getObject<osg::LineStipple,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionLineStipplePattern, helpers::str_plus_info("Pattern", object->getPattern()), _item);
            menuItem->addSimpleAction(MenuActionLineStippleFactor, helpers::str_plus_info("Factor", object->getFactor()), _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Geode>::populate(IContextMenuItem * menuItem)
{
    osg::Geode * object = static_cast<osg::Geode*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            IContextMenuItem * manipulateMenu = menuItem->getOrCreateMenu("Manipulate");
            if(manipulateMenu)
            {
                IContextMenuItem * addShapeMenu = manipulateMenu->addModeMenu(MenuActionGeodeAddShapeDrawable, "Add shape", _item, MenuActionAddShapeInvalid);
                if(addShapeMenu)
                {
                    addShapeMenu->addModeAction("Sphere", MenuActionAddShapeSphere);
                    addShapeMenu->addModeAction("Box", MenuActionAddShapeBox);
                    addShapeMenu->addModeAction("Cone", MenuActionAddShapeCone);
                    addShapeMenu->addModeAction("Cylinder", MenuActionAddShapeCylinder);
                    addShapeMenu->addModeAction("Capsule", MenuActionAddShapeCapsule);
                }
                manipulateMenu->addBoolAction(MenuActionGeodeRenderInfoDrawable, "Render info", _item, RenderInfoDrawable::isPresent(object));
            }
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Transform>::populate(IContextMenuItem * menuItem)
{
    osg::Transform * object = static_cast<osg::Transform*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            IContextMenuItem * refFrameMenu = menuItem->addModeMenu(MenuActionTransformReferenceFrame, "Reference Frame", _item, object->getReferenceFrame());
            if(refFrameMenu)
            {
                refFrameMenu->addModeAction("Relative", osg::Transform::RELATIVE_RF);
                refFrameMenu->addModeAction("Absolute inherit viewpoint", osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT);
                refFrameMenu->addModeAction("Absolute", osg::Transform::ABSOLUTE_RF);
            }
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::MatrixTransform>::populate(IContextMenuItem * menuItem)
{
    osg::MatrixTransform * object = static_cast<osg::MatrixTransform*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionMatrixTransformEdit, "Set matrix...", _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::PositionAttitudeTransform>::populate(IContextMenuItem * menuItem)
{
    osg::PositionAttitudeTransform * object = static_cast<osg::PositionAttitudeTransform*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionPositionAttitudeTransformEdit, "Edit PAT...", _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::AutoTransform>::populate(IContextMenuItem * menuItem)
{
    osg::AutoTransform * object = static_cast<osg::AutoTransform*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionAutoTransformSetAxis, helpers::str_plus_info("Axis", object->getAxis()), _item);
            menuItem->addSimpleAction(MenuActionAutoTransformSetNormal, helpers::str_plus_info("Normal", object->getNormal()), _item);
            menuItem->addSimpleAction(MenuActionAutoTransformSetScale, helpers::str_plus_info("Scale", object->getScale()), _item);
            menuItem->addSimpleAction(MenuActionAutoTransformSetPivotPoint, helpers::str_plus_info("Pivot point", object->getPivotPoint()), _item);
            menuItem->addSimpleAction(MenuActionAutoTransformSetMinimumScale, helpers::str_plus_info("Minimum scale", object->getMinimumScale()), _item);
            menuItem->addSimpleAction(MenuActionAutoTransformSetMaximumScale, helpers::str_plus_info("Maximum scale", object->getMaximumScale()), _item);
            menuItem->addSimpleAction(MenuActionAutoTransformSetAutoUpdateEyeMovementTolerance, helpers::str_plus_info("Auto update eye mvmt tolerance", object->getAutoUpdateEyeMovementTolerance()), _item);
            menuItem->addSimpleAction(MenuActionAutoTransformSetAutoScaleTransitionWidthRatio, helpers::str_plus_info("Auto scal trans width ratio", object->getAutoScaleTransitionWidthRatio()), _item);

            IContextMenuItem * autoRotateModeMenu = menuItem->addModeMenu(MenuActionAutoTransformAutoRotateMode, "Auto rotate mode", _item, object->getAutoRotateMode());
            if(autoRotateModeMenu)
            {
                autoRotateModeMenu->addModeAction("No rotation", osg::AutoTransform::NO_ROTATION);
                autoRotateModeMenu->addModeAction("To screen", osg::AutoTransform::ROTATE_TO_SCREEN);
                autoRotateModeMenu->addModeAction("To camera", osg::AutoTransform::ROTATE_TO_CAMERA);
                autoRotateModeMenu->addModeAction("To axis", osg::AutoTransform::ROTATE_TO_AXIS);
            }
            menuItem->addBoolAction(MenuActionAutoTransformAutoScaleToScreen, "Auto scale to screen", _item, object->getAutoScaleToScreen());
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::CameraView>::populate(IContextMenuItem * menuItem)
{
    osg::CameraView * object = static_cast<osg::CameraView*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Billboard>::populate(IContextMenuItem * menuItem)
{
    osg::Billboard * object = static_cast<osg::Billboard*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            osg::Billboard::Mode currentMode = object->getMode();
            IContextMenuItem * modeMenu = menuItem->addModeMenu(MenuActionBillboardMode, "Mode", _item, currentMode);
            if(modeMenu)
            {
                modeMenu->addModeAction("Rotate to Eye", osg::Billboard::POINT_ROT_EYE);
                modeMenu->addModeAction("Rotate to World", osg::Billboard::POINT_ROT_WORLD);
                modeMenu->addModeAction("Axial rotation", osg::Billboard::AXIAL_ROT);
            }
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Camera>::populate(IContextMenuItem * menuItem)
{
    osg::Camera * object = static_cast<osg::Camera*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            SGIHostItemOsg graphicscontext(object->getGraphicsContext());
            if(graphicscontext.hasObject())
                menuItem->addMenu("Graphics context", &graphicscontext);

            IContextMenuItem * computeNearFarModeMenu = menuItem->addModeMenu(MenuActionCameraComputeNearFarMode, "Compute near/far mode", _item, object->getComputeNearFarMode());
            if(computeNearFarModeMenu)
            {
                computeNearFarModeMenu->addModeAction("Do not compute", osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
                computeNearFarModeMenu->addModeAction("Using bounding volumes", osg::Camera::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
                computeNearFarModeMenu->addModeAction("Near/far using primitives", osg::Camera::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
                computeNearFarModeMenu->addModeAction("Near using primitives", osg::Camera::COMPUTE_NEAR_USING_PRIMITIVES);
            }
            IContextMenuItem * manipulateMenu = menuItem->getOrCreateMenu("Manipulate");
            if(manipulateMenu)
                manipulateMenu->addSimpleAction(MenuActionProxyNodeForceLoad, "Force load", _item);

            menuItem->addSimpleAction(MenuActionCameraClearColor, "Clear color...", _item);
            menuItem->addSimpleAction(MenuActionCameraCullSettings, "Cull settings...", _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::View>::populate(IContextMenuItem * menuItem)
{
    osg::View * object = dynamic_cast<osg::View*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgViewer::View>::populate(IContextMenuItem * menuItem)
{
    osgViewer::View * object = dynamic_cast<osgViewer::View*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            SGIHostItemOsg camera(object->getCamera());
            if(camera.hasObject())
                menuItem->addMenu("Camera", &camera);

            SGIHostItemOsg cameraManipulator(object->getCameraManipulator());
            if(cameraManipulator.hasObject())
                menuItem->addMenu("Camera manipulator", &cameraManipulator);

            SGIHostItemOsg viewerbase(object->getViewerBase());
            if(viewerbase.hasObject())
                menuItem->addMenu("ViewerBase", &viewerbase);

            SGIHostItemOsg scene(object->getScene());
            if(scene.hasObject())
                menuItem->addMenu("Scene", &scene);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgViewer::Scene>::populate(IContextMenuItem * menuItem)
{
    osgViewer::Scene * object = static_cast<osgViewer::Scene*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            SGIHostItemOsg sceneData(object->getSceneData());
            if(sceneData.hasObject())
                menuItem->addMenu("SceneData", &sceneData);

            SGIHostItemOsg imagepager(object->getImagePager());
            if(imagepager.hasObject())
                menuItem->addMenu("ImagePager", &imagepager);

            SGIHostItemOsg databasepager(object->getDatabasePager());
            if(databasepager.hasObject())
                menuItem->addMenu("DatabasePager", &databasepager);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::ProxyNode>::populate(IContextMenuItem * menuItem)
{
    osg::ProxyNode * object = static_cast<osg::ProxyNode*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionProxyNodeSetDatabasePath, helpers::str_plus_info("Database path", object->getDatabasePath()), _item);
            osg::ProxyNode::CenterMode centerMode = object->getCenterMode();
            IContextMenuItem * centerModeMenu = menuItem->addModeMenu(MenuActionProxyNodeSetCenterMode, "Center mode", _item, centerMode);
            if(centerModeMenu)
            {
                centerModeMenu->addModeAction("Bounding sphere", osg::ProxyNode::USE_BOUNDING_SPHERE_CENTER);
                centerModeMenu->addModeAction("User defined", osg::ProxyNode::USER_DEFINED_CENTER);
                centerModeMenu->addModeAction("Union bounding sphere and user defined", osg::ProxyNode::UNION_OF_BOUNDING_SPHERE_AND_USER_DEFINED);
            }
            menuItem->addSimpleAction(MenuActionProxyNodeSetCenter, helpers::str_plus_info("Center", object->getCenter()), _item);
            menuItem->addSimpleAction(MenuActionProxyNodeSetRadius, helpers::str_plus_info("Radius", object->getRadius()), _item);

            osg::ProxyNode::LoadingExternalReferenceMode loadingMode = object->getLoadingExternalReferenceMode();
            IContextMenuItem * loadingModeMenu = menuItem->addModeMenu(MenuActionProxyNodeLoadingExternalReferenceMode, "Loading mode", _item, loadingMode);
            if(loadingModeMenu)
            {
                loadingModeMenu->addModeAction("Immediately", osg::ProxyNode::LOAD_IMMEDIATELY);
                loadingModeMenu->addModeAction("Database pager", osg::ProxyNode::DEFER_LOADING_TO_DATABASE_PAGER);
                loadingModeMenu->addModeAction("No loading", osg::ProxyNode::NO_AUTOMATIC_LOADING);
            }
            IContextMenuItem * manipulateMenu = menuItem->getOrCreateMenu("Manipulate");
            if(manipulateMenu)
                manipulateMenu->addSimpleAction(MenuActionProxyNodeForceLoad, "Force load", _item);

        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::LOD>::populate(IContextMenuItem * menuItem)
{
    osg::LOD * object = getObject<osg::LOD,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            osg::LOD::CenterMode centerMode = object->getCenterMode();
            IContextMenuItem * centerModeMenu = menuItem->addModeMenu(MenuActionProxyNodeSetCenterMode, "Center mode", _item, centerMode);
            if(centerModeMenu)
            {
                centerModeMenu->addModeAction("Bounding sphere", osg::LOD::USE_BOUNDING_SPHERE_CENTER);
                centerModeMenu->addModeAction("User defined", osg::LOD::USER_DEFINED_CENTER);
                centerModeMenu->addModeAction("Union bounding sphere and user defined", osg::LOD::UNION_OF_BOUNDING_SPHERE_AND_USER_DEFINED);
            }
            menuItem->addSimpleAction(MenuActionProxyNodeSetCenter, helpers::str_plus_info("Center", object->getCenter()), _item);
            menuItem->addSimpleAction(MenuActionProxyNodeSetRadius, helpers::str_plus_info("Radius", object->getRadius()), _item);

            IContextMenuItem * rangeModeMenu = menuItem->addModeMenu(MenuActionLODSetRangeMode, "Range mode", _item, centerMode);
            if(rangeModeMenu)
            {
                centerModeMenu->addModeAction("Distance from eye point", osg::LOD::DISTANCE_FROM_EYE_POINT);
                centerModeMenu->addModeAction("Pixel size on screen", osg::LOD::PIXEL_SIZE_ON_SCREEN);
            }

        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::PagedLOD>::populate(IContextMenuItem * menuItem)
{
    osg::PagedLOD * object = getObject<osg::PagedLOD,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addBoolAction(MenuActionPagedLODDisableExternalChildrenPaging, "Disable external children paging", _item, object->getDisableExternalChildrenPaging());
            menuItem->addSimpleAction(MenuActionPagedLODNumChildrenThatCannotBeExpired, helpers::str_plus_info("Num children that cannot be expired", object->getNumChildrenThatCannotBeExpired()), _item);
            menuItem->addSimpleAction(MenuActionPagedLODFrameNumberOfLastTraversal, helpers::str_plus_info("Frame number of last traversal", object->getFrameNumberOfLastTraversal()), _item);
            menuItem->addSimpleAction(MenuActionProxyNodeSetDatabasePath, helpers::str_plus_info("Database path", object->getDatabasePath()), _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Drawable>::populate(IContextMenuItem * menuItem)
{
    osg::Drawable * object = static_cast<osg::Drawable*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addBoolAction(MenuActionDrawableToggleDisabled, "Disabled", _item, DisabledDrawCallback::isDisabled(object));
            SGIHostItemOsg stateSet(object->getStateSet());
            if(stateSet.hasObject())
                menuItem->addMenu("StateSet", &stateSet);

            SGIHostItemOsg shape(object->getShape());
            if(shape.hasObject())
                menuItem->addMenu("Shape", &shape);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::ShapeDrawable>::populate(IContextMenuItem * menuItem)
{
    osg::ShapeDrawable * object = static_cast<osg::ShapeDrawable*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            IContextMenuItem * manipulateMenu = menuItem->getOrCreateMenu("Manipulate");
            if(manipulateMenu)
                manipulateMenu->addSimpleAction(MenuActionShapeDrawableColor, "Color...", _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Geometry>::populate(IContextMenuItem * menuItem)
{
    osg::Geometry * object = static_cast<osg::Geometry*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            IContextMenuItem * manipulateMenu = menuItem->getOrCreateMenu("Manipulate");
            if(manipulateMenu)
            {
                manipulateMenu->addSimpleAction(MenuActionGeometryColor, "Color...", _item);
                manipulateMenu->addSimpleAction(MenuActionGeometryDirtyDisplayList, "Dirty display list", _item);
            }
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Image>::populate(IContextMenuItem * menuItem)
{
    osg::Image * object = static_cast<osg::Image*>(item<SGIItemOsg>()->object());
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



bool contextMenuPopulateImpl<osg::Sphere>::populate(IContextMenuItem * menuItem)
{
    osg::Sphere * object = getObject<osg::Sphere,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionShapeCenter, helpers::str_plus_info("Center", object->getCenter()), _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Box>::populate(IContextMenuItem * menuItem)
{
    osg::Box * object = getObject<osg::Box,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionShapeCenter, helpers::str_plus_info("Center", object->getCenter()), _item);
            menuItem->addSimpleAction(MenuActionShapeRotation, helpers::str_plus_info("Rotation", object->getRotation()), _item);
            menuItem->addSimpleAction(MenuActionShapeBoxHalfLength, helpers::str_plus_info("Half length", object->getHalfLengths()), _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Cone>::populate(IContextMenuItem * menuItem)
{
    osg::Cone * object = getObject<osg::Cone,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionShapeCenter, helpers::str_plus_info("Center", object->getCenter()), _item);
            menuItem->addSimpleAction(MenuActionShapeRotation, helpers::str_plus_info("Rotation", object->getRotation()), _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Cylinder>::populate(IContextMenuItem * menuItem)
{
    osg::Cylinder * object = getObject<osg::Cylinder,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionShapeCenter, helpers::str_plus_info("Center", object->getCenter()), _item);
            menuItem->addSimpleAction(MenuActionShapeRotation, helpers::str_plus_info("Rotation", object->getRotation()), _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Capsule>::populate(IContextMenuItem * menuItem)
{
    osg::Capsule * object = getObject<osg::Capsule,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionShapeCenter, helpers::str_plus_info("Center", object->getCenter()), _item);
            menuItem->addSimpleAction(MenuActionShapeRotation, helpers::str_plus_info("Rotation", object->getRotation()), _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::InfinitePlane>::populate(IContextMenuItem * menuItem)
{
    osg::InfinitePlane * object = getObject<osg::InfinitePlane,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::TriangleMesh>::populate(IContextMenuItem * menuItem)
{
    osg::TriangleMesh * object = getObject<osg::TriangleMesh,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::HeightField>::populate(IContextMenuItem * menuItem)
{
    osg::HeightField * object = getObject<osg::HeightField,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionHeightFieldPreview, "Preview...", _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::CompositeShape>::populate(IContextMenuItem * menuItem)
{
    osg::CompositeShape * object = getObject<osg::CompositeShape,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::StateAttribute>::populate(IContextMenuItem * menuItem)
{
    osg::StateAttribute * object = getObject<osg::StateAttribute,SGIItemOsg>();

    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Texture>::populate(IContextMenuItem * menuItem)
{
    osg::Texture * object = static_cast<osg::Texture*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionTexturePreview, "Preview...", _item);
            menuItem->addSimpleAction(MenuActionTextureSetImage, "Set image...", _item);
            menuItem->addSimpleAction(MenuActionTextureBorderWidth, "Border width...", _item);
            menuItem->addSimpleAction(MenuActionTextureBorderColor, "Border color...", _item);

            IContextMenuItem * minFilterMenu = menuItem->addModeMenu(MenuActionTextureSetMinFilter, "Min filter", _item, object->getFilter(osg::Texture::MIN_FILTER));
            if(minFilterMenu)
            {
                minFilterMenu->addModeAction("Linear", osg::Texture::LINEAR);
                minFilterMenu->addModeAction("Linear, MipMap linear", osg::Texture::LINEAR_MIPMAP_LINEAR);
                minFilterMenu->addModeAction("Linear, MipMap nearest", osg::Texture::LINEAR_MIPMAP_NEAREST);
                minFilterMenu->addModeAction("Nearest", osg::Texture::NEAREST);
                minFilterMenu->addModeAction("Nearest, MipMap linear", osg::Texture::NEAREST_MIPMAP_LINEAR);
                minFilterMenu->addModeAction("Nearest, MipMap nearest", osg::Texture::NEAREST_MIPMAP_NEAREST);
            }
            IContextMenuItem * magFilterMenu = menuItem->addModeMenu(MenuActionTextureSetMinFilter, "Mag filter", _item, object->getFilter(osg::Texture::MAG_FILTER));
            if(magFilterMenu)
            {
                magFilterMenu->addModeAction("Linear", osg::Texture::LINEAR);
                magFilterMenu->addModeAction("Linear, MipMap linear", osg::Texture::LINEAR_MIPMAP_LINEAR);
                magFilterMenu->addModeAction("Linear, MipMap nearest", osg::Texture::LINEAR_MIPMAP_NEAREST);
                magFilterMenu->addModeAction("Nearest", osg::Texture::NEAREST);
                magFilterMenu->addModeAction("Nearest, MipMap linear", osg::Texture::NEAREST_MIPMAP_LINEAR);
                magFilterMenu->addModeAction("Nearest, MipMap nearest", osg::Texture::NEAREST_MIPMAP_NEAREST);
            }

            menuItem->addBoolAction(MenuActionTextureSetResizeNonPowerOfTwoHint, "Resize NPOT", _item, object->getResizeNonPowerOfTwoHint());
            menuItem->addBoolAction(MenuActionTextureUseHardwareMipMapGeneration, "Use hardware MipMap Generation", _item, object->getUseHardwareMipMapGeneration());
            menuItem->addBoolAction(MenuActionTextureUnRefImageDataAfterApply, "UnRef image data after apply", _item, object->getUnRefImageDataAfterApply());

        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Texture2D>::populate(IContextMenuItem * menuItem)
{
    osg::Texture2D * object = static_cast<osg::Texture2D*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Texture3D>::populate(IContextMenuItem * menuItem)
{
    osg::Texture3D * object = static_cast<osg::Texture3D*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::ClipNode>::populate(IContextMenuItem * menuItem)
{
    osg::ClipNode * object = static_cast<osg::ClipNode*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionClipNodeReset, "Reset", _item);

            StateAttributeModeValue currentMode = getStateAttributeModeFromOverrideValue(((const ClipNodeAccess*)object)->value());
            createStateAttributeModeMenu(MenuActionClipNodeSetState, GL_CLIP_PLANE0, menuItem, "State", _item, currentMode);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::GraphicsContext>::populate(IContextMenuItem * menuItem)
{
    osg::GraphicsContext * object = static_cast<osg::GraphicsContext*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osg::Shader>::populate(IContextMenuItem * menuItem)
{
    osg::Shader * object = static_cast<osg::Shader*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
    case SGIItemTypeShaderSource:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionShaderEditSource, "Edit...", _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgViewer::GraphicsWindow>::populate(IContextMenuItem * menuItem)
{
    osgViewer::GraphicsWindow * object = static_cast<osgViewer::GraphicsWindow*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionGraphicsWindowHitTest, "Hit test", _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgQt::GraphicsWindowQt>::populate(IContextMenuItem * menuItem)
{
    osgQt::GraphicsWindowQt * object = static_cast<osgQt::GraphicsWindowQt*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            SGIHostItemQt widget(object->getGLWidget());
            if(widget.hasObject())
                menuItem->addMenu("GLWidget", &widget);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgDB::Registry>::populate(IContextMenuItem * menuItem)
{
    osgDB::Registry * object = getObject<osgDB::Registry,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            osg::NotifySeverity notifyLevel = osg::getNotifyLevel();
            IContextMenuItem * notifyLevelMenu = menuItem->addModeMenu(MenuActionNotifyLevel, "Notify level", _item, notifyLevel);
            if(notifyLevelMenu)
            {
                notifyLevelMenu->addModeAction("Always", osg::ALWAYS);
                notifyLevelMenu->addModeAction("Fatal", osg::FATAL);
                notifyLevelMenu->addModeAction("Warn", osg::WARN);
                notifyLevelMenu->addModeAction("Notice", osg::NOTICE);
                notifyLevelMenu->addModeAction("Info", osg::INFO);
                notifyLevelMenu->addModeAction("Debug info", osg::DEBUG_INFO);
                notifyLevelMenu->addModeAction("Debug FP", osg::DEBUG_FP);
            }
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgDB::ImagePager>::populate(IContextMenuItem * menuItem)
{
    osgDB::ImagePager * object = static_cast<osgDB::ImagePager*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgDB::DatabasePager>::populate(IContextMenuItem * menuItem)
{
    osgDB::DatabasePager * object = static_cast<osgDB::DatabasePager*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addBoolAction(MenuActionDatabasePagerPause, "Pause", _item, object->getDatabasePagerThreadPause());
            menuItem->addBoolAction(MenuActionDatabasePagerAcceptNewRequests, "Accept new requests", _item, object->getAcceptNewDatabaseRequests());
            menuItem->addBoolAction(MenuActionDatabasePagerDoPreCompile, "Do pre-compile", _item, object->getDoPreCompile());
            menuItem->addBoolAction(MenuActionDatabasePagerDeleteSubgraphsInDBThread, "Delete Subgraphs in DB thread", _item, object->getDeleteRemovedSubgraphsInDatabaseThread());

            menuItem->addSimpleAction(MenuActionDatabasePagerTargetPageLODNumber, helpers::str_plus_count("Target PagedLOD number",object->getTargetMaximumNumberOfPageLOD()), _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgGA::GUIEventHandler>::populate(IContextMenuItem * menuItem)
{
    osgGA::GUIEventHandler * object = dynamic_cast<osgGA::GUIEventHandler*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
#if OSG_VERSION_LESS_THAN(3,3,1)
            menuItem->addSimpleAction(MenuActionEventHandlerIgnoreHandledEventsMask, helpers::str_plus_hex("Ignore handled events mask",object->getIgnoreHandledEventsMask()), _item);
#endif
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgGA::GUIEventAdapter>::populate(IContextMenuItem * menuItem)
{
    osgGA::GUIEventAdapter * object = static_cast<osgGA::GUIEventAdapter*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionEventAdapterHitTest, "Hit test", _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgGA::CameraManipulator>::populate(IContextMenuItem * menuItem)
{
    osgGA::CameraManipulator * object = dynamic_cast<osgGA::CameraManipulator*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addBoolAction(MenuActionCameraManipulatorAutoComputeHome, "Auto compute home", _item, object->getAutoComputeHomePosition());

            SGIHostItemOsg node(object->getNode());
            if(node.hasObject())
                menuItem->addMenu("Node", &node);

        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgText::TextBase>::populate(IContextMenuItem * menuItem)
{
    osgText::TextBase * object = getObject<osgText::TextBase, SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addBoolAction(MenuActionTextBaseAutoRotateToScreen, "Auto rotate to screen", _item, object->getAutoRotateToScreen());
            menuItem->addSimpleAction(MenuActionTextBaseCharacterHeight, helpers::str_plus_info("Character height", object->getCharacterHeight()), _item);
            menuItem->addSimpleAction(MenuActionTextBaseCharacterAspectRatio, helpers::str_plus_info("Character aspect ratio", object->getCharacterAspectRatio()), _item);

            IContextMenuItem * characterSizeMenu = menuItem->addModeMenu(MenuActionTextBaseCharacterSizeMode, "Character size mode", _item, object->getCharacterSizeMode());
            if(characterSizeMenu)
            {
                characterSizeMenu->addModeAction("Object", osgText::TextBase::OBJECT_COORDS);
                characterSizeMenu->addModeAction("Screen", osgText::TextBase::SCREEN_COORDS);
                characterSizeMenu->addModeAction("Screen+Max font height", osgText::TextBase::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
            }
            IContextMenuItem * drawModeMenu = menuItem->addModeMenu(MenuActionTextBaseDrawMode, "Draw mode", _item, object->getDrawMode());
            if(drawModeMenu)
            {
                drawModeMenu->addModeAction("Text", osgText::Text::TEXT);
                drawModeMenu->addModeAction("BoundingBox", osgText::Text::BOUNDINGBOX);
                drawModeMenu->addModeAction("FilledBoundingBox", osgText::Text::FILLEDBOUNDINGBOX);
                drawModeMenu->addModeAction("Alignment", osgText::Text::ALIGNMENT);
                drawModeMenu->addModeAction("Text+BBox", osgText::Text::TEXT|osgText::Text::BOUNDINGBOX);
                drawModeMenu->addModeAction("Text+FilledBBox", osgText::Text::TEXT|osgText::Text::FILLEDBOUNDINGBOX);
            }
            IContextMenuItem * axisAlignmentMenu = menuItem->addModeMenu(MenuActionTextBaseAxisAlignment, "Axis alignment", _item, object->getAxisAlignment());
            if(axisAlignmentMenu)
            {
                axisAlignmentMenu->addModeAction("XY plane", osgText::Text::XY_PLANE);
                axisAlignmentMenu->addModeAction("Rev-XY plane", osgText::Text::REVERSED_XY_PLANE);
                axisAlignmentMenu->addModeAction("XZ plane", osgText::Text::XZ_PLANE);
                axisAlignmentMenu->addModeAction("Rev-XZ plane", osgText::Text::REVERSED_XZ_PLANE);
                axisAlignmentMenu->addModeAction("YZ plane", osgText::Text::YZ_PLANE);
                axisAlignmentMenu->addModeAction("Rev-YZ plane", osgText::Text::REVERSED_YZ_PLANE);
                axisAlignmentMenu->addModeAction("Screen", osgText::Text::SCREEN);
                axisAlignmentMenu->addModeAction("User defined", osgText::Text::USER_DEFINED_ROTATION);
            }

            menuItem->addSimpleAction(MenuActionTextBaseModifyText, "Modify text...", _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgText::Text>::populate(IContextMenuItem * menuItem)
{
    osgText::Text * object = getObject<osgText::Text, SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            IContextMenuItem * backdropTypeMenu = menuItem->addModeMenu(MenuActionTextBackdropType, "Backdrop type", _item, object->getBackdropType());
            if(backdropTypeMenu)
            {
                backdropTypeMenu->addModeAction("BOTTOM_RIGHT", osgText::Text::DROP_SHADOW_BOTTOM_RIGHT);
                backdropTypeMenu->addModeAction("CENTER_RIGHT", osgText::Text::DROP_SHADOW_CENTER_RIGHT);
                backdropTypeMenu->addModeAction("TOP_RIGHT", osgText::Text::DROP_SHADOW_TOP_RIGHT);
                backdropTypeMenu->addModeAction("BOTTOM_CENTER", osgText::Text::DROP_SHADOW_BOTTOM_CENTER);
                backdropTypeMenu->addModeAction("TOP_CENTER", osgText::Text::DROP_SHADOW_TOP_CENTER);
                backdropTypeMenu->addModeAction("BOTTOM_LEFT", osgText::Text::DROP_SHADOW_BOTTOM_LEFT);
                backdropTypeMenu->addModeAction("CENTER_LEFT", osgText::Text::DROP_SHADOW_CENTER_LEFT);
                backdropTypeMenu->addModeAction("TOP_LEFT", osgText::Text::DROP_SHADOW_TOP_LEFT);
                backdropTypeMenu->addModeAction("Outline", osgText::Text::OUTLINE);
                backdropTypeMenu->addModeAction("None", osgText::Text::NONE);
            }
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgAnimation::AnimationManagerBase>::populate(IContextMenuItem * menuItem)
{
    osgAnimation::AnimationManagerBase * object = getObject<osgAnimation::AnimationManagerBase, SGIItemOsg, DynamicCaster>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addBoolAction(MenuActionAnimationManagerBaseAutomaticLink, "Automatic link", _item, object->getAutomaticLink());
            menuItem->addBoolAction(MenuActionAnimationManagerBaseDirty, "Dirty", _item, object->needToLink());

            AnimationManagerBaseAccess * access = (AnimationManagerBaseAccess*)object;
            SGIHostItemOsg linkVisitor(access->getLinkVisitor());
            if(linkVisitor.hasObject())
                menuItem->addMenu("LinkVisitor", &linkVisitor);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<sgi::ISceneGraphDialogToolsMenu>::populate(IContextMenuItem * menuItem)
{
    ISceneGraphDialogToolsMenu * object = static_cast<ISceneGraphDialogToolsMenu*>(item<SGIItemInternal>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        menuItem->addSimpleAction(MenuActionToolFindUpdateNodes, "Find update nodes", _item);
        menuItem->addSimpleAction(MenuActionToolFindEventNodes, "Find event nodes", _item);
        menuItem->addSimpleAction(MenuActionToolFindNaNNodes, "Find NaN nodes", _item);
        menuItem->addSimpleAction(MenuActionToolFindVisibleNodes, "Find visible", _item);
        menuItem->addSimpleAction(MenuActionToolListNodeMasks, "List node masks", _item);
        menuItem->addSimpleAction(MenuActionToolFindPagedLODChilds, "Find PagedLOD childs", _item);
        menuItem->addSimpleAction(MenuActionToolEffectiveStateSet, "Effective StateSet", _item);
        menuItem->addSimpleAction(MenuActionToolFindCamera, "Find camera", _item);
        menuItem->addSimpleAction(MenuActionToolFindView, "Find view", _item);
        menuItem->addSimpleAction(MenuActionToolDistanceToCamera, "Distance to camera", _item);
        ret = true;
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

} // namespace osg_plugin
} // namespace sgi
