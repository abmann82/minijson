minijson
========

Minijson is a low dependeny, easy to use json parser and writer. Minijson works with all common compilers and is tested and used on the following platforms:

* Linux (gcc)
* OSX (clang)
* iOS
* Android
* Windows (visual c++)
* Windows Phone 8 (and newer)

Usage:

Download or clone the source files and add minijson.h and minijson.cpp to your project. 

Examples:

minijson parse example:

    #include "minijson.h"
    using namespace minijson;
    
    int main(int argc, char* argv[])
    {
        const char* JSON = " { \"test\":[\"a\", 1, false ] } ";
        
        CEntity* json = nullptr;
        try 
        {
            json = CParser::ParseString(JSON);
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
        delete json;
    }
