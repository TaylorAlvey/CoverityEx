#pragma region includes
#include "Compiler.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#pragma endregion

#pragma region AdditionalFunctions
std::string getProgramNameFromUser()
{
    std::string userInput = "";
    std::cout << "Enter the name of the Bobball file to compile: " << std::endl;
    std::getline(std::cin, userInput);
    return userInput;
}

std::vector<std::string> readInFile()
{
    std::vector<std::string> linesFromFile;

    std::string programNameFromUser = getProgramNameFromUser();
    std::fstream newFile;
    newFile.open(programNameFromUser, std::ios::in);
    if (!newFile.is_open())
    {
        return linesFromFile;
    }

    std::string newLine = "";
    while (std::getline(newFile, newLine))
    {
        std::stringstream ss(newLine, ';');
        while (ss.good())
        {
            std::string substr;
            std::getline(ss, substr, ';');
            if (ss.good())
            {
                linesFromFile.push_back(substr + ';');
            }
            else
            {
                linesFromFile.push_back(substr);
            }
        }
    }

    newFile.close();
    return linesFromFile;
}

void removeSinglelineComments(std::vector<std::string>& linesFromFile)
{
    static const std::string tokenLineComment = "//";
    for (unsigned int i = 0; i < linesFromFile.size(); i++)
    {
        std::string line = linesFromFile.at(i);
        auto tokenFoundAt = line.find(tokenLineComment);
        if (tokenFoundAt != std::string::npos)
        {
            std::string newLineWithoutComment = line.substr(0, tokenFoundAt);
            linesFromFile[i] = newLineWithoutComment;
        }
    }
}

void removeMultilineComments(std::vector<std::string>& linesFromFile)
{
    static const std::string tokenStartComment = "/*";
    static const std::string tokenEndComment = "*/";
    for (unsigned int i = 0; i < linesFromFile.size(); i++)
    {
        unsigned int endOfCommentIndex = i;
        std::string line = linesFromFile.at(i);
        auto startTokenFoundAt = line.find(tokenStartComment);
        if (startTokenFoundAt == std::string::npos)
        {
            continue;
        }

        auto endTokenFoundAt = line.find(tokenEndComment);
        while (endTokenFoundAt == std::string::npos && endOfCommentIndex + 1 != linesFromFile.size())
        {
            endOfCommentIndex++;
            line = linesFromFile[endOfCommentIndex];
            endTokenFoundAt = line.find(tokenEndComment);
        }

        if (endOfCommentIndex == i)
        {
            std::string newLineWithoutComment = line.substr(0, startTokenFoundAt) + line.substr(endTokenFoundAt + tokenEndComment.size(), line.size());
            linesFromFile[i] = newLineWithoutComment;
        }
        else
        {
            //Truncate first comment line
            std::string newLineWithoutComment = linesFromFile.at(i).substr(0, startTokenFoundAt);
            linesFromFile[i] = newLineWithoutComment;
            //Remove the start of the last comment line
            newLineWithoutComment = linesFromFile.at(endOfCommentIndex).substr(endTokenFoundAt + tokenEndComment.size(), linesFromFile.at(endOfCommentIndex).size());
            linesFromFile[endOfCommentIndex] = newLineWithoutComment;
            //Remove all middle comment lines
            if (endOfCommentIndex - i > 2)
            {
                linesFromFile.erase(linesFromFile.begin() + i + 1, linesFromFile.begin() + endOfCommentIndex);
            }
        }
    }
}

void removeBlankLines(std::vector<std::string>& linesFromFile)
{
    for (unsigned int i = 0; i < linesFromFile.size();)
    {
        std::string line = linesFromFile.at(i);
        if (line.find_first_not_of("\t\n ") == std::string::npos)
        {
            linesFromFile.erase(linesFromFile.begin() + i, linesFromFile.begin() + i + 1);
        }
        else
        {
            i++;
        }
    }
}

void removeTabs(std::vector<std::string>& linesFromFile)
{
    for (unsigned int i = 0; i < linesFromFile.size(); i++)
    {
        std::string line = linesFromFile.at(i);
        line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());
        linesFromFile[i] = line;
    }
}

void adjustSpacing(std::vector<std::string>& linesFromFile, std::string delimiters)
{
    for (unsigned int i = 0; i < linesFromFile.size(); i++)
    {
        std::string line = linesFromFile[i];
        std::vector<std::string> words = Compiler::split(line, delimiters);
        std::string newLine = words[0];
        for (int j = 1; j < words.size(); ++j)
        {
            if (words[j] != ";")
            {
                newLine += " " + words[j];
            }
            else
            {
                newLine += words[j];
            }
        }
        linesFromFile[i] = newLine;
    }
}

