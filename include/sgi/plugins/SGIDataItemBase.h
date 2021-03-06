#pragma once
#include "SGIItemBase.h"
#include "SGIPluginInterface.h"

#ifdef max
#undef max
#endif

namespace sgi {

class SGIPluginHostInterface;

class SGIDataFieldBase : public osg::Referenced
{
public:
    SGIDataFieldBase() : osg::Referenced() {}
    SGIDataFieldBase(const SGIDataFieldBase & rhs) : osg::Referenced(rhs) {}

    virtual std::string toString(SGIPluginHostInterface * hostInterface) const = 0;
};

typedef osg::ref_ptr<SGIDataFieldBase> SGIDataFieldBasePtr;
typedef std::vector<SGIDataFieldBasePtr> SGIDataFieldBaseVector;

template<typename DATATYPE>
class SGIDataFieldT : public SGIDataFieldBase
{
public:
    SGIDataFieldT(const DATATYPE & value=DATATYPE())
        : SGIDataFieldBase(), _value(value) {}
    SGIDataFieldT(const SGIDataFieldT & rhs)
        : SGIDataFieldBase(rhs), _value(rhs._value) {}
    virtual ~SGIDataFieldT() {}

    const DATATYPE & value() const { return _value; }
    DATATYPE & value() { return _value; }

    bool operator<(const SGIDataFieldT & rhs) const { return _value < rhs._value; }
    bool operator==(const SGIDataFieldT & rhs) const { return _value == rhs._value; }
    bool operator!=(const SGIDataFieldT & rhs) const { return _value != rhs._value; }
    bool operator==(const DATATYPE & rhs) const { return _value == rhs; }
    bool operator!=(const DATATYPE & rhs) const { return _value != rhs; }

    virtual std::string toString(SGIPluginHostInterface * hostInterface) const;

protected:
    DATATYPE _value;
};

typedef SGIDataFieldT<std::string> SGIDataFieldString;
typedef SGIDataFieldT<int> SGIDataFieldInteger;
typedef SGIDataFieldT<unsigned> SGIDataFieldUnsigned;
typedef SGIDataFieldT<bool> SGIDataFieldBool;
typedef SGIDataFieldT<SGIItemBasePtr> SGIDataFieldItemBasePtr;

template<>
inline std::string SGIDataFieldT<std::string>::toString(SGIPluginHostInterface * hostInterface) const
{
    return _value;
}

template<>
inline std::string SGIDataFieldT<int>::toString(SGIPluginHostInterface * hostInterface) const
{
    std::stringstream ss;
    ss << _value;
    return ss.str();
}

template<>
inline std::string SGIDataFieldT<unsigned>::toString(SGIPluginHostInterface * hostInterface) const
{
    std::stringstream ss;
    ss << _value;
    return ss.str();
}

template<>
inline std::string SGIDataFieldT<bool>::toString(SGIPluginHostInterface * hostInterface) const
{
    std::stringstream ss;
    ss << ((_value)?"true":"false");
    return ss.str();
}

template<>
inline std::string SGIDataFieldT<SGIItemBasePtr>::toString(SGIPluginHostInterface * hostInterface) const
{
    std::string ret;
    if(!hostInterface->getObjectDisplayName(ret, _value.get(), true))
        ret = "N/A";
    return ret;
}

class SGIDataItemBase : public osg::Referenced
{
public:
    SGIDataItemBase(SGIItemBase * item=NULL, size_t numFields=0) : _item(item), _fields(numFields) {}
    SGIDataItemBase(const SGIDataItemBase & rhs)
        : osg::Referenced(rhs), _item(rhs._item), _fields(rhs._fields)
    {
    }
    virtual ~SGIDataItemBase() {}

    template<typename DATAITEMTYPE>
    DATAITEMTYPE * as()
    {
        return dynamic_cast<DATAITEMTYPE*>(this);
    }
    template<typename DATAITEMTYPE>
    const DATAITEMTYPE * as() const
    {
        return dynamic_cast<const DATAITEMTYPE*>(this);
    }

    size_t numFields() const { return _fields.size(); }
    SGIDataFieldBase * getField(unsigned n) const { return  (n < _fields.size()) ? (_fields[n]) : NULL; }
    void setField(unsigned n, SGIDataFieldBase * field) { _fields.resize(std::max(n, (unsigned)_fields.size())); _fields[n] = field; }
    void addField(SGIDataFieldBase * field) { unsigned idx = (unsigned)_fields.size(); _fields.resize(idx+1); _fields[idx] = field; }

protected:
    SGIItemBasePtr          _item;
    SGIDataFieldBaseVector  _fields;
};
typedef osg::ref_ptr<SGIDataItemBase> SGIDataItemBasePtr;
typedef std::vector<SGIDataItemBasePtr> SGIDataItemBasePtrVector;
typedef std::list<SGIDataItemBasePtr> SGIDataItemBasePtrList;

} // namespace sgi

