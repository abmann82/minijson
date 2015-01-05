#include <minijson.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool Validate(const char* fileName);


static bool Validate(const char* fileName)
{
    FILE* f = fopen(fileName, "r");
    if (!f)
    {
        fprintf(stderr, "ERROR: Failed to open file %s for reading\n", fileName);
        fflush(stderr);
        return false;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0)
    {
        fprintf(stderr, "ERROR: Empty file %s\n", fileName);
        fflush(stderr);
        fclose(f);
        return false;
    }
    if (size > 1024 * 1024 * 20)
    {
        fprintf(stderr, "ERROR: File too large: %s\n", fileName);
        fflush(stderr);
        fclose(f);
        return false;
    }
    char* data = (char*)malloc(size + 1);
    if (fread(data, 1, size, f) != (size_t)size)
    {
        fprintf(stderr, "ERROR: Failed to read %d bytes from file %s\n", (int)size, fileName);
        fflush(stderr);
        fclose(f);
        free(data);
        return false;
    }
    fclose(f);
    f = NULL;
    try
    {
        minijson::CParser parser;
        minijson::CEntity* entity = parser.Parse(data);
        delete entity;
    }
    catch (const minijson::CParseErrorException& ex)
    {
        if (ex.Line() > 0)
        {
            fprintf(stderr, "ERROR: Parse error in file %s at or after line %d column %d (position %d in file):\n----------\n%s----------\nException: %s\n", fileName, ex.Line(), ex.Column(), ex.Position(), ex.Surrounding().c_str(), ex.Message().c_str());
        }
        else
        {
            fprintf(stderr, "ERROR: Parse error in file %s at or after position %d, exception: %s\n", fileName, (int)ex.Position(), ex.Message().c_str());
        }
        fflush(stderr);
        free(data);
        return false;
    }
    catch (const minijson::CException& ex)
    {
        fprintf(stderr, "ERROR: Failed to parse file %s, exception: %s\n", fileName, ex.Message().c_str());
        fflush(stderr);
        free(data);
        return false;
    }
    free(data);
    fprintf(stdout, "SUCCESSFULLY parsed %s\n", fileName);
    fflush(stdout);
    return true;
}

int main(int argc, char** argv)
{
    bool ok = true;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            fprintf(stdout, "Usage: %s <files>\n", argv[0]);
            fprintf(stdout, "  This tool will attempt to parse all specified JSON files and report any parse errors\n");
            fflush(stdout);
            return 0;
        }
    }
    for (int i = 1; i < argc; i++)
    {
        if (!Validate(argv[i]))
        {
            ok = false;
        }
    }
    if (ok)
    {
        return 0;
    }
    return 1;
}