void changeSubtractionToAddingNegative(std::vector<std::string>& linesFromFile)
{
    std::string sRegexVariableName(R"([a-zA-Z][a-zA-Z0-9_]*)");
    std::string sRegexNumLiteral(R"(-?[0-9]+)");
    std::string sRegexFloatLiteral(R"(-?[0-9]+\.?[0-9]*)");
    std::string sRegexOperators("[+^*]");
    std::string sRegexVariableNameOrNumLiteral(R"(()" + sRegexVariableName + R"(|)" + sRegexNumLiteral + R"())");

    //Change 2-2 to 2+-2
    std::string sRegexSingleMinusLiteral(sRegexVariableNameOrNumLiteral + " ?(-) " + sRegexNumLiteral);
    //Change 2--2 to 2+2
    std::string sRegexDoubleMinusLiteral("(-) ?(-) ?[0-9]+");
    //Change 2+-num
    std::string sRegexDoubleMinusVariable("(-) ?(-) ?" + sRegexVariableName);
    //Change 2--num
    std::string sRegexPlusNegativeVariable("(+) ?(-) ?" + sRegexVariableName);



    for (unsigned int i = 0; i < linesFromFile.size(); i++)
    {
        std::smatch sm;
        //std::string sRegexVariableName(R"(\(?\)?[a-zA-Z][a-zA-Z0-9_]*\(?\)?)");
        //std::string sRegexNumLiteral(R"(\(?\)?-?[0-9]+\(?\)?)");
        //std::string sRegexVariableNameOrNumLiteral("(" + sRegexVariableName + "|" + sRegexNumLiteral + ")");
        std::vector<std::string> words = Compiler::split(linesFromFile[i]);
        if (words.size() < 3)
        {
            continue;
        }
        for (int j = 0; j < words.size() - 2; ++j)
        {
            std::vector<std::string> set{ words[j], words[j + 1], words[j + 2] };
            if (std::regex_match(set[0], sm, std::regex(R"(\(?)" + sRegexVariableNameOrNumLiteral)) && std::regex_match(set[1], sm, std::regex("-")) && std::regex_match(set[2], sm, std::regex("-" + sRegexVariableNameOrNumLiteral + R"(\)?)")))
            {
                words[j + 1] = "+";
                words[j + 2] = words[j + 2].substr(1, words[j + 2].size());
            }
            else if (std::regex_match(set[0], sm, std::regex(R"(\(?)" + sRegexVariableNameOrNumLiteral + R"(\)?)")) && std::regex_match(set[1], sm, std::regex("-")) && std::regex_match(set[2], sm, std::regex(sRegexVariableNameOrNumLiteral + R"(\)?)")))
            {
                words[j + 1] = "+";
                words[j + 2] = "-" + words[j + 2];
            }
        }

        std::string newLine = words[0];
        for (int i = 1; i < words.size(); ++i)
        {
            newLine += " " + words[i];
        }
        newLine += ";";
        linesFromFile[i] = newLine;

        //std::regex regexDoubleMinusLiteral(sRegexDoubleMinusLiteral);
        //while (std::regex_search(linesFromFile[i], sm, regexDoubleMinusLiteral))
        //{
        //    linesFromFile[i] = linesFromFile[i].substr(0, sm.position()) + "+ " + linesFromFile[i].substr(sm.position() + 4, linesFromFile[i].size());
        //}

        //std::regex regexSingleMinusLiteral(sRegexSingleMinusLiteral);
        //while (std::regex_search(linesFromFile[i], sm, regexSingleMinusLiteral))
        //{
        //    //linesFromFile[i] = linesFromFile[i].substr(0, linesFromFile[i].find_first_of("-")) + "+ -" + linesFromFile[i].substr(linesFromFile[i].find_first_of("-") + 2, linesFromFile[i].size());
        //    std::string first = linesFromFile[i].substr(0, sm.position() + 2);
        //    std::string second = "+ -";
        //    std::string third = linesFromFile[i].substr(sm.position() + sm.length() - 1, linesFromFile[i].size());
        //    linesFromFile[i] = first + second + third;
        //}

        //std::regex regexDoubleMinusVariable(sRegexDoubleMinusVariable);
        //while (std::regex_search(linesFromFile[i], sm, regexDoubleMinusVariable))
        //{
        //    linesFromFile[i] = linesFromFile[i].substr(0, sm.position()) + "+ " + linesFromFile[i].substr(sm.position() + 3, linesFromFile[i].size());
        //}

        /*while (std::regex_search(linesFromFile[i], sm, std::regex("- ")))
        {
            linesFromFile[i] = sm.prefix().str() + "-" + sm.suffix().str();
        }*/
    }
}

void cleanUpFile(std::vector<std::string>& linesFromFile)
{
    removeSinglelineComments(linesFromFile);
    removeMultilineComments(linesFromFile);
    removeBlankLines(linesFromFile);
    removeTabs(linesFromFile);
    adjustSpacing(linesFromFile, "+^= ");
    changeSubtractionToAddingNegative(linesFromFile);
    adjustSpacing(linesFromFile, "+^=() ");
}

void writeAsmFile(Compiler& comp, std::vector<std::string> compiledText)
{
    std::fstream newFile;
    newFile.open(comp.getProgramName() + ".asm", std::ios::trunc | std::ios::out);
    if (!newFile.is_open())
    {
        return;
    }

    for (std::string line : compiledText)
    {
        newFile << line << std::endl;
    }

    newFile.close();
}
#pragma endregion

int main()
{
    try
    {
        std::vector<std::string> linesFromFile = readInFile();
        if (linesFromFile.size() == 0)
        {
            std::cout << "Error reading file" << std::endl;
            return 1;
        }
        cleanUpFile(linesFromFile);
        /*for (std::string line : linesFromFile)
        {
            std::cout << line << std::endl;
        }*/
        Compiler comp;
        //comp.testRegex();
        writeAsmFile(comp, comp.compileLines(linesFromFile));
    }
    catch (std::exception e)
    {
        std::cout << "Error when compiling" << std::endl;
    }
}
