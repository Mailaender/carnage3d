#include "stdafx.h"
#include "ConsoleVar.h"

//////////////////////////////////////////////////////////////////////////

Cvar::Cvar(const std::string& cvarName, const std::string& description, CvarFlags cvarFlags)
    : mName(cvarName)
    , mCvarFlags(cvarFlags)
    , mDescription(description)
{
}

Cvar::~Cvar()
{
}

bool Cvar::SetFromString(const std::string& input, eCvarSetMethod setMethod)
{
    if (setMethod == eCvarSetMethod_Console)
    {
        debug_assert(!IsHidden());
    }
    // check rom access
    if (IsReadonly() && (setMethod != eCvarSetMethod_Config))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot change '%s', it is readonly", mName.c_str());
        return false;
    }
    // check init only access
    if (IsInit() && (setMethod == eCvarSetMethod_Console))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot change '%s', it is write protected", mName.c_str());
        return false;
    }
    // check cheat
    if (IsCheat() && (setMethod != eCvarSetMethod_CommandLine))
    {
        // todo
    }

    bool isChanged = false;
    bool isSuccess = DeserializeValue(input, isChanged);
    if (isSuccess && isChanged)
    {
        SetModified();

        if (IsRequiresAppRestart() && (setMethod == eCvarSetMethod_Console))
        {
            gConsole.LogMessage(eLogMessage_Info, "New value of '%s' will be applied after app restart", mName.c_str());
        }

        if (IsRequiresMapRestart() && (setMethod == eCvarSetMethod_Console))
        {
            gConsole.LogMessage(eLogMessage_Info, "New value of '%s' will be applied after map restart", mName.c_str()); 
        }
    }
    return isSuccess;
}

void Cvar::SaveCvar(cxx::json_document_node rootNode) const
{
    std::string printableValue;
    GetPrintableValue(printableValue);

    cxx::json_document_node node = rootNode.create_string_node(mName, printableValue);
    debug_assert(node);
}

bool Cvar::LoadCvar(cxx::json_document_node rootNode)
{
    std::string printableValue;
    if (!cxx::json_get_attribute(rootNode, mName, printableValue))
        return false;

    return SetFromString(printableValue, eCvarSetMethod_Config);
}

//////////////////////////////////////////////////////////////////////////

CvarBoolean::CvarBoolean(const std::string& cvarName, bool cvarValue, const std::string& description, CvarFlags cvarFlags) 
    : Cvar(cvarName, description, cvarFlags | CvarFlags_CvarBool)
    , mValue(cvarValue)
    , mDefaultValue(cvarValue)
{
}

void CvarBoolean::GetPrintableValue(std::string& output) const
{
    output = mValue ? "true" : "false";
}

void CvarBoolean::GetPrintableDefaultValue(std::string& output) const
{
    output = mDefaultValue ? "true" : "false";
}

bool CvarBoolean::DeserializeValue(const std::string& input, bool& valueChanged)
{
    bool prevValue = mValue;
    if (input == "true" || input == "1")
    {
        mValue = true;
    }
    else if (input == "false" || input == "0")
    {
        mValue = false;
    }
    else 
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot parse bool value");
        return false;
    }
    valueChanged = (prevValue != mValue);
    return true;
}

//////////////////////////////////////////////////////////////////////////

CvarString::CvarString(const std::string& cvarName, const std::string& cvarValue, const std::string& description, CvarFlags cvarFlags) 
    : Cvar(cvarName, description, cvarFlags | CvarFlags_CvarString)
    , mValue(cvarValue)
    , mDefaultValue(cvarValue)
{
}

void CvarString::GetPrintableValue(std::string& output) const
{
    output = mValue;
}

void CvarString::GetPrintableDefaultValue(std::string& output) const
{
    output = mDefaultValue;
}

