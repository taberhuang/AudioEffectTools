#pragma once
#include <JuceHeader.h>

class Utils
{
public:
    // Smart path wrapping function
    static juce::String wrapPath(const juce::String& path, int maxCharsPerLine = 40)
    {
        juce::StringArray parts;
        juce::String result;
        juce::String currentLine;
        
        // Split using path separator
        #if JUCE_WINDOWS
            juce::String separator = "\\";
        #else
            juce::String separator = "/";
        #endif
        
        // Split the path
        int start = 0;
        while (start < path.length())
        {
            int pos = path.indexOfChar(start, separator[0]);
            if (pos < 0)
            {
                parts.add(path.substring(start));
                break;
            }
            parts.add(path.substring(start, pos + 1));
            start = pos + 1;
        }
        
        // Recombine with line breaks at appropriate positions
        for (const auto& part : parts)
        {
            if (currentLine.length() + part.length() > maxCharsPerLine && currentLine.isNotEmpty())
            {
                result += currentLine + "\n";
                currentLine = part;
            }
            else
            {
                currentLine += part;
            }
        }
        
        if (currentLine.isNotEmpty())
            result += currentLine;
        
        return result;
    }
    
    // Additional path-related utility functions can be added here
    static juce::String getFileNameWithoutPath(const juce::String& fullPath)
    {
        return juce::File(fullPath).getFileName();
    }
    
    static juce::String shortenPath(const juce::String& fullPath, int maxLength = 50)
    {
        if (fullPath.length() <= maxLength)
            return fullPath;
        
        juce::File file(fullPath);
        juce::String fileName = file.getFileName();
        juce::String parentDir = file.getParentDirectory().getFileName();
        
        if (fileName.length() > 30)
        {
            fileName = fileName.substring(0, 27) + "...";
        }
        
        return "..." + juce::File::getSeparatorString() + parentDir + 
               juce::File::getSeparatorString() + fileName;
    }
};