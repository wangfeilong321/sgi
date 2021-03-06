#include "stdafx.h"
#include "ContextMenuOSGEarth.h"
#include <sgi/plugins/SGIItemOsg>
#include <sgi/helpers/osg>
#include <sgi/helpers/string>

#include <osgEarth/Version>
#include <osgEarth/MapNode>
#if OSGEARTH_VERSION_GREATER_OR_EQUAL(2,6,0)
#include <osgEarthUtil/Sky>
#else
#include <osgEarthUtil/SkyNode>
#endif
#include <osgEarthUtil/AutoClipPlaneHandler>
#include <osgEarthUtil/Controls>
#include <osgEarth/TileSource>
#include <osgEarth/Registry>
//#include <osgEarth/TimeControl>

#include "MenuActionOSGEarth.h"

#include "osgearth_accessor.h"
#include "ElevationQueryReferenced"

namespace sgi {

class SGIItemOsg;

namespace osgearth_plugin {

CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Node)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osg::Image)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::Registry)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::Map)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::MapNode)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::Layer)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::TerrainLayer)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::ImageLayer)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::ElevationLayer)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::ModelLayer)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::MaskLayer)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::Util::SkyNode)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::Util::AutoClipPlaneCullCallback)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::Util::Controls::Control)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::Util::Controls::Container)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::Util::Controls::ControlCanvas)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::TileSource)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::TileBlacklist)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(osgEarth::VirtualProgram)
CONTEXT_MENU_POPULATE_IMPL_REGISTER(ElevationQueryReferenced)

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
                manipulateMenu->addSimpleAction(MenuActionNodeRegenerateShaders, "Re-generate shaders", _item);
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
    osg::Image* object = getObject<osg::Image,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionImagePreviewRGBA, "Preview RGBA", _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgEarth::Registry>::populate(IContextMenuItem * menuItem)
{
    osgEarth::Registry * object = getObject<osgEarth::Registry,SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            osg::NotifySeverity notifyLevel = osgEarth::getNotifyLevel();
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

bool contextMenuPopulateImpl<osgEarth::Map>::populate(IContextMenuItem * menuItem)
{
    osgEarth::Map * object = static_cast<osgEarth::Map*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        {
            ret = callNextHandler(menuItem);
            if(ret)
            {
                SGIHostItemOsg cache(object->getCache());
                if(cache.hasObject())
                    menuItem->addMenu("Cache", &cache);

                const osgEarth::MapOptions & mapOptions = object->getMapOptions();
                osgEarth::CachePolicy::Usage cachePolicyUsage = mapOptions.cachePolicy().value().usage().value();

                IContextMenuItem * cacheMenu = menuItem->addModeMenu(MenuActionMapCachePolicyUsage, "Cache usage", _item, cachePolicyUsage);
                if(cacheMenu)
                {
                    cacheMenu->addModeAction("Read/write", osgEarth::CachePolicy::USAGE_READ_WRITE);
                    cacheMenu->addModeAction("Cache only", osgEarth::CachePolicy::USAGE_CACHE_ONLY);
                    cacheMenu->addModeAction("Read only", osgEarth::CachePolicy::USAGE_READ_ONLY);
                    cacheMenu->addModeAction("No cache", osgEarth::CachePolicy::USAGE_NO_CACHE);
                }

                menuItem->addSimpleAction(MenuActionTileInspector, "Tile inspector...", _item);
                unsigned numImage = object->getNumImageLayers();
                if(numImage || true)
                    menuItem->addMenu(helpers::str_plus_count("Image layers", numImage), cloneItem<SGIItemOsg>(SGIItemTypeImageLayers));
                unsigned numElev = object->getNumElevationLayers();
                if(numElev)
                    menuItem->addMenu(helpers::str_plus_count("Elevation layers", numElev), cloneItem<SGIItemOsg>(SGIItemTypeElevationLayers));
                unsigned numModel = object->getNumModelLayers();
                if(numModel)
                    menuItem->addMenu(helpers::str_plus_count("Model layers", numModel), cloneItem<SGIItemOsg>(SGIItemTypeModelLayers));
                unsigned numMask = 0;
                {
                    osgEarth::MaskLayerVector maskLayers;
                    object->getTerrainMaskLayers(maskLayers);
                    numMask = maskLayers.size();
                }
                if(numMask)
                    menuItem->addMenu(helpers::str_plus_count("Mask layers", numMask), cloneItem<SGIItemOsg>(SGIItemTypeMaskLayers));
            }
        }
        break;
    case SGIItemTypeElevationLayers:
        {
            osgEarth::MapFrame mapframe(object, osgEarth::Map::ELEVATION_LAYERS);
            const osgEarth::ElevationLayerVector & elevationLayers = mapframe.elevationLayers();
            for(osgEarth::ElevationLayerVector::const_iterator it = elevationLayers.begin(); it != elevationLayers.end(); it++)
            {
                const osg::ref_ptr<osgEarth::ElevationLayer> & layer = *it;
                SGIHostItemOsg childItem(layer.get());
                menuItem->addMenu(std::string(), &childItem);
            }
            ret = true;
        }
        break;
    case SGIItemTypeImageLayers:
        {
            osg::ref_ptr<osgEarth::ImageLayer> imageLayer;
            MapDebugImageLayer debugImageLayer = getDebugImageLayer(object, imageLayer);
            IContextMenuItem * debugImageLayerMenu = menuItem->addModeMenu(MenuActionMapDebugImageLayer, "Debug image layer", _item, debugImageLayer);
            if(debugImageLayerMenu)
            {
                debugImageLayerMenu->addModeAction("Off", MapDebugImageLayerNone);
                debugImageLayerMenu->addModeAction("Normal", MapDebugImageLayerNormal);
                debugImageLayerMenu->addModeAction("Inverted", MapDebugImageLayerInverted);
            }

            osgEarth::MapFrame mapframe(object, osgEarth::Map::IMAGE_LAYERS);
            const osgEarth::ImageLayerVector & imageLayers = mapframe.imageLayers();
            if(!imageLayers.empty())
            {
                menuItem->addSeparator();
                for(osgEarth::ImageLayerVector::const_iterator it = imageLayers.begin(); it != imageLayers.end(); it++)
                {
                    const osg::ref_ptr<osgEarth::ImageLayer> & layer = *it;
                    SGIHostItemOsg childItem(layer.get());
                    menuItem->addMenu(std::string(), &childItem);
                }
            }
            ret = true;
        }
        break;
    case SGIItemTypeModelLayers:
        {
            osgEarth::MapFrame mapframe(object, osgEarth::Map::MODEL_LAYERS);
            const osgEarth::ModelLayerVector & modelLayers = mapframe.modelLayers();
            for(osgEarth::ModelLayerVector::const_iterator it = modelLayers.begin(); it != modelLayers.end(); it++)
            {
                const osg::ref_ptr<osgEarth::ModelLayer> & layer = *it;
                SGIHostItemOsg childItem(layer.get());
                menuItem->addMenu(std::string(), &childItem);
            }
            ret = true;
        }
        break;
    case SGIItemTypeMaskLayers:
        {
            osgEarth::MapFrame mapframe(object, osgEarth::Map::MASK_LAYERS);
            const osgEarth::MaskLayerVector & maskLayers = mapframe.terrainMaskLayers();
            for(osgEarth::MaskLayerVector::const_iterator it = maskLayers.begin(); it != maskLayers.end(); it++)
            {
                const osg::ref_ptr<osgEarth::MaskLayer> & layer = *it;
                SGIHostItemOsg childItem(layer.get());
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

bool contextMenuPopulateImpl<osgEarth::MapNode>::populate(IContextMenuItem * menuItem)
{
    osgEarth::MapNode * object = static_cast<osgEarth::MapNode*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        {
            ret = callNextHandler(menuItem);
            if(ret)
            {
                SGIHostItemOsg map(object->getMap());
                if(map.hasObject())
                    menuItem->addMenu("Map", &map);
                menuItem->addSimpleAction(MenuActionTileInspector, "Tile inspector...", _item);
            }
        }
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgEarth::Layer>::populate(IContextMenuItem * menuItem)
{
    osgEarth::Layer * object = static_cast<osgEarth::Layer*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            /*
            SGIHostItemOsg sequencecontrol(object->getSequenceControl());
            if(sequencecontrol.hasObject())
                menuItem->addMenu("Sequence control", &sequencecontrol);
            */
        }
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgEarth::TerrainLayer>::populate(IContextMenuItem * menuItem)
{
    osgEarth::TerrainLayer * object = static_cast<osgEarth::TerrainLayer*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addBoolAction(MenuActionTerrainLayerEnable, "Enable", _item, object->getEnabled());
            menuItem->addBoolAction(MenuActionTerrainLayerVisible, "Visible", _item, object->getVisible());

            std::string url = ((TerrainLayerAccessor*)object)->getURL();
            if(!url.empty())
                menuItem->addSimpleAction(MenuActionTerrainLayerSetURL, helpers::str_plus_info("URL", url), _item);

            SGIHostItemOsg cache(object->getCache());
            if(cache.hasObject())
                menuItem->addMenu("Cache", &cache);

            osgEarth::CachePolicy::Usage cachePolicyUsage = ((TerrainLayerAccessor*)object)->getCachePolicy().usage().value();
            IContextMenuItem * cacheMenu = menuItem->addModeMenu(MenuActionTerrainLayerCacheUsage, "Cache usage", _item, cachePolicyUsage);
            if(cacheMenu)
            {
                cacheMenu->addModeAction("Read/write", osgEarth::CachePolicy::USAGE_READ_WRITE);
                cacheMenu->addModeAction("Cache only", osgEarth::CachePolicy::USAGE_CACHE_ONLY);
                cacheMenu->addModeAction("Read only", osgEarth::CachePolicy::USAGE_READ_ONLY);
                cacheMenu->addModeAction("No cache", osgEarth::CachePolicy::USAGE_NO_CACHE);
            }

            SGIHostItemOsg tilesource(object->getTileSource());
            if(tilesource.hasObject())
                menuItem->addMenu("Tile source", &tilesource);

            menuItem->addSimpleAction(MenuActionTileInspector, "Tile inspector...", _item);
        }

        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgEarth::ImageLayer>::populate(IContextMenuItem * menuItem)
{
    osgEarth::ImageLayer * object = static_cast<osgEarth::ImageLayer*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionImageLayerCreateImage, "Create image...", _item);
        }
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgEarth::ElevationLayer>::populate(IContextMenuItem * menuItem)
{
    osgEarth::ElevationLayer * object = static_cast<osgEarth::ElevationLayer*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionElevationLayerCreateHeightField, "Create height field...", _item);
        }
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgEarth::ModelLayer>::populate(IContextMenuItem * menuItem)
{
    osgEarth::ModelLayer * object = static_cast<osgEarth::ModelLayer*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addBoolAction(MenuActionModelLayerEnable, "Enable", _item, object->getEnabled());
            menuItem->addBoolAction(MenuActionModelLayerVisible, "Visible", _item, object->getVisible());
            menuItem->addBoolAction(MenuActionModelLayerLighting, "Lighting", _item, object->isLightingEnabled());

            SGIHostItemOsg modelsource(object->getModelSource());
            if(modelsource.hasObject())
            {
                menuItem->addMenu("Model source", &modelsource);

                std::string url;
                const osgEarth::ModelSourceOptions & layerOpts = static_cast<osgEarth::ModelSource*>(modelsource.object())->getOptions();
                osgEarth::Config layerConf = layerOpts.getConfig();
                if(layerConf.hasValue("url"))
                    url = layerConf.value("url");
                if(!url.empty())
                    menuItem->addSimpleAction(MenuActionModelLayerSetURL, helpers::str_plus_info("URL", url), _item);
            }
        }
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgEarth::MaskLayer>::populate(IContextMenuItem * menuItem)
{
    osgEarth::MaskLayer * object = static_cast<osgEarth::MaskLayer*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            SGIHostItemOsg masksource(object->getMaskSource());
            if(masksource.hasObject())
            {
                menuItem->addMenu("Mask source", &masksource);

                std::string url;
                const osgEarth::MaskSourceOptions & layerOpts = static_cast<osgEarth::MaskSource*>(masksource.object())->getOptions();
                osgEarth::Config layerConf = layerOpts.getConfig();
                if(layerConf.hasValue("url"))
                    url = layerConf.value("url");
                if(!url.empty())
                    menuItem->addSimpleAction(MenuActionMaskLayerSetURL, helpers::str_plus_info("URL", url), _item);
            }
        }
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgEarth::Util::SkyNode>::populate(IContextMenuItem * menuItem)
{
    osgEarth::Util::SkyNode * object = static_cast<osgEarth::Util::SkyNode*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionSkyNodeLightSettings, "Light settings...", _item);

#if OSGEARTH_VERSION_GREATER_OR_EQUAL(2,6,0)
            const osgEarth::DateTime & t = object->getDateTime();
#else
            osgEarth::DateTime t;
            object->getDateTime(t);
#endif
            menuItem->addSimpleAction(MenuActionSkyNodeSetDateTime, helpers::str_plus_info("Date/time", t.asRFC1123()), _item);

#if OSGEARTH_VERSION_GREATER_OR_EQUAL(2,6,0)
            menuItem->addBoolAction(MenuActionSkyNodeSetSunVisible, "Sun", _item, object->getSunVisible());
#endif
            menuItem->addBoolAction(MenuActionSkyNodeSetMoonVisible, "Moon", _item, object->getMoonVisible());
            menuItem->addBoolAction(MenuActionSkyNodeSetStarsVisible, "Stars", _item, object->getStarsVisible());
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgEarth::Util::AutoClipPlaneCullCallback>::populate(IContextMenuItem * menuItem)
{
    osgEarth::Util::AutoClipPlaneCullCallback * object = getObject<osgEarth::Util::AutoClipPlaneCullCallback,SGIItemOsg,DynamicCaster>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionAutoClipPlaneCullCallbackSetup, "Setup...", _item);
            menuItem->addSimpleAction(MenuActionAutoClipPlaneCullCallbackMinNearFarRatio, helpers::str_plus_info("MinNearFarRatio", object->getMinNearFarRatio()), _item);
            menuItem->addSimpleAction(MenuActionAutoClipPlaneCullCallbackMaxNearFarRatio, helpers::str_plus_info("MaxNearFarRatio", object->getMaxNearFarRatio()), _item);
            menuItem->addSimpleAction(MenuActionAutoClipPlaneCullCallbackHeightThreshold, helpers::str_plus_info("HeightThreshold", object->getHeightThreshold()), _item);
            menuItem->addBoolAction(MenuActionAutoClipPlaneCullCallbackClampFarClipPlane, "ClampFarClipPlane", _item, object->getClampFarClipPlane());
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgEarth::Util::Controls::Control>::populate(IContextMenuItem * menuItem)
{
    osgEarth::Util::Controls::Control * object = static_cast<osgEarth::Util::Controls::Control*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            ControlAccess * access = (ControlAccess*)object;

            IContextMenuItem * manipulateMenu = menuItem->getOrCreateMenu("Manipulate");
            if(manipulateMenu)
            {
                manipulateMenu->addBoolAction(MenuActionControlDirty, "Dirty", _item, object->isDirty());
                manipulateMenu->addBoolAction(MenuActionControlActive, "Active", _item, object->getActive());
                manipulateMenu->addBoolAction(MenuActionControlAbsorbEvents, "Absorb events", _item, object->getAbsorbEvents());
            }

            SGIHostItemOsg geode(access->geode());
            if(geode.hasObject())
                menuItem->addMenu("Geode", &geode);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgEarth::Util::Controls::Container>::populate(IContextMenuItem * menuItem)
{
    osgEarth::Util::Controls::Container * object = static_cast<osgEarth::Util::Controls::Container*>(item<SGIItemOsg>()->object());
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

bool contextMenuPopulateImpl<osgEarth::Util::Controls::ControlCanvas>::populate(IContextMenuItem * menuItem)
{
    osgEarth::Util::Controls::ControlCanvas * object = static_cast<osgEarth::Util::Controls::ControlCanvas*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            ControlCanvasAccess* access = (ControlCanvasAccess*)object;
            const osgEarth::Util::Controls::ControlContext & controlContext = access->controlContext();

            IContextMenuItem * manipulateMenu = menuItem->getOrCreateMenu("Manipulate");
            if(manipulateMenu)
            {
                manipulateMenu->addBoolAction(MenuActionControlCanvasDirty, "Dirty", _item, access->isContextDirty());
            }

            SGIHostItemOsg controlNodeBin(access->controlNodeBin());
            if(controlNodeBin.hasObject())
                menuItem->addMenu("ControlNodeBin", &controlNodeBin);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}


bool contextMenuPopulateImpl<osgEarth::TileSource>::populate(IContextMenuItem * menuItem)
{
    osgEarth::TileSource * object = dynamic_cast<osgEarth::TileSource*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            SGIHostItemOsg profile(object->getProfile());
            if(profile.hasObject())
                menuItem->addMenu("Profile", &profile);

            SGIHostItemOsg blacklist(object->getBlacklist());
            if(blacklist.hasObject())
                menuItem->addMenu("Blacklist", &blacklist);

            menuItem->addSimpleAction(MenuActionTileSourceCreateImage, "Create image...", _item);
            menuItem->addSimpleAction(MenuActionTileSourceCreateHeightField, "Create height field...", _item);
            menuItem->addSimpleAction(MenuActionTileSourceUpdateMetaData, "Update meta data", _item);
            menuItem->addSimpleAction(MenuActionTileInspector, "Tile inspector...", _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgEarth::TileBlacklist>::populate(IContextMenuItem * menuItem)
{
    osgEarth::TileBlacklist * object = dynamic_cast<osgEarth::TileBlacklist*>(item<SGIItemOsg>()->object());
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionTileBlacklistClear, "Clear", _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<ElevationQueryReferenced>::populate(IContextMenuItem * menuItem)
{
    ElevationQueryReferenced * query_ref = getObject<ElevationQueryReferenced, SGIItemOsg>();
    osgEarth::ElevationQuery* object = query_ref->get();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            menuItem->addSimpleAction(MenuActionElevationQueryCustom, "Query...", _item);
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

bool contextMenuPopulateImpl<osgEarth::VirtualProgram>::populate(IContextMenuItem * menuItem)
{
    osgEarth::VirtualProgram * object = getObject<osgEarth::VirtualProgram, SGIItemOsg>();
    bool ret = false;
    switch(itemType())
    {
    case SGIItemTypeObject:
        ret = callNextHandler(menuItem);
        if(ret)
        {
            VirtualProgramAccessor * access = (VirtualProgramAccessor*)object;

            menuItem->addSimpleAction(MenuActionVirtualProgramMask, helpers::str_plus_hex("Mask", access->mask()), _item);

            VirtualProgramInheritMode mode;
            if(!access->inheritSet())
                mode = VirtualProgramInheritModeUnspecified;
            else if(access->inherit())
                mode = VirtualProgramInheritModeEnabled;
            else
                mode = VirtualProgramInheritModeDisabled;
            IContextMenuItem * inheritMenu = menuItem->addModeMenu(MenuActionVirtualProgramInherit, "Inherit", _item, mode);
            if(inheritMenu)
            {
                inheritMenu->addModeAction("Unspecified", VirtualProgramInheritModeUnspecified);
                inheritMenu->addModeAction("Enabled", VirtualProgramInheritModeEnabled);
                inheritMenu->addModeAction("Disabled", VirtualProgramInheritModeDisabled);
            }
        }
        break;
    default:
        ret = callNextHandler(menuItem);
        break;
    }
    return ret;
}

} // namespace osgearth_plugin
} // namespace sgi
