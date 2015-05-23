#include <string>
#include <map>
#include <vector>

#ifdef _WIN32
#ifndef __attribute__
#define __attribute__(x) /* nothing */
#endif
#endif // _WIN32
#if __cplusplus > 199711L
#define MINIJSON_OVERRIDE override
#else
#define MINIJSON_OVERRIDE
#endif

#ifdef _WIN32
#ifdef GetObject
#undef GetObject
#endif
#endif // _WIN32

namespace minijson {

class CEntity;
class CObject;
class CArray;
class CNumber;
class CString;
class CBoolean;
class CNull;

class CException
{
public:
    CException(const char* txt, ...) __attribute__((format(printf, 2, 3)));
    virtual ~CException();

    const std::string& Message() const { return m_Message; }

protected:
    CException();
protected:
    std::string m_Message;
};

class CParseErrorException : public CException
{
public:
    CParseErrorException(const char* data, int position, const char* txt, ...) __attribute__((format(printf, 4, 5)));
    virtual ~CParseErrorException();

    int Position() const { return m_Position; }
    int Line() const { return m_Line; }
    int Column() const { return m_Column; }
    const std::string& Surrounding() const { return m_Surrounding; }

private:
    int m_Position;
    int m_Line;
    int m_Column;
    std::string m_Surrounding; // if possible: 2 lines before error ; error line ; 'marker' line ; 2 lines after error
};

class CEntity
{
public:

    CEntity();
    virtual ~CEntity();

    const CObject& Object() const;
    CObject& Object();
    const CArray& Array() const;
    CArray& Array();
    const CNumber& Number() const;
    const CString& String() const;
    const CBoolean& Boolean() const;
    const CNull& Null() const;

    bool IsObject() const;
    bool IsArray() const;
    bool IsString() const;
    bool IsNumber() const;
    bool IsBoolean() const;
    bool IsNull() const;


    virtual int Count() const;
    const std::string& StringValue() const;
    float FloatValue() const;
    double DoubleValue() const;
    int IntValue() const;
    bool BoolValue() const;


    virtual bool Contains(const char* name) const { (void)name; return false; }
    const std::string& ObjectMemberNameByIndex(int index) const;


    const CEntity& operator[] (int idx) const;
    const CEntity& operator[] (const char* key) const;
    const CEntity& operator[] (const std::string& key) const;

    CEntity& operator[] (int idx);
    CEntity& operator[] (const char* key);
    CEntity& operator[] (const std::string& key);

    virtual std::string ToString(bool prettyPrint = true, const std::string& indentation = std::string("  "), int level = 0) const = 0;
    virtual CEntity* Copy() const = 0;
protected:
    static std::string s_EmptyString;

};

class CObject : public CEntity
{
public:
    CObject();
    virtual ~CObject();

    virtual bool Contains(const char* name) const MINIJSON_OVERRIDE;
    bool Remove(const char* name);


    CArray* AddArray(const char* name);
    CObject* AddObject(const char* name);
    CNumber* AddNumber(const char* name);
    CNumber* AddInt(const char* name, int i);
    CNumber* AddFloat(const char* name, float f);
    CNumber* AddDouble(const char* name, double d);
    CString* AddString(const char* name, const char* value = NULL);
    CBoolean* AddBoolean(const char* name, bool b = false);
    CNull* AddNull(const char* name);

    const std::string& GetString(const char* name, const std::string& defaultValue = s_EmptyString) const { return GetString(std::string(name), defaultValue); }
    const std::string& GetString(const std::string& name, const std::string& defaultValue = s_EmptyString) const;
    CNumber* GetNumber(const char* name) const { return GetNumber(std::string(name)); }
    CNumber* GetNumber(const std::string& name) const;
    int GetInt(const char* name, int defaultValue = 0) const { return GetInt(std::string(name), defaultValue); }
    int GetInt(const std::string& name, int defaultValue = 0) const;
    float GetFloat(const char* name, float defaultValue = 0.0f) const { return GetFloat(std::string(name), defaultValue); }
    float GetFloat(const std::string& name, float defaultValue = 0.0f) const;
    double GetDouble(const char* name, double defaultValue = 0.0f) const { return GetDouble(std::string(name), defaultValue); }
    double GetDouble(const std::string& name, double defaultValue = 0.0f) const;
    CArray* GetArray(const char* name) const { return GetArray(std::string(name)); }
    CArray* GetArray(const std::string& name) const;
    CObject* GetObject(const char* name) const { return GetObject(std::string(name)); }
    CObject* GetObject(const std::string& name) const;
    CBoolean* GetBoolean(const char* name) const { return GetBoolean(std::string(name)); }
    CBoolean* GetBoolean(const std::string& name) const;
    bool GetBool(const std::string& name, bool defaultValue = false) const;
    CNull* GetNull(const std::string& name) const;
    CEntity* GetEntity(const std::string& name) const;

    const std::string& MemberNameByIndex(int index) const { return m_MemberNameByIndex[index]; }

