using System;
using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;



bool running = true;
Settings settings = LoadSettings("settings.yaml");

while (running)
{
    Console.WriteLine("Press esc to exit, otherwise it will run");
    var key = Console.ReadKey();

    if(key.Key == ConsoleKey.Escape)
        running = false;
    else
    {
        Console.WriteLine($"\nFixing library header files and CMakeFiles");
        Run(settings);
    }
}






//---------------------------------------------------

void Run(Settings settings)
{
    foreach (var item in settings.parseInfos)
    {
        var fullPath = Path.GetFullPath(item.Path);
        if(item.DoIncludes)
            CreateIncludeFile(fullPath);
        if(item.DoCMAKE)
            FixCMakeFile(fullPath);
        Console.WriteLine($"");
    }
}

Settings LoadSettings(string settingsFile)
{
    var settings = new Settings();
    if(!File.Exists(settingsFile))
    {
        var serializer = new SerializerBuilder()
            .WithNamingConvention(CamelCaseNamingConvention.Instance)
            .Build();
        File.WriteAllText(settingsFile, serializer.Serialize(settings));
    }
    else
    {
        var deserializer = new DeserializerBuilder()
            .WithNamingConvention(CamelCaseNamingConvention.Instance)  // see height_in_inches in sample yml 
            .Build();
        string yaml = File.ReadAllText(settingsFile).Trim();
        settings = deserializer.Deserialize<Settings>(yaml);
        Console.WriteLine($"Settings:");
        Console.WriteLine($"{yaml}");
    }
    return settings;
}


string[] GetPathsFromFile(string file)
{
    if (!File.Exists(file))
    {
        using var stream = File.Create(file);
    }

    return File.ReadAllLines(file).ToArray();
}


void CreateIncludeFile(string path, int indent = 0)
{
    string folderName = Path.GetFileName(path);
    string includeFile = Path.Combine(path, folderName + ".h");

    using StreamWriter outputFile = new StreamWriter(includeFile, false);
    string pre = $"{new string(' ', indent * 2)}{folderName}";
    Console.Write($"{pre}{new string(' ', 20 - pre.Length)} ( ");


    outputFile.WriteLine("#pragma once");
    outputFile.WriteLine("");


    foreach (string file in Directory.GetFiles(path, "*.h"))
    {
        if (file == includeFile)
            continue;
        string fileName = Path.GetFileName(file);
        outputFile.WriteLine($"#include \"{fileName}\"");
        Console.Write($"{fileName} ");
    }
    outputFile.WriteLine("");
    Console.WriteLine(")");

    foreach (string subDir in Directory.GetDirectories(path))
    {
        string dirName = Path.GetFileName(subDir);
        outputFile.WriteLine($"#include \"{dirName}/{dirName}.h\"");
        CreateIncludeFile(Path.Combine(path, subDir), indent + 1);
    }
}



void FixCMakeFile(string path)
{
    string file = Path.Combine(path, "CMakeLists.txt");
    string[] lines = File.ReadAllLines(file).ToArray();
    using StreamWriter outputFile = new StreamWriter(file, false);

    bool write = true;
    bool srcs = false;
    bool dirs = false;

    foreach (var line in lines)
    {
        if (line.Trim('\t', ' ').StartsWith("SRCS"))
        {
            srcs = true;
            write = false;
        }

        if (line.Trim('\t', ' ').StartsWith("INCLUDE_DIRS"))
        {
            dirs = true;
            write = false;
        }

        if (line.Trim('\t', ' ').StartsWith("REQUIRES"))
        {
            write = true;
        }

        if (write)
            outputFile.WriteLine(line);

        if (srcs)
        {
            CMakeSources(path, outputFile);
            srcs = false;

        }
        if (dirs)
        {
            CMakeDirs(path, outputFile);
            dirs = false;
        }
    }
}


void CMakeSources(string path, StreamWriter outputFile)
{
    var query = from file in Directory.EnumerateFiles(path, "*.*", SearchOption.AllDirectories)
                where file.EndsWith(".cpp") || file.EndsWith(".c")
                select file;

    var dirName = Path.GetFileName(path);
    outputFile.WriteLine("\tSRCS");
    foreach (string file in query)
    {
        string relative = Path.GetRelativePath(path, file).Replace("\\", "/");
        outputFile.WriteLine($"\t\t{relative}");
    }
}

void CMakeDirs(string path, StreamWriter outputFile)
{
    var query = from dir in Directory.EnumerateDirectories(path, "*", SearchOption.AllDirectories)
                select dir;

    outputFile.WriteLine("\tINCLUDE_DIRS");
    outputFile.WriteLine($"\t\t.");

    foreach (string dir in query)
    {
        string relative = Path.GetRelativePath(path, dir).Replace("\\", "/");
        outputFile.WriteLine($"\t\t{relative}");
    }
}





