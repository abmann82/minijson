minijson
========

Low-dependency JSON parser for C++

minijson parse example:

using namespace minijson;

#include "minijson.h"
using namespace minijson;

int main(int argc, char* argv[])
{
    const char* JSON = " { \"test\":[\"a\", 1, false ] } ";
    try 
    {
        CEntity* json = CParser::ParseString(JSON);
        minijson::CObject& obj = json->Object();
        std::string str = obj["test"][0].StringValue();
        int i = obj["test"][1].IntValue();
        bool b = obj["test"][2].BoolValue();
    }
    catch (CException e)
    {
        printf("Json error: %s\n", e.Message().c_str());
        exit(1);
    }
}
