#include "minijson.h"
#include <string.h>
#include <sstream>
#include <cstdarg>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>


#ifndef _WIN32
#define MJSONvsprintf(str, size, format, args) vsnprintf(str, size, format, args)
#else // !_WIN32
#define MJSONvsprintf(str, size, format, args) vsprintf_s(str, size, format, args)
#endif // !_WIN32

namespace minijson {

std::string CEntity::s_EmptyString;

CException::CException(const char* txt, ...)
{
    char buf[16384];
    va_list list;
    va_start(list, txt);
    MJSONvsprintf(buf, 16384, txt, list);
    va_end(list);

    m_Message = std::string(buf);
}
CException::CException()
{
}
CException::~CException()
{
}

CParseErrorException::CParseErrorException(const char* data, int position, const char* txt, ...)
    : CException(),
      m_Position(position),
      m_Line(-1),
      m_Column(-1)
{
    char buf[16384];
    va_list list;
    va_start(list, txt);
    MJSONvsprintf(buf, 16384, txt, list);
    va_end(list);

    m_Message = std::string(buf);

    size_t dataLen = strlen(data);
    if (position >= 0 && data && dataLen > (size_t)position)
    {

        int currentStartOfLinePos = 0;
        int prevStartOfLinePos = -1;
        int prev2StartOfLinePos = -1;
        int line = 1;
        for (int i = 0; i < position; i++)
        {
            if (data[i] == '\n')
            {
                prev2StartOfLinePos = prevStartOfLinePos;
                prevStartOfLinePos = currentStartOfLinePos;
                currentStartOfLinePos = i + 1;
                line++;
            }
        }
        int endPosOfLine = position;
        for (int i = position; (size_t)i < dataLen; i++)
        {
            endPosOfLine = i;
            if (data[i] == '\n')
            {
                break;
            }
        }
        int nextLinesCount = 0;
        int next2LinesEndPos = endPosOfLine;
        for (int i = endPosOfLine + 1; (size_t)i < dataLen; i++)
        {
            next2LinesEndPos = i;
            if (data[i] == '\n')
            {
                nextLinesCount++;
                if (nextLinesCount >= 2)
                {
                    break;
                }
            }
        }

        m_Line = line;
        m_Column = (m_Position - currentStartOfLinePos + 1);

        std::string markerLine;
        if (m_Column > 0)
        {
            markerLine = std::string(m_Column - 1, ' ');
        }
        markerLine.push_back('^');
        markerLine.push_back('\n');

        int surroundingStart = prev2StartOfLinePos; // attempt to include 2 previous lines
        if (surroundingStart < 0)
        {
            surroundingStart = prevStartOfLinePos;
        }
        if (surroundingStart < 0)
        {
            surroundingStart = currentStartOfLinePos;
        }
        std::string prevAndCurrentText(data + surroundingStart, endPosOfLine - surroundingStart + 1);
        std::string nextText(data + endPosOfLine + 1, next2LinesEndPos - endPosOfLine);
        m_Surrounding = prevAndCurrentText + markerLine + nextText;
    }
}
CParseErrorException::~CParseErrorException()
{
}
CIOException::CIOException(const char* txt, ...)
: CException()
{
    char buf[16384];
    va_list list;
    va_start(list, txt);
    MJSONvsprintf(buf, 16384, txt, list);
    va_end(list);

    m_Message = std::string(buf);
}

static std::string EscapeString(const std::string& str)
{
    int escapeCount = 0;
    for (std::string::size_type i = 0; i < str.length(); i++)
    {
        char c = str[i];
        if (c == '\b' ||
            c == '\r' ||
            c == '\n' ||
            c == '\f' ||
            c == '\t' ||
            c == '\\' ||
            c == '/' ||
            c == '\"')
        {
            escapeCount++;
        }
    }
    std::string out;
    out.reserve(str.length() + escapeCount);
    for (std::string::size_type i = 0; i < str.length(); i++)
    {
        char c = str[i];
        switch (c)
        {
        case '\b': out += "\\b"; break;
        case '\r': out += "\\r"; break;
        case '\n': out += "\\n"; break;
        case '\f': out += "\\f"; break;
        case '\t': out += "\\t"; break;
        case '\\': out += "\\\\"; break;
        case '/': out += "\\/"; break;
        case '\"': out += "\\\""; break;
        default: out += c;
        }
    }
    return out;
}

CEntity::CEntity()
{
}
CEntity::~CEntity()
{
}
bool CEntity::IsObject() const
{
    return dynamic_cast<const CObject*>(this) != NULL;
}
bool CEntity::IsArray() const
{
    return dynamic_cast<const CArray*>(this) != NULL;
}
bool CEntity::IsString() const
{
    return dynamic_cast<const CString*>(this) != NULL;
}
bool CEntity::IsNumber() const
{
    return dynamic_cast<const CNumber*>(this) != NULL;
}
bool CEntity::IsBoolean() const
{
    return dynamic_cast<const CBoolean*>(this) != NULL;
}
bool CEntity::IsNull() const
{
    return dynamic_cast<const CNull*>(this) != NULL;
}
const CObject& CEntity::Object() const
{
    const CObject* obj = dynamic_cast<const CObject*>(this);
    if (!obj)
    {
        throw CException("Object() failed for non CObject entity");
    }
    return *obj;
}
CObject& CEntity::Object()
{
    CObject* obj = dynamic_cast<CObject*>(this);
    if (!obj)
    {
        throw CException("Object() failed for non CObject entity");
    }
    return *obj;
}

const CArray& CEntity::Array() const
{
    const CArray* arr = dynamic_cast<const CArray*>(this);
    if (!arr)
    {
        throw CException("Array() failed for non CArray entity");
    }
    return *arr;
}
CArray& CEntity::Array()
{
    CArray* arr = dynamic_cast<CArray*>(this);
    if (!arr)
    {
        throw CException("Array() failed for non CArray entity");
    }
    return *arr;
}
const CString& CEntity::String() const
{
    const CString* str = dynamic_cast<const CString*>(this);
    if (!str)
    {
        throw CException("String() failed for non CString entity");
    }
    return *str;
}
CString& CEntity::String()
{
    CString* str = dynamic_cast<CString*>(this);
    if (!str)
    {
        throw CException("String() faild for non CString entity");
    }
    return *str;
}
const CNumber& CEntity::Number() const
{
    const CNumber* number = dynamic_cast<const CNumber*>(this);
    if (!number)
    {
        throw CException("Number() failed for non CNumber entity");
    }
    return *number;
}
CNumber& CEntity::Number()
{
    CNumber* number = dynamic_cast<CNumber*>(this);
    if (!number)
    {
        throw CException("Number() failed for non CNumber entity");
    }
    return *number;
}

const CBoolean& CEntity::Boolean() const
{
    const CBoolean* boolean = dynamic_cast<const CBoolean*>(this);
    if (!boolean)
    {
        throw CException("Boolean() failed for non CBoolean entity");
    }
    return *boolean;
}
CBoolean& CEntity::Boolean()
{
    CBoolean* boolean = dynamic_cast<CBoolean*>(this);
    if (!boolean)
    {
        throw CException("Boolean() failed for non CBoolean entity");
    }
    return *boolean;
}
const CNull& CEntity::Null() const
{
    const CNull* null = dynamic_cast<const CNull*>(this);
    if (!null)
    {
        throw CException("Null() failed for non CNull entity");
    }
    return *null;
}
CNull& CEntity::Null()
{
    CNull* null = dynamic_cast<CNull*>(this);
    if (!null)
    {
        throw CException("Null() failed for non CNull entity");
    }
    return *null;
}
const std::string& CEntity::ObjectMemberNameByIndex(int index) const
{
    if (!IsObject())
    {
        throw CException("ObjectMemberNameByIndex() is only allowed for objects");
    }
    return Object().MemberNameByIndex(index);
}

int CEntity::Count() const
{
    throw CException("Count is not applicapable for this type");
}
const std::string& CEntity::StringValue() const
{
    if (!IsString())
    {
        throw CException("Called StringValue for non string entity");
    }
    return String().Value();
}
int CEntity::IntValue() const
{
    return atoi(Number().Value().c_str());
}
float CEntity::FloatValue() const
{
    return DoubleValue();
}
double CEntity::DoubleValue() const
{
    return atof(Number().Value().c_str());
}
bool CEntity::BoolValue() const
{
    return Boolean().Value();
}

const CEntity& CEntity::operator[] (int idx) const
{
    if (IsArray())
    {
        return Array().EntityAtIndex(idx);
    }
    else if (IsObject())
    {
        return Object().EntityAtIndex(idx);
    }
    else
    {
        throw CException("operator[](int) is only allowed for arrays and objects");
    }
}
const CEntity& CEntity::operator[] (const char* key) const
{
    if (!IsObject())
    {
        throw CException("operator[](key) is only allowed for objects");
    }
    return *Object().GetEntity(key);
}
const CEntity& CEntity::operator[] (const std::string& key) const
{
    if (!IsObject())
    {
        throw CException("operator[](key) is only allowed for objects");
    }
    return *Object().GetEntity(key);
}

CEntity& CEntity::operator[] (int idx)
{
    if (IsArray())
    {
        return Array().EntityAtIndex(idx);
    }
    else if (IsObject())
    {
        return Object().EntityAtIndex(idx);
    }
    else
    {
        throw CException("operator[](int) is only allowed for arrays and objects");
    }
}
CEntity& CEntity::operator[] (const char* key)
{
    if (!IsObject())
    {
        throw CException("operator[](key) is only allowed for objects");
    }
    return *Object().GetEntity(key);
}
CEntity& CEntity::operator[] (const std::string& key)
{
    if (!IsObject())
    {
        throw CException("operator[](key) is only allowed for objects");
    }
    return *Object().GetEntity(key);
}

CNumber::CNumber()
{
}
CNumber::~CNumber()
{
}

void CNumber::SetInt(int i)
{
    char buf[256];
#ifndef _WIN32
    snprintf(buf, 255, "%d", i);
#else // !_WIN32
    sprintf_s(buf, 255, "%d", i);
#endif // !_WIN32
    buf[255] = 0;
    m_Number = buf;
}
void CNumber::SetFloat(float f)
{
    char buf[256];
#ifndef _WIN32
    snprintf(buf, 255, "%f", f);
#else // !_WIN32
    sprintf_s(buf, 255, "%f", f);
#endif // !_WIN32
    buf[255] = 0;
    m_Number = buf;
}
void CNumber::SetDouble(double d)
{
    char buf[256];
#ifndef _WIN32
    snprintf(buf, 255, "%f", d);
#else // !_WIN32
    sprintf_s(buf, 255, "%f", d);
#endif // !_WIN32
    buf[255] = 0;
    m_Number = buf;
}
void CNumber::SetString(const std::string& num)
{
    m_Number = num;
}
int CNumber::ValueInt() const
{
    std::stringstream stream(m_Number);
    int v;
    stream >> v;
    if (stream.fail())
    {
        v = 0;
    }
    return v;
}
float CNumber::ValueFloat() const
{
    std::stringstream stream(m_Number);
    float v;
    stream >> v;
    if (stream.fail())
    {
        v = 0.0f;
    }
    return v;
}
double CNumber::ValueDouble() const
{
    std::stringstream stream(m_Number);
    double v;
    stream >> v;
    if (stream.fail())
    {
        v = 0.0;
    }
    return v;
}

std::string CNumber::ToString(bool prettyPrint, const std::string& indentation, int level) const
{
    return m_Number;
}

CEntity* CNumber::Copy() const
{
    CNumber* copy = new CNumber();
    copy->m_Number = m_Number;
    return copy;
}

CString::CString()
{
}
CString::~CString()
{
}
void CString::SetString(const char* str)
{
    m_Value = std::string(str);
}
void CString::SetString(const std::string& str)
{
    m_Value = str;
}
std::string CString::ToString(bool prettyPrint, const std::string& indentation, int level) const
{
    std::string s;
    s += "\"";
    s += EscapeString(m_Value);
    s += "\"";
    return s;
}
CEntity* CString::Copy() const
{
    CString* copy = new CString();
    copy->m_Value = m_Value;
    return copy;
}

CArray::CArray()
{
}
CArray::~CArray()
{
    for (int i = 0; i < (int)m_Values.size(); i++)
    {
        delete m_Values[i];
    }
}

void CArray::Remove(int index)
{
    if (index < 0 ||
        index >= (int)m_Values.size())
    {
        throw CException("index out of range");
    }
    CEntity* ent = m_Values[index];
    delete ent;
    m_Values.erase(m_Values.begin() + index);
}

CArray* CArray::AddArray()
{
    CArray* arr = new CArray();
    m_Values.push_back(arr);
    return arr;
}
CObject* CArray::AddObject()
{
    CObject* arr = new CObject();
    m_Values.push_back(arr);
    return arr;
}

CNumber* CArray::AddInt(int value)
{
    CNumber* num = new CNumber();
    num->SetInt(value);
    m_Values.push_back(num);
    return num;
}
CNumber* CArray::AddFloat(float value)
{
    CNumber* num = new CNumber();
    num->SetFloat(value);
    m_Values.push_back(num);
    return num;
}
CNumber* CArray::AddDouble(double value)
{
    CNumber* num = new CNumber();
    num->SetDouble(value);
    m_Values.push_back(num);
    return num;
}

CString* CArray::AddString(const char* str)
{
    CString* s = new CString();
    s->SetString(str);
    m_Values.push_back(s);
    return s;
}
CString* CArray::AddString(const std::string& str)
{
    CString* s = new CString();
    s->SetString(str);
    m_Values.push_back(s);
    return s;
}
CBoolean* CArray::AddBool(bool value)
{
    CBoolean* b = new CBoolean();
    b->SetBool(value);
    m_Values.push_back(b);
    return b;
}
CNull* CArray::AddNull()
{
    CNull* n = new CNull();
    m_Values.push_back(n);
    return n;
}
std::string CArray::ToString(bool prettyPrint, const std::string& indentation, int level) const
{
    std::string s;
    s += "[";
    for (int i = 0; i < (int)m_Values.size(); i++)
    {
        if (i != 0)
        {
            s += ",";
        }
        s += m_Values.at(i)->ToString(prettyPrint, indentation, level+1);
    }
    s += "]";
    return s;
}
const std::string& CArray::GetString(int index, const std::string& defaultValue) const
{
    if (index < 0 || index >= Count() || !m_Values[index] || !m_Values[index]->IsString())
    {
        return defaultValue;
    }
    return static_cast<CString*>(m_Values[index])->Value();
}
CNumber* CArray::GetNumber(int index) const
{
    if (index < 0 || index >= Count() || !m_Values[index] || !m_Values[index]->IsNumber())
    {
        return NULL;
    }
    return static_cast<CNumber*>(m_Values[index]);
}
int CArray::GetInt(int index, int defaultValue) const
{
    CNumber* number = GetNumber(index);
    if (!number)
    {
        return defaultValue;
    }
    return number->ValueInt();
}
float CArray::GetFloat(int index, float defaultValue) const
{
    CNumber* number = GetNumber(index);
    if (!number)
    {
        return defaultValue;
    }
    return number->ValueFloat();
}
double CArray::GetDouble(int index, double defaultValue) const
{
    CNumber* number = GetNumber(index);
    if (!number)
    {
        return defaultValue;
    }
    return number->ValueDouble();
}
CArray* CArray::GetArray(int index) const
{
    if (index < 0 || index >= Count())
    {
        return NULL;
    }
    CEntity* e = m_Values[index];
    if (e && e->IsArray())
    {
        return static_cast<CArray*>(e);
    }
    return NULL;
}
CObject* CArray::GetObject(int index) const
{
    if (index < 0 || index >= Count())
    {
        return NULL;
    }
    CEntity* e = m_Values[index];
    if (e && e->IsObject())
    {
        return static_cast<CObject*>(e);
    }
    return NULL;
}
CBoolean* CArray::GetBoolean(int index) const
{
    if (index < 0 || index >= Count())
    {
        return NULL;
    }
    CEntity* e = m_Values[index];
    if (e && e->IsBoolean())
    {
        return static_cast<CBoolean*>(e);
    }
    return NULL;
}
bool CArray::GetBool(int index, bool defaultValue) const
{
    CBoolean* b = GetBoolean(index);
    if (!b)
    {
        return defaultValue;
    }
    return b->Value();
}
CNull* CArray::GetNull(int index) const
{
    if (index < 0 || index >= Count())
    {
        return NULL;
    }
    CEntity* e = m_Values[index];
    if (e && e->IsNull())
    {
        return static_cast<CNull*>(e);
    }
    return NULL;
}
CEntity& CArray::EntityAtIndex(int index)
{
    return *m_Values[index];
}
const CEntity& CArray::EntityAtIndex(int index) const
{
    return *m_Values[index];
}

CEntity* CArray::Copy() const
{
    CArray* copy = new CArray();
    copy->m_Values.resize(m_Values.size());
    for (std::size_t i = 0; i < m_Values.size(); i++)
    {
        CEntity* e = m_Values[i]->Copy();
        copy->m_Values[i] = e;
    }
    return copy;
}


CObject::CObject()
{
}
CObject::~CObject()
{
    std::map<std::string, CEntity*>::iterator it;
    for (it = m_Values.begin(); it != m_Values.end(); ++it)
    {
        delete it->second;
    }
}
bool CObject::Contains(const char* name) const
{
    std::string s(name);
    return m_Values.find(s) != m_Values.end();
}
CArray* CObject::AddArray(const char* name)
{
    if (Contains(name))
    {
        return NULL;
    }
    CArray* arr = new CArray();
    m_Values[std::string(name)] = arr;
    m_MemberNameByIndex.push_back(std::string(name));
    return arr;
}
CObject* CObject::AddObject(const char* name)
{
    if (Contains(name))
    {
        return NULL;
    }
    CObject* obj = new CObject();
    m_Values[std::string(name)] = obj;
    m_MemberNameByIndex.push_back(std::string(name));
    return obj;
}
CNumber* CObject::AddNumber(const char* name)
{
    if (Contains(name))
    {
        return NULL;
    }
    CNumber* num = new CNumber();
    m_Values[std::string(name)] = num;
    m_MemberNameByIndex.push_back(std::string(name));
    return num;
}

CNumber* CObject::AddInt(const char* name, int i)
{
    CNumber* num = AddNumber(name);
    if (num)
    {
        num->SetInt(i);
    }
    return num;
}
CNumber* CObject::AddFloat(const char* name, float f)
{
    CNumber* num = AddNumber(name);
    if (num)
    {
        num->SetFloat(f);
    }
    return num;
}
CNumber* CObject::AddDouble(const char* name, double d)
{
    CNumber* num = AddNumber(name);
    if (num)
    {
        num->SetDouble(d);
    }
    return num;
}
CString* CObject::AddString(const char* name, const char* value)
{
    if (Contains(name))
    {
        return NULL;
    }
    CString* s = new CString();
    if (value != NULL)
    {
        s->SetString(value);
    }
    m_Values[std::string(name)] = s;
    m_MemberNameByIndex.push_back(std::string(name));
    return s;
}
CBoolean* CObject::AddBoolean(const char* name, bool b)
{
    if (Contains(name))
    {
        return NULL;
    }
    CBoolean* boolean = new CBoolean();
    boolean->SetBool(b);
    m_Values[std::string(name)] = boolean;
    m_MemberNameByIndex.push_back(std::string(name));
    return boolean;
}
CNull* CObject::AddNull(const char* name)
{
    if (Contains(name))
    {
        return NULL;
    }
    CNull* null = new CNull();
    m_Values[std::string(name)] = null;
    m_MemberNameByIndex.push_back(std::string(name));
    return null;
}
CNumber* CObject::SetInt(const char* name, int i)
{
    CEntity* ent = GetEntity(name);
    if (!ent)
    {
        return AddInt(name, i);
    }
    if (!ent->IsNumber())
    {
        Remove(name);
        return AddInt(name, i);
    }
    ent->Number().SetInt(i);
    return &ent->Number();
}
CNumber* CObject::SetFloat(const char* name, float f)
{
    CEntity* ent = GetEntity(name);
    if (!ent)
    {
        return AddFloat(name, f);
    }
    if (!ent->IsNumber())
    {
        Remove(name);
        return AddFloat(name, f);
    }
    ent->Number().SetFloat(f);
    return &ent->Number();
}
CNumber* CObject::SetDouble(const char* name, double d)
{
    CEntity* ent = GetEntity(name);
    if (!ent)
    {
        return AddDouble(name, d);
    }
    if (!ent->IsNumber())
    {
        Remove(name);
        return AddDouble(name, d);
    }
    ent->Number().SetDouble(d);
    return &ent->Number();
}
CString* CObject::SetString(const char* name, const char* value)
{
    CEntity* ent = GetEntity(name);
    if (!ent)
    {
        return AddString(name, value);
    }
    if (!ent->IsString())
    {
        Remove(name);
        return AddString(name, value);
    }
    ent->String().SetString(value);
    return &ent->String();
}

CBoolean* CObject::SetBoolean(const char* name, bool b)
{
    CEntity* ent = GetEntity(name);
    if (!ent)
    {
        return AddBoolean(name, b);
    }
    if (!ent->IsBoolean())
    {
        Remove(name);
        return AddBoolean(name, b);
    }
    ent->Boolean().SetBool(b);  
    return &ent->Boolean();
}
CEntity& CObject::EntityAtIndex(int idx)
{
    return *m_Values[m_MemberNameByIndex[idx]];
}
const CEntity& CObject::EntityAtIndex(int idx) const
{
    std::string key = m_MemberNameByIndex[idx];
    return *m_Values.find(key)->second;
}


std::string CObject::ToString(bool prettyPrint, const std::string& indentation, int level) const
{
    std::string indent;
    if (prettyPrint)
    {
        for (int i = 0; i < level; i++)
        {
            indent += indentation;
        }
    }

    std::string s;
    if (prettyPrint &&
        level > 0)
    {
        s += "\n";
    }
    s += indent + "{";
    if (prettyPrint)
    {
        s += "\n";
    }
    std::map<std::string, CEntity*>::const_iterator it;
    int i = 0;
    for (it = m_Values.begin(); it != m_Values.end(); ++it)
    {
        if (i != 0)
        {
            s += ",";
            if (prettyPrint)
            {
                s += "\n";
            }
        }
        s += indent + indentation + "\"";
        s += EscapeString(it->first);
        s += "\"";
        s += ":";
        s += it->second->ToString(prettyPrint, indentation, level+1);
        i++;
    }
    if (prettyPrint)
    {
        s += "\n";
    }
    s += indent + "}";
    return s;
}
const std::string& CObject::GetString(const std::string& name, const std::string& defaultValue) const
{
    std::map<std::string, CEntity*>::const_iterator it = m_Values.find(name);
    if (it == m_Values.end() || !it->second || !it->second->IsString())
    {
        return defaultValue;
    }
    return static_cast<CString*>(it->second)->Value();
}
CNumber* CObject::GetNumber(const std::string& name) const
{
    std::map<std::string, CEntity*>::const_iterator it = m_Values.find(name);
    if (it == m_Values.end() || !it->second || !it->second->IsNumber())
    {
        return NULL;
    }
    return static_cast<CNumber*>(it->second);
}
int CObject::GetInt(const std::string& name, int defaultValue) const
{
    CNumber* number = GetNumber(name);
    if (!number)
    {
        return defaultValue;
    }
    return number->ValueInt();
}
float CObject::GetFloat(const std::string& name, float defaultValue) const
{
    CNumber* number = GetNumber(name);
    if (!number)
    {
        return defaultValue;
    }
    return number->ValueFloat();
}
double CObject::GetDouble(const std::string& name, double defaultValue) const
{
    CNumber* number = GetNumber(name);
    if (!number)
    {
        return defaultValue;
    }
    return number->ValueDouble();
}
CArray* CObject::GetArray(const std::string& name) const
{
    std::map<std::string, CEntity*>::const_iterator it = m_Values.find(name);
    if (it == m_Values.end() || !it->second || !it->second->IsArray())
    {
        return NULL;
    }
    return static_cast<CArray*>(it->second);
}
CObject* CObject::GetObject(const std::string& name) const
{
    std::map<std::string, CEntity*>::const_iterator it = m_Values.find(name);
    if (it == m_Values.end() || !it->second || !it->second->IsObject())
    {
        return NULL;
    }
    return static_cast<CObject*>(it->second);
}
CBoolean* CObject::GetBoolean(const std::string& name) const
{
    std::map<std::string, CEntity*>::const_iterator it = m_Values.find(name);
    if (it == m_Values.end() || !it->second || !it->second->IsBoolean())
    {
        return NULL;
    }
    return static_cast<CBoolean*>(it->second);
}
bool CObject::GetBool(const std::string& name, bool defaultValue) const
{
    CBoolean* b = GetBoolean(name);
    if (!b)
    {
        return defaultValue;
    }
    return b->Value();
}
CNull* CObject::GetNull(const std::string& name) const
{
    std::map<std::string, CEntity*>::const_iterator it = m_Values.find(name);
    if (it == m_Values.end() || !it->second || !it->second->IsNull())
    {
        return NULL;
    }
    return static_cast<CNull*>(it->second);
}
CEntity* CObject::GetEntity(const std::string& name) const
{
    std::map<std::string, CEntity*>::const_iterator it = m_Values.find(name);
    if (it == m_Values.end() || !it->second)
    {
        return NULL;
    }
    return it->second;
}
bool CObject::Remove(const char* name)
{
    std::string s(name);
    std::map<std::string, CEntity*>::iterator it = m_Values.find(s);
    if (it == m_Values.end())
    {
        return false;
    }
    CEntity* ent = it->second;
    m_Values.erase(it);

    std::vector<std::string>::iterator it2 = std::find(m_MemberNameByIndex.begin(), m_MemberNameByIndex.end(), s);
    m_MemberNameByIndex.erase(it2);
    delete ent;
    return true;
}
CEntity* CObject::Copy() const
{
    CObject* copy = new CObject();
    for (std::map<std::string, CEntity*>::const_iterator it = m_Values.begin(); it != m_Values.end(); ++it)
    {
        CEntity* e = it->second->Copy();
        copy->m_Values[it->first] = e;
    }
    copy->m_MemberNameByIndex = m_MemberNameByIndex;
    return copy;
}
void CObject::MergeFrom(const CObject& obj, bool overwrite)
{
    for (std::map<std::string, CEntity*>::const_iterator it = obj.m_Values.begin(); it != obj.m_Values.end(); ++it)
    {
        const std::string& key = it->first;
        if (Contains(key.c_str()))
        {
            if (!overwrite)
            {
                continue;
            }
            delete m_Values[key];
        }
        else
        {
            m_MemberNameByIndex.push_back(key);
        }
        m_Values[key] = it->second->Copy();
    }
}


CBoolean::CBoolean()
    : m_Value(false)
{
}
CBoolean::~CBoolean()
{
}
void CBoolean::SetBool(bool b)
{
    m_Value = b;
}
std::string CBoolean::ToString(bool prettyPrint, const std::string& indentation, int level) const
{
    return m_Value ? std::string("true") : std::string("false");
}
CEntity* CBoolean::Copy() const
{
    CBoolean* copy = new CBoolean();
    copy->m_Value = m_Value;
    return copy;
}

CNull::CNull()
{
}
CNull::~CNull()
{
}
std::string CNull::ToString(bool prettyPrint, const std::string& indentation, int level) const
{
    return std::string("null");
}
CEntity* CNull::Copy() const
{
    return new CNull();
}


CParser::CParser()
{
}
CParser::~CParser()
{
}
void CParser::SkipWhitespaces()
{
    while ( m_Position < m_Length &&
           (m_Text[m_Position] == ' ' ||
            m_Text[m_Position] == '\t' ||
            m_Text[m_Position] == '\r' ||
            m_Text[m_Position] == '\n'))
    {
        m_Position++;
    }
}
bool CParser::TryToConsume(const char* txt)
{
    int storedPos = m_Position;
    int i = 0;
    bool found = false;
    while (m_Position < m_Length)
    {
        char c = m_Text[m_Position];
        if (c != txt[i])
        {
            break;
        }
        m_Position++;
        i++;
        if (txt[i] == 0)
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        m_Position = storedPos;
    }
    return found;
}
void CParser::ConsumeOrDie(const char* txt)
{
    int origPos = m_Position;
    bool b = TryToConsume(txt);
    if (!b)
    {
        throw CParseErrorException(m_Text, origPos, "Syntax error: Expected '%s' at or after position %d", txt, origPos);
    }
}

static int WriteUTF8Chars(char* buf, short unsigned int c)
{
    if (c < 128)
    {
        buf[0] = c;
        return 1;
    }
    else if (c < 2048)
    {
        buf[0] = 0xc0 | ((c >> 6) & 0xff);
        buf[1] = 0x80 | (c & 63);
        return 2;
    }
    else
    {
        buf[0] = 0xe0 | ((c >> 12) & 0xff);
        buf[1] = 0x80 | ((c >> 6)  & 63);
        buf[2] = 0x80 | (c & 63);
        return 3;
    }
}

// NOTE: does NOT support empty strings, caller needs to check that!
std::string CParser::ParseStringLiteral()
{
    std::string str;
    str.reserve(1024);

    TryToConsume("\""); // NOTE: required because caller *may* have consumed this already, but does not have to. NOTE that due to this, we cannot support empty strings (this call would consume the closing \")
    int origPos = m_Position;
    char c = m_Text[m_Position];
    while (c != '\"')
    {
        if (m_Position == m_Length)
        {
            throw CParseErrorException(m_Text, origPos, "Closing \" not found");
        }
        if (c == '\\' &&
            m_Position+1 < m_Length)
        {
            m_Position++;
            c = m_Text[m_Position];
            switch (c)
            {
            case 'b': c = '\b'; break;
            case 'r': c = '\r'; break;
            case 'n': c = '\n'; break;
            case 'f': c = '\f'; break;
            case 't': c = '\t'; break;
            case '\\': c = '\\'; break;
            case '/': c = '/'; break;
            case '\"': c = '\"'; break;
            case 'u':
                {
                    m_Position++;
                    if (m_Position + 4 > m_Length)
                    {
                        throw CParseErrorException(m_Text, origPos, "Invalid \\u escaping");
                    }
                    char buf[5];
                    memcpy(buf, &m_Text[m_Position], 4);
                    buf[4] = 0;
                    int utf8Char = (int)strtol(buf, NULL, 16);
                    char utf8Buf[16];
                    int len = WriteUTF8Chars(utf8Buf, utf8Char);
                    for (int i = 0; i < len-1; i++)
                    {
                        str += utf8Buf[i];
                    }
                    c = utf8Buf[len-1];
                    m_Position += 3;
                }
                break;
            }
        }
        str += c;
        m_Position++;
        if (m_Position == m_Length)
        {
            throw CParseErrorException(m_Text, origPos, "Closing \" not found");
        }
        c = m_Text[m_Position];
    }
    m_Position++;
    return str;
}
CEntity* CParser::ParseValue()
{
    CEntity* data = NULL;
    if (TryToConsume("\""))
    {
        if (TryToConsume("\""))
        {
            // special case: empty string
            CString* s = new CString();
            s->SetString(std::string());
            data = s;
        }
        else
        {
            data = ParseString();
        }
    }
    else if (TryToConsume("["))
    {
        data = ParseArray();
    }
    else if (TryToConsume("{"))
    {
        data = ParseObject();
    }
    else if (TryToConsume("true"))
    {
        CBoolean* b = new CBoolean();
        b->SetBool(true);
        data = b;
    }
    else if (TryToConsume("false"))
    {
        CBoolean* b = new CBoolean();
        b->SetBool(false);
        data = b;
    }
    else if (TryToConsume("null"))
    {
        data = new CNull();
    }
    else
    {
        data = ParseNumber();
    }
    return data;
}

CArray* CParser::ParseArray()
{
    CArray* arr = new CArray();
    while (1)
    {
        SkipWhitespaces();
        if (TryToConsume("]"))
        {
            break;
        }
        CEntity* ent = ParseValue();
        arr->m_Values.push_back(ent);

        SkipWhitespaces();
        if (!TryToConsume(","))
        {
            ConsumeOrDie("]");
            break;
        }
    }
    return arr;
}
CObject* CParser::ParseObject()
{
    CObject* obj = new CObject();
    while (1)
    {
        SkipWhitespaces();
        if (TryToConsume("}"))
        {
            break;
        }

        std::string key = ParseStringLiteral();
        SkipWhitespaces();
        ConsumeOrDie(":");
        SkipWhitespaces();
        CEntity* ent = ParseValue();

        obj->m_Values[key] = ent;
        obj->m_MemberNameByIndex.push_back(key);
        
        SkipWhitespaces();
        if (!TryToConsume(","))
        {
            ConsumeOrDie("}");
            break;
        }
    }
    return obj;
}
CNumber* CParser::ParseNumber()
{
    CNumber* num = new CNumber();
    std::string str;
    str.reserve(32);
    while (m_Position < m_Length)
    {
        char c = m_Text[m_Position];
        if ((c < '0' || c > '9') && c != '.' && (c != '-' || !str.empty()))
        {
            break;
        }
        str += c;
        m_Position++;
    }
    num->m_Number = str;
    return num;
}


// NOTE: does NOT support empty strings, caller needs to check that!
CString* CParser::ParseString()
{
    std::string str = ParseStringLiteral();
    CString* s = new CString();
    s->SetString(str);
    return s;
}

CEntity* CParser::Parse(const char* txt, int length)
{
    m_Text = txt;
    m_Position = 0;
    if (length < 0)
    {
        m_Length = (int)strlen(txt);
    }
    else
    {
        m_Length = length;
    }
    CEntity* root = NULL;
    while (m_Position < m_Length)
    {
        SkipWhitespaces();
        if (m_Position == m_Length)
        {
            break;
        }
        if (TryToConsume("["))
        {
            root = ParseArray();
        }
        else if (TryToConsume("{"))
        {
            root = ParseObject();
        }
        else
        {
            throw CParseErrorException(m_Text, m_Position, "Syntax error");
        }
    }
    return root;
}
CEntity* CParser::ParseFromFile(const char* path)
{
    struct SFileCloser
    {
        FILE* m_File;
        SFileCloser(FILE* f)
        {
            m_File = f;
        }
        ~SFileCloser()
        {
            fclose(m_File);
        }
    };

    FILE* f = fopen(path, "rb");
    if (!f)
    {
        throw CIOException("Failed to open file %s", path);
    }

    SFileCloser file(f); // close the file when leaving this method

    fseek(f, 0, SEEK_END);
    int size = (int)ftell(f);
    fseek(f, 0, SEEK_SET);
    std::unique_ptr<char> buf((char*)malloc(size));
    int rd = (int)fread(buf.get(), 1, size, f);
    if (rd != size)
    {
        throw CIOException("Failed to read %d bytes from file (read=%d)", size, rd);
    }
    return ParseString(buf.get(), (int)size);
}
CEntity* CParser::ParseFromFile(const std::string& path)
{
    return CParser::ParseFromFile(path.c_str());
}
CWriter::CWriter(bool prettyPrint, const std::string& indentation, int level)
: m_PrettyPrint(true),
  m_Indentation(indentation),
  m_Level(level)
{
}
void CWriter::WriteToFile(const char* path, const CEntity& ent)
{
    std::string json = ent.ToString(m_PrettyPrint, m_Indentation, m_Level);
    FILE* f = fopen(path, "wb");
    if (!f)
    {
        throw CIOException("Failed to open file for writing");
    }
    size_t wr = fwrite(json.c_str(), 1, json.length(), f);
    fclose(f);
    if (wr != json.length())
    {
        throw CIOException("Failed to write all bytes to file");
    }
}
void CWriter::WriteToFile(const std::string& path, const CEntity& ent)
{
    WriteToFile(path.c_str(), ent);
}
} // minijson