bool CvarString::DeserializeValue(const std::string& input, bool& valueChanged)
{
    valueChanged = (input != mValue);
    if (valueChanged)
    {
        mValue = input;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////

CvarInt::CvarInt(const std::string& cvarName, int cvarValue, int cvarMin, int cvarMax, const std::string& description, CvarFlags cvarFlags)
    : Cvar(cvarName, description, cvarFlags | CvarFlags_CvarInt)
    , mValue(cvarValue)
    , mDefaultValue(cvarValue)
    , mMinValue(cvarMin)
    , mMaxValue(cvarMax)
{
}

CvarInt::CvarInt(const std::string& cvarName, int cvarValue, const std::string& description, CvarFlags cvarFlags)
    : Cvar(cvarName, description, cvarFlags | CvarFlags_CvarInt)
    , mValue(cvarValue)
    , mDefaultValue(cvarValue)
    , mMinValue(std::numeric_limits<int>::min())
    , mMaxValue(std::numeric_limits<int>::max())
{
}

void CvarInt::GetPrintableValue(std::string& output) const
{
    output = cxx::va("%d", mValue);
}

void CvarInt::GetPrintableDefaultValue(std::string& output) const
{
    output = cxx::va("%d", mDefaultValue);
}

bool CvarInt::DeserializeValue(const std::string& input, bool& valueChanged)
{
    int prevValue = mValue;
    int newValue = 0;
    if (::sscanf(input.c_str(), "%d", &newValue) < 1)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot parse integer value");
        return false;
    }
    mValue = glm::clamp(newValue, mMinValue, mMaxValue);
    if (mValue != newValue)
    {
        gConsole.LogMessage(eLogMessage_Debug, "Integer value %d is not within range [%d..%d]", newValue, mMinValue, mMaxValue);
    }
    valueChanged = (mValue != newValue);
    return true;
}

//////////////////////////////////////////////////////////////////////////

CvarFloat::CvarFloat(const std::string& cvarName, float cvarValue, float cvarMin, float cvarMax, const std::string& description, CvarFlags cvarFlags)
    : Cvar(cvarName, description, cvarFlags | CvarFlags_CvarFloat)
    , mValue(cvarValue)
    , mDefaultValue(cvarValue)
    , mMinValue(cvarMin)
    , mMaxValue(cvarMax)
{
}

CvarFloat::CvarFloat(const std::string& cvarName, float cvarValue, const std::string& description, CvarFlags cvarFlags)
    : Cvar(cvarName, description, cvarFlags | CvarFlags_CvarFloat)
    , mValue(cvarValue)
    , mDefaultValue(cvarValue)
    , mMinValue(std::numeric_limits<float>::min())
    , mMaxValue(std::numeric_limits<float>::max())
{
}

void CvarFloat::GetPrintableValue(std::string& output) const
{
    output = cxx::va("%.3f", mValue);
}

void CvarFloat::GetPrintableDefaultValue(std::string& output) const
{
    output = cxx::va("%.3f", mDefaultValue);
}

bool CvarFloat::DeserializeValue(const std::string& input, bool& valueChanged)
{
    float prevValue = mValue;
    float newValue = 0;
    if (::sscanf(input.c_str(), "%f", &newValue) == 0)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot parse float value");
        return false;
    }
    mValue = glm::clamp(newValue, mMinValue, mMaxValue);
    if (mValue != newValue)
    {
        gConsole.LogMessage(eLogMessage_Debug, "Float value %.3f is not within range [%.3f..%.3f]", newValue, mMinValue, mMaxValue);
    }
    valueChanged = (mValue != newValue);
    return true;
}

//////////////////////////////////////////////////////////////////////////

CvarColor::CvarColor(const std::string& cvarName, Color32 cvarValue, const std::string& description, CvarFlags cvarFlags)
    : Cvar(cvarName, description, cvarFlags | CvarFlags_CvarColor)
    , mValue(cvarValue)
    , mDefaultValue(cvarValue)
{
}

void CvarColor::GetPrintableValue(std::string& output) const
{
    output = cxx::va("%u %u %u %u", mValue.mR, mValue.mG, mValue.mB, mValue.mA);
}

void CvarColor::GetPrintableDefaultValue(std::string& output) const
{
    output = cxx::va("%u %u %u %u", 
        mDefaultValue.mR, mDefaultValue.mG, 
        mDefaultValue.mB, mDefaultValue.mA);
}

bool CvarColor::DeserializeValue(const std::string& input, bool& valueChanged)
{
    int colorR;
    int colorG;
    int colorB;
    int colorA;

    if (::sscanf(input.c_str(), "%d %d %d %d", &colorR, &colorG, &colorB, &colorA) < 4)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot parse color32 value");
        return false;
    }

    Color32 prevColor = mValue;
    mValue.SetComponents(colorR, colorG, colorB, colorA);

    valueChanged = (prevColor != mValue);
    return true;
}

//////////////////////////////////////////////////////////////////////////

CvarPoint::CvarPoint(const std::string& cvarName, const Point& cvarValue, const std::string& description, CvarFlags cvarFlags)
    : Cvar(cvarName, description, cvarFlags | CvarFlags_CvarPoint)
    , mValue(cvarValue)
    , mDefaultValue(cvarValue)
{
}

void CvarPoint::GetPrintableValue(std::string& output) const
{
    output = cxx::va("%d %d", mValue.x, mValue.y);
}

void CvarPoint::GetPrintableDefaultValue(std::string& output) const
{
    output = cxx::va("%d %d", mDefaultValue.x, mDefaultValue.y);
}

bool CvarPoint::DeserializeValue(const std::string& input, bool& valueChanged)
{
    Point prevValue = mValue;
    Point newValue;
    if (::sscanf(input.c_str(), "%d %d", &newValue.x, &newValue.y) < 2)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot parse point2 value");
        return false;
    }

    valueChanged = (prevValue != newValue);
    if (valueChanged)
    {
        mValue = newValue;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////

CvarVec3::CvarVec3(const std::string& cvarName, const glm::vec3& cvarValue, const std::string& description, CvarFlags cvarFlags)
    : Cvar(cvarName, description, cvarFlags | CvarFlags_CvarVec3)
    , mValue(cvarValue)
    , mDefaultValue(cvarValue)
{
}

void CvarVec3::GetPrintableValue(std::string& output) const
{
    output = cxx::va("%.3f %.3f %.3f", mValue.x, mValue.y, mValue.z);
}

void CvarVec3::GetPrintableDefaultValue(std::string& output) const
{
    output = cxx::va("%.3f %.3f %.3f", mDefaultValue.x, mDefaultValue.y, mDefaultValue.z);
}

bool CvarVec3::DeserializeValue(const std::string& input, bool& valueChanged)
{
    glm::vec3 prevValue = mValue;
    glm::vec3 newValue;
    if (::sscanf(input.c_str(), "%f %f %f", &newValue.x, &newValue.y, &newValue.z) < 3)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot parse vec3 value");
        return false;
    }
    valueChanged = (prevValue != newValue);
    if (valueChanged)
    {
        mValue = newValue;
    }
    return true;
}