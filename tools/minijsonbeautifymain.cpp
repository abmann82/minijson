#include <minijson.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool Beautify(const char* inputFileName, const char* outputFileName);
static bool Beautify(const std::vector<char>& data, const char* inputFileName, const char* outputFileName);

static bool Beautify(const char* inputFileName, const char* outputFileName)
{
    FILE* output = nullptr;
    if (outputFileName)
    {
        output = fopen(outputFileName, "r");
        if (output)
        {
            fclose(output);
            fprintf(stderr, "ERROR: Output file '%s' already exists\n", outputFileName);
            return 1;
        }
    }
    FILE* input = fopen(inputFileName, "r");
    if (!input)
    {
        fprintf(stderr, "ERROR: Failed to open file %s for reading\n", inputFileName);
        fflush(stderr);
        return false;
    }
    fseek(input, 0, SEEK_END);
    long size = ftell(input);
    fseek(input, 0, SEEK_SET);
    if (size <= 0)
    {
        fprintf(stderr, "ERROR: Empty file %s\n", inputFileName);
        fflush(stderr);
        fclose(input);
        return false;
    }
    if (size > 1024 * 1024 * 20)
    {
        fprintf(stderr, "ERROR: File too large: %s\n", inputFileName);
        fflush(stderr);
        fclose(input);
        return false;
    }
    std::vector<char> data((size_t)size + 1);
    if (fread(data.data(), 1, (size_t)size, input) != (size_t)size)
    {
        fprintf(stderr, "ERROR: Failed to read %d bytes from file %s\n", (int)size, inputFileName);
        fflush(stderr);
        fclose(input);
        return false;
    }
    fclose(input);
    input = NULL;
    return Beautify(data, inputFileName, outputFileName);
}

static bool Beautify(const std::vector<char>& data, const char* inputFileName, const char* outputFileName)
{
    std::unique_ptr<minijson::CEntity> entity;
    try
    {
        minijson::CParser parser;
        entity.reset(parser.Parse(data.data()));
    }
    catch (const minijson::CParseErrorException& ex)
    {
        if (ex.Line() > 0)
        {
            fprintf(stderr, "ERROR: Parse error in file %s at or after line %d column %d (position %d in file):\n----------\n%s----------\nException: %s\n", inputFileName, ex.Line(), ex.Column(), ex.Position(), ex.Surrounding().c_str(), ex.Message().c_str());
        }
        else
        {
            fprintf(stderr, "ERROR: Parse error in file %s at or after position %d, exception: %s\n", inputFileName, (int)ex.Position(), ex.Message().c_str());
        }
        fflush(stderr);
        return false;
    }
    catch (const minijson::CException& ex)
    {
        fprintf(stderr, "ERROR: Failed to parse file %s, exception: %s\n", inputFileName, ex.Message().c_str());
        fflush(stderr);
        return false;
    }
    minijson::CWriter writer;
    if (outputFileName)
    {
        writer.WriteToFile(outputFileName, *entity);
        fprintf(stderr, "SUCCESSFULLY wrote %s to %s\n", inputFileName, outputFileName);
        fflush(stderr);
    }
    else
    {
        writer.WriteToFile(stdout, *entity);
        fflush(stdout);
    }
    return true;
}

static void usage(const char* argv0)
{
    fprintf(stderr, "Usage: %s <input> [<output>]\n", argv0);
    fprintf(stderr, "  This tool will attempt to file <input> and output it to <output>.\n");
    fprintf(stderr, "  If <output> is omitted, stdout is used.\n");
    fflush(stderr);
}

int main(int argc, char** argv)
{
    bool ok = true;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            usage(argv[0]);
            return 0;
        }
    }
    if (argc > 1)
    {
        const char* input = argv[1];
        const char* output = nullptr;
        if (argc > 2)
        {
            output = argv[2];
        }
        if (!Beautify(input, output))
        {
            ok = false;
        }
    }
    else
    {
        // try reading from stdin
        std::vector<char> data;
        bool done = false;
        while (!done)
        {
            size_t offset = data.size();
            data.resize(data.size() + 1024);
            size_t s = fread(data.data() + offset, 1, 1024, stdin);
            if (s < 1024)
            {
                done = true;
                data.resize(offset + s);
            }
        }

        if (!data.empty())
        {
            ok = Beautify(data, "<stdin>", nullptr);
        }
        else
        {
            fprintf(stderr, "ERROR: No data on stdin and no arguments provided\n");
            usage(argv[0]);
            return 1;
        }
    }
    if (ok)
    {
        return 0;
    }
    return 1;
}