    virtual int Count() const MINIJSON_OVERRIDE { return (int)m_Values.size(); }
    CEntity& EntityAtIndex(int idx);
    const CEntity& EntityAtIndex(int idx) const;

    virtual std::string ToString(bool prettyPrint = true, const std::string& indentation = std::string("  "), int level = 0) const MINIJSON_OVERRIDE;
    virtual CEntity* Copy() const MINIJSON_OVERRIDE;
    void MergeFrom(const CObject& obj, bool overwrite);

private:
    std::map<std::string, CEntity*> m_Values;
    std::vector<std::string> m_MemberNameByIndex;
    friend class CParser;

};

class CArray : public CEntity
{
public:
    CArray();
    virtual ~CArray();

    CArray* AddArray();
    CObject* AddObject();
    CNumber* AddInt(int value);
    CNumber* AddFloat(float value);
    CNumber* AddDouble(double value);
    CString* AddString(const char* str);
    CString* AddString(const std::string& str);
    CBoolean* AddBool(bool value);
    CNull* AddNull();

    const std::string& GetString(int index, const std::string& defaultValue = s_EmptyString) const;
    CNumber* GetNumber(int index) const;
    int GetInt(int index, int defaultValue = 0) const;
    float GetFloat(int index, float defaultValue = 0.0f) const;
    double GetDouble(int index, double defaultValue = 0.0f) const;
    CArray* GetArray(int index) const;
    CObject* GetObject(int index) const;
    CBoolean* GetBoolean(int index) const;
    bool GetBool(int index, bool defaultValue = false) const;
    CNull* GetNull(int index) const;

    virtual std::string ToString(bool prettyPrint = true, const std::string& indentation = std::string("  "), int level = 0) const MINIJSON_OVERRIDE;
    virtual CEntity* Copy() const MINIJSON_OVERRIDE;

    virtual int Count() const MINIJSON_OVERRIDE{ return (int)m_Values.size(); }
    CEntity& EntityAtIndex(int index);
    const CEntity& EntityAtIndex(int index) const;
private:
    std::vector<CEntity*> m_Values;
    friend class CParser;
};

class CString : public CEntity
{
public:
    CString();
    virtual ~CString();

    void SetString(const char* str);
    void SetString(const std::string& str);

    virtual std::string ToString(bool prettyPrint = true, const std::string& indentation = std::string("  "), int level = 0) const MINIJSON_OVERRIDE;
    virtual CEntity* Copy() const MINIJSON_OVERRIDE;

    const std::string& Value() const { return m_Value; }

private:
    std::string m_Value;
    friend class CParser;
};

class CNumber : public CEntity
{
public:
    CNumber();
    virtual ~CNumber();

    void SetInt(int i);
    void SetFloat(float f);
    void SetDouble(double d);
    void SetString(const std::string& num);

    virtual std::string ToString(bool prettyPrint = true, const std::string& indentation = std::string("  "), int level = 0) const MINIJSON_OVERRIDE;
    virtual CEntity* Copy() const MINIJSON_OVERRIDE;

    const std::string& Value() const { return m_Number; }
    int ValueInt() const;
    float ValueFloat() const;
    double ValueDouble() const;
private:
    std::string m_Number;
    friend class CParser;
};

class CBoolean : public CEntity
{
public:
    CBoolean();
    virtual ~CBoolean();

    void SetBool(bool b);

    virtual std::string ToString(bool prettyPrint = true, const std::string& indentation = std::string("  "), int level = 0) const MINIJSON_OVERRIDE;
    virtual CEntity* Copy() const MINIJSON_OVERRIDE;

    bool Value() const { return m_Value; }
private:
    bool m_Value;
    friend class CParser;
};

class CNull : public CEntity
{
public:
    CNull();
    virtual ~CNull();

    virtual std::string ToString(bool prettyPrint = true, const std::string& indentation = std::string("  "), int level = 0) const MINIJSON_OVERRIDE;
    virtual CEntity* Copy() const MINIJSON_OVERRIDE;

private:
    friend class CParser;
};

class CParser
{
public:
    CParser();
    virtual ~CParser();

    CEntity* Parse(const char* txt, int length = -1);
    CEntity* Parse(const std::string& txt) { return Parse(txt.c_str(), (int) txt.size()); }

    // static convenience functions
    static CEntity* ParseString(const char* txt, int length = -1)
    {
        CParser p;
        return p.Parse(txt, length);
    }
    static CEntity* ParseString(const std::string& txt)
    {
        CParser p;
        return p.Parse(txt);
    }
private:
    void SkipWhitespaces();
    bool TryToConsume(const char* txt);
    void ConsumeOrDie(const char* txt);
    std::string ParseStringLiteral();
    CEntity* ParseValue();
    CArray* ParseArray();
    CObject* ParseObject();
    CNumber* ParseNumber();
    CString* ParseString();

    int m_Position;
    int m_Length;
    const char* m_Text;
};


} // minijson
