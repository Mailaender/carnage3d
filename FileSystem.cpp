#include "stdafx.h"
#include "FileSystem.h"

FileSystem gFiles;

bool FileSystem::Initialize()
{
    char buffer[MAX_PATH + 1];
    if (::GetModuleFileNameA(NULL, buffer, MAX_PATH) == 0)
    {
        gConsole.LogMessage(eLogMessage_Warning, "FileSystem::FileSystem(), GetModuleFileNameA Failed");
        return false;
    }

    mExecutablePath.assign(buffer);
    mWorkingDirectoryPath = cxx::get_parent_directory(mExecutablePath);

//#ifdef _DEBUG
    std::string debugDataPath = cxx::get_parent_directory(mWorkingDirectoryPath);
    mSearchPlaces.emplace_back(debugDataPath + "/gamedata");
//#else
    mSearchPlaces.emplace_back(mWorkingDirectoryPath + "/gamedata");
//#endif
    return true;
}

void FileSystem::Deinit()
{
    mExecutablePath.clear();
    mWorkingDirectoryPath.clear();
    mGTADataDirectoryPath.clear();
}

bool FileSystem::OpenBinaryFile(const char* objectName, std::ifstream& instream)
{
    return false;
}

bool FileSystem::OpenTextFile(const char* objectName, std::ifstream& instream)
{
    instream.close();

    if (cxx::is_absolute_path(objectName))
    {
        instream.open(objectName, std::ios::in);
        return instream.is_open();
    }

    cxx::string_buffer_512 pathBuffer;
    // search file in search places
    for (const std::string& currPlace: mSearchPlaces)
    {
        pathBuffer.printf("%s/%s", currPlace.c_str(), objectName);
        if (IsFileExists(pathBuffer.c_str()))
        {
            instream.open(pathBuffer.c_str(), std::ios::in);
            return instream.is_open();
        }
        return false;
    }
    return false;
}

bool FileSystem::IsDirectoryExists(const char* objectName)
{
    if (cxx::is_absolute_path(objectName))
    {
        return cxx::is_directory_exists(objectName);
    }
    cxx::string_buffer_512 pathBuffer;
    // search directory in search places
    for (const std::string& currPlace: mSearchPlaces)
    {
        pathBuffer.printf("%s/%s", currPlace.c_str(), objectName);
        if (cxx::is_directory_exists(pathBuffer.c_str()))
            return true;
    }
    return false;
}

bool FileSystem::IsFileExists(const char* objectName)
{
    if (cxx::is_absolute_path(objectName))
    {
        return cxx::is_file_exists(objectName);
    }
    cxx::string_buffer_512 pathBuffer;
    // search file in search places
    for (const std::string& currPlace: mSearchPlaces)
    {
        pathBuffer.printf("%s/%s", currPlace.c_str(), objectName);
        if (cxx::is_file_exists(pathBuffer.c_str()))
            return true;
    }
    return false;
}

bool FileSystem::ReadTextFile(const char* objectName, std::string& output)
{
    output.clear();

    std::ifstream fileStream;
    if (!OpenTextFile(objectName, fileStream))
        return false;

    std::string stringLine {};
    while (std::getline(fileStream, stringLine, '\n'))
    {
        output.append(stringLine);
        output.append("\n");
    }
    return true;
}

void FileSystem::AddSearchPlace(const char* searchPlace)
{
    for (const std::string& currPlace: mSearchPlaces)
    {
        if (currPlace == searchPlace)
            return;
    }

    mSearchPlaces.emplace_back(searchPlace);
}