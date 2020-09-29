#pragma region includes
#include "Compiler.h"
#include "BobballArray.h"

#include <iostream>
#include <regex>
#include <stack>
#pragma endregion

#pragma region Regex
std::string Compiler::sRegexVariableName("[a-zA-Z][a-zA-Z0-9_]*" + sRegexZeroOrMoreWhiteSpace);
std::string Compiler::sRegexNumLiteral(R"(-?[0-9]+)" + sRegexZeroOrMoreWhiteSpace);
std::string Compiler::sRegexFloatLiteral(R"(-?[0-9]+\.?[0-9]*)" + sRegexZeroOrMoreWhiteSpace);
std::string Compiler::sRegexVariableNameOrNumLiteral("(" + sRegexVariableName + "|" + sRegexNumLiteral + ")");
std::string Compiler::sRegexOperators("[-+^*]");
std::string Compiler::sRegexZeroOrMoreWhiteSpace(R"([\s\t\r\n]*)");
std::string Compiler::sRegexEndsInSemicolon(".*;" + sRegexZeroOrMoreWhiteSpace);
std::string Compiler::sRegexAe(sRegexVariableNameOrNumLiteral + "( " + sRegexOperators + " " + sRegexVariableNameOrNumLiteral + ")+");

std::string Compiler::sRegexProgramLine("program " + sRegexVariableName + ";" + sRegexZeroOrMoreWhiteSpace);
std::string Compiler::sRegexBeginLine("begin" + sRegexZeroOrMoreWhiteSpace);
std::string Compiler::sRegexRawEndLine(R"(end\.)");
std::string Compiler::sRegexEndLine(sRegexRawEndLine + sRegexZeroOrMoreWhiteSpace);

std::string Compiler::sRegexNumDeclaration("num " + sRegexVariableName + ";" + sRegexZeroOrMoreWhiteSpace);
std::string Compiler::sRegexNumDeclarationThenAssignmentToVariable("num " + sRegexVariableName + " = " + sRegexVariableName + ";" + sRegexZeroOrMoreWhiteSpace);
std::string Compiler::sRegexNumDeclarationThenAssignmentToLiteral("num " + sRegexVariableName + " = " + sRegexNumLiteral + ";" + sRegexZeroOrMoreWhiteSpace);
std::string Compiler::sRegexNumDeclarationThenAssignmentThenOperation("num " + sRegexVariableName + " = " + sRegexVariableNameOrNumLiteral + " " + sRegexOperators + " " + sRegexVariableNameOrNumLiteral + ";" + sRegexZeroOrMoreWhiteSpace);
std::string Compiler::sRegexNumDeclarationThenAssignmentThenAe("num " + sRegexVariableName + " = " + sRegexAe + ";" + sRegexZeroOrMoreWhiteSpace);

std::string Compiler::sRegexNumAssignment(sRegexVariableName + " = " + sRegexVariableNameOrNumLiteral + ";" + sRegexZeroOrMoreWhiteSpace);
std::string Compiler::sRegexAssignmentThenOperation(sRegexVariableName + " = " + sRegexVariableNameOrNumLiteral + " " + sRegexOperators + " " + sRegexVariableNameOrNumLiteral + ";" + sRegexZeroOrMoreWhiteSpace);
std::string Compiler::sRegexAssignmentThenAe(sRegexVariableName + " = " + sRegexAe + ";" + sRegexZeroOrMoreWhiteSpace);

std::string Compiler::sRegexArrayRange(sRegexNumLiteral + R"(\.\.)" + sRegexNumLiteral);
std::string Compiler::sRegexArrayDeclarationStatement("array " + sRegexVariableName + R"(\[)" + sRegexArrayRange + "(," + sRegexArrayRange + ")*" + R"(\])" + ";" + sRegexZeroOrMoreWhiteSpace);
std::string Compiler::sRegexArraySpecificIndex(sRegexVariableName + R"(\[)" + sRegexNumLiteral + "(," + sRegexNumLiteral + ")*" + R"(\])");
std::string Compiler::sRegexArrayAssignmentStatement(sRegexArraySpecificIndex + " = " + sRegexNumLiteral + ";" + sRegexZeroOrMoreWhiteSpace);

std::string Compiler::sRegexWriteNum("write " + sRegexVariableNameOrNumLiteral + ";" + sRegexZeroOrMoreWhiteSpace);
std::string Compiler::sRegexWriteRaw(R"(write ".*")");
std::string Compiler::sRegexWriteStringLiteral(sRegexWriteRaw + ";" + sRegexZeroOrMoreWhiteSpace);
std::string Compiler::sRegexWriteArrayNum("write " + sRegexArraySpecificIndex + ";" + sRegexZeroOrMoreWhiteSpace);

void Compiler::testRegex()
{
	std::string regexToUse = sRegexAssignmentThenAe;
	//std::string toMatch = "result = 3 ^ X + 67 - 34 - 77 * 2 ^ X;";
	std::string toMatch = "result = 3 ^ X + 67 + -34 -77 * 2 ^ X; ";
	std::regex theRegex;
	try
	{
		theRegex = std::regex(regexToUse, std::regex_constants::icase);
	}
	catch (std::regex_error rgxError)
	{
		std::cout << rgxError.what() << std::endl;
		std::cout << regexToUse << std::endl;
		exit(1);
	}

	if (std::regex_match(toMatch , theRegex))
	{
		std::cout << "Regex: " + regexToUse + " matches: " + toMatch << std::endl;
	}
	else
	{
		std::cout << "Regex: " + regexToUse + " doesn't match: " + toMatch << std::endl;
	}
}
#pragma endregion

#pragma region asmWriteFunctions
std::string Compiler::buildAsmNumVariableName(std::string& declaredWord)
{
	numberOfNumberVariables++;
	return "_n_" + std::to_string(numberOfNumberVariables-1) + "_" + declaredWord;
}

std::string Compiler::buildAsmTempNumVariableName(std::string& declaredWord)
{
	numberOfTempNumberVariables++;
	return declaredWord + "_" + std::to_string(numberOfTempNumberVariables);
}

std::string Compiler::buildAsmStringLiteralVariableName()
{
	numberOfStringVariables++;
	return "s" + std::to_string(numberOfStringVariables - 1);
}

std::string Compiler::buildAsmArrayVariableName(std::string& declaredWord)
{
	numberOfArrayVariables++;
	return "_g_" + declaredWord;
}

void Compiler::asmWriteNumDirectAssignmentToVariable(std::string declaredWord, std::string assignedVariable)
{
	sCompiledMain += "\tmov edi,\tDWORD[" + mapCodeNamesToAsmNames[assignedVariable] + "]\n";
	sCompiledMain += "\tmov DWORD[" + mapCodeNamesToAsmNames[declaredWord] + "],\t edi\n\n";
}

void Compiler::asmWriteNumDeclarationWithAssignmentToLiteral(std::string declaredWord, std::string assignedLiteral)
{
	if (isRedeclaration(declaredWord))
	{
		return;
	}
	listVariables.push_back(declaredWord);
	std::string asmNumVariableName = buildAsmNumVariableName(declaredWord);
	mapCodeNamesToAsmNames.emplace(declaredWord, asmNumVariableName);
	sInitializedData += "\t" + asmNumVariableName + "\t\tdd\t" + assignedLiteral + "\n";
}

void Compiler::asmWriteNumDeclarationWithoutAssignment(std::string word)
{
	if (isRedeclaration(word))
	{
		return;
	}
	listVariables.push_back(word);
	std::string asmNumVariableName = buildAsmNumVariableName(word);
	mapCodeNamesToAsmNames.emplace(word, asmNumVariableName);
	sUninitializedData += "\t" + asmNumVariableName + "\tresd\t1\n";
}

void Compiler::asmWriteTempNumDeclarationWithoutAssignment(std::string word)
{
	std::string newWord = word + std::to_string(numberOfTempNumberVariables + 1);
	if (isRedeclaration(newWord))
	{
		return;
	}
	listVariables.push_back(newWord);
	std::string asmNumVariableName = buildAsmTempNumVariableName(word);
	mapCodeNamesToAsmNames.emplace(newWord, asmNumVariableName);
	sUninitializedData += "\t" + asmNumVariableName + "\tresd\t1\n";
}

void Compiler::asmWriteNumAssignment(std::vector<std::string> words, bool secondNumIsVariable)
{
	std::string firstNum = "DWORD[" + mapCodeNamesToAsmNames[words[0]] + "]";
	std::string secondNum = secondNumIsVariable ? "DWORD[" + mapCodeNamesToAsmNames[words[2]] + "]" : words[2];

	sCompiledMain += "\tmov edi,\t" + secondNum + "\n";
	sCompiledMain += "\tmov " + firstNum + ",\tedi\n\n";
}

void Compiler::asmWriteNumAssignmentToResultOfAddition(std::vector<std::string> words, bool secondNumIsVariable, bool thirdNumIsVariable)
{
	std::string firstNum = "DWORD[" + mapCodeNamesToAsmNames[words[0]] + "]";
	std::string secondNum = secondNumIsVariable ? "DWORD[" + mapCodeNamesToAsmNames[words[2]] + "]" : words[2];
	std::string thirdNum = thirdNumIsVariable ? "DWORD[" + mapCodeNamesToAsmNames[words[4]] + "]" : words[4];

	sCompiledMain += "\tmov edi,\t" + secondNum + "\n";
	sCompiledMain += "\tadd edi,\t" + thirdNum + "\n";
	sCompiledMain += "\tmov " + firstNum + ",\tedi\n\n";
}

void Compiler::asmWriteNumAssignmentToResultOfSubtraction(std::vector<std::string> words, bool secondNumIsVariable, bool thirdNumIsVariable)
{
	std::string firstNum = "DWORD[" + mapCodeNamesToAsmNames[words[0]] + "]";
	std::string secondNum = secondNumIsVariable ? "DWORD[" + mapCodeNamesToAsmNames[words[2]] + "]" : words[2];
	std::string thirdNum = thirdNumIsVariable ? "DWORD[" + mapCodeNamesToAsmNames[words[4]] + "]" : words[4];

	sCompiledMain += "\tmov edi,\t" + secondNum + "\n";
	sCompiledMain += "\tsub edi,\t" + thirdNum + "\n";
	sCompiledMain += "\tmov " + firstNum + ",\tedi\n\n";
}

void Compiler::asmWriteNumAssignmentToResultOfMultiplication(std::vector<std::string> words, bool secondNumIsVariable, bool thirdNumIsVariable)
{
	std::string firstNum = "DWORD[" + mapCodeNamesToAsmNames[words[0]] + "]";
	std::string secondNum = secondNumIsVariable ? "DWORD[" + mapCodeNamesToAsmNames[words[2]] + "]" : words[2];
	std::string thirdNum = thirdNumIsVariable ? "DWORD[" + mapCodeNamesToAsmNames[words[4]] + "]" : words[4];

	sCompiledMain += "\tmov edi,\t" + secondNum + "\n";
	sCompiledMain += "\timul edi,\t" + thirdNum + "\n";
	sCompiledMain += "\tmov " + firstNum + ",\tedi\n\n";
}

void Compiler::asmWriteNumAssignmentToResultOfExponentiation(std::vector<std::string> words, bool secondNumIsVariable, bool thirdNumIsVariable)
{
	std::string firstNum = "DWORD[" + mapCodeNamesToAsmNames[words[0]] + "]";
	std::string secondNum = secondNumIsVariable ? "DWORD[" + mapCodeNamesToAsmNames[words[2]] + "]" : words[2];
	std::string thirdNum = thirdNumIsVariable ? "DWORD[" + mapCodeNamesToAsmNames[words[4]] + "]" : words[4];

	sCompiledMain += "\txor edi,\tedi\n";
	sCompiledMain += "\tmov eax,\t0x00000001\n";
	sCompiledMain += "_exp_top_" + std::to_string(numberOfExponentiationLoops) + ":\n";
	sCompiledMain += "\tcmp edi,\t" + thirdNum + "\n";
	sCompiledMain += "\tjz _exp_out_" + std::to_string(numberOfExponentiationLoops) + "\n";
	sCompiledMain += "\timul eax,\t" + secondNum + "\n";
	sCompiledMain += "\tinc edi\n";
	sCompiledMain += "\tjmp _exp_top_" + std::to_string(numberOfExponentiationLoops) + "\n";
	sCompiledMain += "_exp_out_" + std::to_string(numberOfExponentiationLoops) + ":\n";
	sCompiledMain += "\tmov " + firstNum + ",\teax\n\n";

	numberOfExponentiationLoops++;
}

void Compiler::asmWriteArrayDeclaration(std::string declaredWord, std::shared_ptr<BobballArray> newArray)
{
	std::string asmArrayName = buildAsmArrayVariableName(declaredWord);
	mapCodeNamesToAsmNames.emplace(declaredWord, asmArrayName);
	mapArrayNamesToArrays.emplace(declaredWord, newArray);
	sUninitializedData += "\t" + asmArrayName + "\tresb\t" + std::to_string(newArray->getTotalReservedSpace()) + "\n";
}

void Compiler::asmWriteArrayAssignment(std::string variableName, std::vector<int> indices, std::string numToAssign)
{
	std::shared_ptr<BobballArray> currentArray = mapArrayNamesToArrays[variableName];
	auto bytePosition = currentArray->getBytePosition(indices);
	sCompiledMain += "\txor edi,\tedi\n";
	sCompiledMain += "\tmov edi,\t" + std::to_string(bytePosition) + "\n";
	sCompiledMain += "\tadd edi,\t" + mapCodeNamesToAsmNames[variableName] + "\n";
	sCompiledMain += "\tmov DWORD[edi], " + numToAssign + "\n\n";
}

void Compiler::asmWriteNumberPrinter()
{
	sInitializedData += "\tnumberPrinter\tdb \"%d\",0x0d,0x0a,0\n";
}

void Compiler::asmWriteStringPrinter()
{
	sInitializedData += "\tstringPrinter\tdb \"%s\",0\n";
}

void Compiler::asmWriteInitializedStringVariable(std::string sLiteral)
{
	std::string asmStringVariableName = buildAsmStringLiteralVariableName();
	mapCodeNamesToAsmNames.emplace(sLiteral, asmStringVariableName);
	sInitializedData += "\t" + asmStringVariableName + "\tdb " + sLiteral + ",0x0d,0x0a,0\n";
}

void Compiler::asmWriteNumWrite(std::string sNum, bool isVariable)
{
	if (!numberPrinterCreated)
	{
		asmWriteNumberPrinter();
		numberPrinterCreated = true;
	}
	std::string numToPrint = isVariable ? "DWORD[" + mapCodeNamesToAsmNames[sNum] + "]" : sNum;
	std::string printer = "numberPrinter";
	asmWrite(numToPrint, printer);
}

void Compiler::asmWriteStringLiteralWrite(std::string sLiteral, bool isVariable)
{
	if (mapCodeNamesToAsmNames.find(sLiteral) == mapCodeNamesToAsmNames.end())
	{
		asmWriteInitializedStringVariable(sLiteral);
	}
	if (!stringPrinterCreated)
	{
		asmWriteStringPrinter();
		stringPrinterCreated = true;
	}
	std::string stringLiteralToPrint = mapCodeNamesToAsmNames[sLiteral];
	std::string printer = "stringPrinter";
	asmWrite(stringLiteralToPrint, printer);
}

void Compiler::asmWriteArrayNumWrite(std::string variableName, std::vector<int> indices)
{
	if (!numberPrinterCreated)
	{
		asmWriteNumberPrinter();
		numberPrinterCreated = true;
	}
	std::shared_ptr<BobballArray> currentArray = mapArrayNamesToArrays[variableName];
	auto bytePosition = currentArray->getBytePosition(indices);

	sCompiledMain += "\txor edi,\tedi\n";
	sCompiledMain += "\tmov edi,\t" + std::to_string(bytePosition) + "\n";
	sCompiledMain += "\tadd edi,\t" + mapCodeNamesToAsmNames[variableName] + "\n";

	std::string wordToPrint = "DWORD[edi]";
	std::string printer = "numberPrinter";
	asmWrite(wordToPrint, printer);
}

void Compiler::asmWrite(std::string wordToPrint, std::string printer)
{
	sCompiledMain += "\tpush\t" + wordToPrint + "\n";
	sCompiledMain += "\tpush\t" + printer + "\n";
	sCompiledMain += "\tcall\t_printf\n";
	sCompiledMain += "\tadd esp,\t0x08\n\n";
}
#pragma endregion

#pragma region HandleFunctions
void Compiler::handleProgram(std::vector<std::string>& lines)
{
	if (!std::regex_match(lines[nextLineNumberToCompile], std::regex(sRegexProgramLine)))
	{
		failedToCompile(lines[nextLineNumberToCompile]);
	}
	auto firstLine = split(lines[nextLineNumberToCompile]);
	programName = removeLastCharIfSemicolon(firstLine[1]);
	nextLineNumberToCompile++;
}

void Compiler::handleBegin(std::vector<std::string>& lines)
{
	if (!std::regex_match(lines[nextLineNumberToCompile], std::regex(sRegexBeginLine)))
	{
		failedToCompile(lines[nextLineNumberToCompile]);
	}
	nextLineNumberToCompile++;
}

void Compiler::handleDeclarationThenAssignmentThenOperation(std::string line)
{
	auto words = split(line);
	handleNumDeclarationWithoutAssignment(words[1]);

	//variableName = variableNameOrNumLiteral operator variableNameOrNumLiteral
	handleNumAssignmentToResultOfOperation(words[1] + " = " + words[3] + " " + words[4] + " " + words[5]);
}

void Compiler::handleDeclarationThenAssignmentThenAe(std::string line)
{
	auto words = split(line);
	handleNumDeclarationWithoutAssignment(words[1]);

	std::string assignmentLine = words[1];
	for (int i = 2; i < words.size(); ++i)
	{
		assignmentLine += " " + words[i];
	}

	handleNumAssignmentToResultOfAe(assignmentLine);
}

void Compiler::handleNumDeclarationWithAssignmentToVariable(std::string declaredWord, std::string assignedVariable)
{
	if (mapCodeNamesToAsmNames.find(assignedVariable) == mapCodeNamesToAsmNames.end())
	{
		failedToCompile(declaredWord + " = " + assignedVariable + ", variable: " + assignedVariable + " hasn't been declared");
	}
	asmWriteNumDeclarationWithoutAssignment(declaredWord);
	asmWriteNumDirectAssignmentToVariable(declaredWord, assignedVariable);
}

void Compiler::handleNumDeclarationWithAssignmentToLiteral(std::string declaredWord, std::string assignedLiteral)
{
	asmWriteNumDeclarationWithAssignmentToLiteral(declaredWord, assignedLiteral);
}

void Compiler::handleNumDeclarationWithoutAssignment(std::string word)
{
	asmWriteNumDeclarationWithoutAssignment(word);
}

void Compiler::handleTempNumDeclarationWithoutAssignment(std::string word)
{
	asmWriteTempNumDeclarationWithoutAssignment(word);
}

void Compiler::handleNumAssignment(std::string line)
{
	auto words = split(removeLastCharIfSemicolon(line));
	words[words.size() - 1] = removeLastCharIfSemicolon(words[words.size() - 1]);
	std::string firstVariable = words[0];
	if (mapCodeNamesToAsmNames.find(firstVariable) == mapCodeNamesToAsmNames.end())
	{
		failedToCompile(line + ", variable: " + firstVariable + " hasn't been declared");
	}
	bool secondNumIsVariable = false;
	if (std::regex_match(words[2], std::regex(sRegexVariableName)))
	{
		std::string secondVariable = words[2];
		if (mapCodeNamesToAsmNames.find(secondVariable) == mapCodeNamesToAsmNames.end())
		{
			failedToCompile(line + ", variable: " + secondVariable + " hasn't been declared");
		}
		secondNumIsVariable = true;
	}
	asmWriteNumAssignment(words, secondNumIsVariable);
}

void Compiler::handleNumAssignmentToResultOfOperation(std::string line)
{
	auto words = split(line);
	std::string firstVariable = words[0];
	if (mapCodeNamesToAsmNames.find(firstVariable) == mapCodeNamesToAsmNames.end())
	{
		failedToCompile(line + ", variable: " + firstVariable + " hasn't been declared");
	}
	bool secondNumIsVariable = false;
	if (std::regex_match(words[2], std::regex(sRegexVariableName)))
	{
		std::string secondVariable = words[2];
		if (mapCodeNamesToAsmNames.find(secondVariable) == mapCodeNamesToAsmNames.end())
		{
			failedToCompile(line + ", variable: " + secondVariable + " hasn't been declared");
		}
		secondNumIsVariable = true;
	}
	bool thirdNumIsVariable = false;
	if (std::regex_match(removeLastCharIfSemicolon(words[4]), std::regex(sRegexVariableName)))
	{
		std::string thirdVariable = words[4];
		if (mapCodeNamesToAsmNames.find(removeLastCharIfSemicolon(thirdVariable)) == mapCodeNamesToAsmNames.end())
		{
			failedToCompile(line + ", variable: " + thirdVariable + " hasn't been declared");
		}
		thirdNumIsVariable = true;
	}
	words[4] = removeLastCharIfSemicolon(words[4]);
	switch (words[3][0])
	{
	case '+':
		asmWriteNumAssignmentToResultOfAddition(words, secondNumIsVariable, thirdNumIsVariable);
		break;
	case '-':
		asmWriteNumAssignmentToResultOfSubtraction(words, secondNumIsVariable, thirdNumIsVariable);
		break;
	case '*':
		asmWriteNumAssignmentToResultOfMultiplication(words, secondNumIsVariable, thirdNumIsVariable);
		break;
	case '^':
		asmWriteNumAssignmentToResultOfExponentiation(words, secondNumIsVariable, thirdNumIsVariable);
		break;
	default:
		std::string failedLine = "";
		for (std::string word : words)
		{
			failedLine += word;
		}
		failedToCompile(failedLine + ", operation: " + words[3] + " not supported");
		break;
	}
}

void Compiler::handleNumAssignmentToResultOfAe(std::string line)
{
	auto words = split(line);

	std::vector<std::string> infix;
	for (int i = 2; i < words.size(); ++i)
	{
		infix.push_back(words[i]);
	}

	std::vector<std::string> postfix = getPostfixFromInfix(infix);

	std::stack<std::string> stackOfOperands;
	std::string sRegexIsOperator(sRegexOperators);
	std::string sRegexIsOperand(sRegexVariableNameOrNumLiteral);

	// Scan all characters one by one  
	for (int i = 0; i < postfix.size(); ++i)
	{
		// If operand
		if (std::regex_match(postfix[i], std::regex(sRegexIsOperand)))
		{
			stackOfOperands.push(postfix[i]);
		}
		// If operator 
		else
		{
			std::string tempVariablePrefix = "temp";
			handleTempNumDeclarationWithoutAssignment(tempVariablePrefix);
			std::string newTempVariableName = tempVariablePrefix + std::to_string(numberOfTempNumberVariables);

			std::string newAssignmentOperationLine = "";

			std::string val1 = stackOfOperands.top();
			stackOfOperands.pop();
			std::string val2 = stackOfOperands.top();
			stackOfOperands.pop();
			if (postfix[i] == "*")
			{
				newAssignmentOperationLine = newTempVariableName + " = " + val1 + " * " + val2;
				//push(stackOfOperands, val2 * val1);
			}
			else if (postfix[i] == "^")
			{
				newAssignmentOperationLine = newTempVariableName + " = " + val1 + " ^ " + val2;
				//push(stackOfOperands, val2 / val1);
			}
			else if (postfix[i] == "+")
			{
				newAssignmentOperationLine = newTempVariableName + " = " + val1 + " + " + val2;
				//push(stackOfOperands, val2 + val1);
			}
			else if (postfix[i] == "-")
			{
				newAssignmentOperationLine = newTempVariableName + " = " + val1 + " - " + val2;
				//push(stackOfOperands, val2 - val1);
			}
			handleNumAssignmentToResultOfOperation(newAssignmentOperationLine);
			stackOfOperands.push(newTempVariableName);
		}
	}

	std::string assignmentLine = words[0] + " = " + stackOfOperands.top();
	handleNumAssignment(assignmentLine);
}

void Compiler::handleArrayDeclaration(std::string line)
{
	auto words = split(line, "[],; ");
	std::string declaredWord = words[1];
	if (isRedeclaration(declaredWord))
	{
		return;
	}

	std::vector<std::string> rangeWords = BobballArray::getBbRanges(words);
	std::shared_ptr<BobballArray> newBbArray = std::make_shared<BobballArray>(rangeWords);
	auto arrayRanges = newBbArray->getRanges();
	asmWriteArrayDeclaration(declaredWord, newBbArray);
}

void Compiler::handleArrayAssignment(std::string line)
{
	auto words = split(line, "[,]; ");
	std::string variableName = words[0];
	if (mapCodeNamesToAsmNames.find(variableName) == mapCodeNamesToAsmNames.end())
	{
		failedToCompile(line + ", variable: " + variableName + " hasn't been declared");
	}
	if (mapArrayNamesToArrays.find(variableName) == mapArrayNamesToArrays.end())
	{
		failedToCompile(line + ", variable: " + variableName + " isn't an array");
	}

	std::vector<int> indices;
	for (unsigned int i = 1; i < words.size() - 2; i++)
	{
		indices.push_back(std::stoi(words[i]));
	}

	auto currentArray = mapArrayNamesToArrays[variableName];
	if (indices.size() != currentArray->getNumberOfDimensions())
	{
		failedToCompile(line + ", Incorrect number of indices");
	}

	std::string numToAssign = words[words.size() - 1];

	asmWriteArrayAssignment(variableName, indices, numToAssign);
}

void Compiler::handleWriteNum(std::string sNum)
{
	bool isVariable = false;
	if (std::regex_match(sNum, std::regex(sRegexVariableName)))
	{
		if (mapCodeNamesToAsmNames.find(sNum) == mapCodeNamesToAsmNames.end())
		{
			failedToCompile("write " + sNum + ", variable: " + sNum + " hasn't been declared");
		}
		isVariable = true;
	}
	asmWriteNumWrite(sNum, isVariable);
}

void Compiler::handleWriteStringLiteral(std::string sLiteral)
{
	bool isVariable = false;
	if (std::regex_match(sLiteral, std::regex(sRegexVariableName)))
	{
		if (mapCodeNamesToAsmNames.find(sLiteral) == mapCodeNamesToAsmNames.end())
		{
			failedToCompile("write " + sLiteral + ", variable: " + sLiteral + " hasn't been declared");
		}
		isVariable = true;
	}
	asmWriteStringLiteralWrite(sLiteral, isVariable);
}

void Compiler::handleWriteArrayNum(std::string line)
{
	auto words = split(line, "[,]; ");
	std::string variableName = words[1];
	if (mapCodeNamesToAsmNames.find(variableName) == mapCodeNamesToAsmNames.end())
	{
		failedToCompile(line + ", variable: " + variableName + " hasn't been declared");
	}
	if (mapArrayNamesToArrays.find(variableName) == mapArrayNamesToArrays.end())
	{
		failedToCompile(line + ", variable: " + variableName + " isn't an array");
	}

	std::vector<int> indices;
	for (unsigned int i = 2; i < words.size(); i++)
	{
		indices.push_back(std::stoi(words[i]));
	}

	auto currentArray = mapArrayNamesToArrays[variableName];
	if (indices.size() != currentArray->getNumberOfDimensions())
	{
		failedToCompile(line + ", Incorrect number of indices");
	}

	asmWriteArrayNumWrite(variableName, indices);
}
#pragma endregion

#pragma region OtherFunctions
bool Compiler::isRedeclaration(std::string word)
{
	if (mapCodeNamesToAsmNames.find(word) != mapCodeNamesToAsmNames.end())
	{
		failedToCompile(word + " redeclaration");
		return true;
	}
	return false;
}

void Compiler::failedToCompile(std::string strWhereFailed = "")
{
	errorsDuringCompilation = true;
	std::cout << "Error when compiling: " << strWhereFailed << std::endl;
	exit(1);
}

std::string Compiler::removeLastCharIfSemicolon(std::string word)
{
	if (std::regex_match(word, std::regex(sRegexEndsInSemicolon)))
	{
		return word.substr(0, word.size() - 1);
	}
	return word;
}

std::vector<std::string> Compiler::buildCompiledText()
{
	compiledText.push_back(sExports);
	compiledText.push_back(sImports);
	compiledText.push_back(sInitializedData);
	compiledText.push_back(sUninitializedData);
	compiledText.push_back(sCompiledMain);
	compiledText.push_back(sCompiledExit);
	return compiledText;
}

std::string Compiler::removeParentheses(std::string line)
{
	std::string leftParen = "( ";
	auto itr = line.find(leftParen);
	while (itr != std::string::npos)
	{
		line.erase(itr, leftParen.length());
		itr = line.find(leftParen);
	}

	std::string rightParen = ") ";
	itr = line.find(rightParen);
	while (itr != std::string::npos)
	{
		line.erase(itr, rightParen.length());
		itr = line.find(rightParen);
	}

	leftParen = " (";
	itr = line.find(leftParen);
	while (itr != std::string::npos)
	{
		line.erase(itr, leftParen.length());
		itr = line.find(leftParen);
	}

	rightParen = " )";
	itr = line.find(rightParen);
	while (itr != std::string::npos)
	{
		line.erase(itr, rightParen.length());
		itr = line.find(rightParen);
	}

	return line;
}

int Compiler::getPriority(std::string sOperator, bool fromInput)
{
	if (sOperator == "-" || sOperator == "+")
	{
		return 1;
	}
	else if (sOperator == "*" || sOperator == "/")
	{
		return 2;
	}
	else if (sOperator == "^")
	{
		return 3;
	}
	else if (sOperator == ")")
	{
		return 4;
	}
	else if (sOperator == "(" && fromInput)
	{
		return 4;
	}
	return 0;
}

std::vector<std::string> Compiler::getPostfixFromInfix(std::vector<std::string>& infix)
{
	infix.insert(infix.begin(), "(");
	infix.push_back(")");

	std::string sRegexIsOperator(sRegexOperators);
	std::string sRegexIsOperand(sRegexVariableNameOrNumLiteral);

	std::stack<std::string> elementStack;
	std::vector<std::string> postfix;
	//int l = infix.size();
	
	for (int i = 0; i < infix.size(); ++i) {
		// If operand add to postfix
		if (std::regex_match(infix[i], std::regex(sRegexIsOperand)))
		{
			postfix.push_back(infix[i]);
		}
		// If ‘(‘, push it to the stackOfOperands 
		else if (infix[i] == "(")
		{
			elementStack.push("(");
		}
		// If ‘)’, pop and take from stackOfOperands  
		// until an ‘(‘ is encountered. 
		else if (infix[i] == ")")
		{
			while (elementStack.top() != "(")
			{
				postfix.push_back(elementStack.top());
				elementStack.pop();
			}
			// Remove '(' from the stackOfOperands 
			elementStack.pop();
		}
		// Operator found  
		else
		{
			if (elementStack.empty() || getPriority(infix[i], true) > getPriority(elementStack.top()))
			{
				elementStack.push(infix[i]);
			}
			else
			{
				while (elementStack.empty() == false && getPriority(infix[i], true) <= getPriority(elementStack.top()))
				{
					postfix.push_back(elementStack.top());
					elementStack.pop();
				}

				// Push current Operator on stackOfOperands 
				elementStack.push(infix[i]);
			}
		}
	}
	//Pop all the remaining elements from the stackOfOperands 
	while (elementStack.empty() == false)
	{
		postfix.push_back(elementStack.top());
		elementStack.pop();
	}
	return postfix;
}

std::vector<std::string> Compiler::split(std::string str, std::string delimiters)
{
	std::vector<std::string> words;
	std::string word = "";
	for (auto charInStr : str)
	{
		bool foundDelimiter = false;
		for (auto delimiter : delimiters)
		{
			if (charInStr == delimiter)
			{
				foundDelimiter = true;
				if (word.size() > 0)
				{
					words.push_back(word);
				}
				if (delimiter == '=' || delimiter == '+' || delimiter == '-' || delimiter == '^' || delimiter == '(' || delimiter == ')')
				{
					std::string newWord(1, delimiter);
					words.push_back(newWord);
				}
				word = "";
				continue;
			}
		}
		if (!foundDelimiter)
		{
			word = word + charInStr;
		}
	}
	if (word.size() > 0)
	{
		words.push_back(word);
	}
	return words;
}

std::vector<std::string> Compiler::compileLines(std::vector<std::string>& lines)
{
	std::cout << std::endl;
	nextLineNumberToCompile = 0;
	handleProgram(lines);
	handleBegin(lines);

	for (unsigned int i = nextLineNumberToCompile; i < lines.size(); i++)
	{
		int temp = 0;
		//end. line
		if (std::regex_match(lines[i], std::regex(sRegexEndLine)))
		{
			break;
		}
		//Num declaration then assignment then operation line
		else if (std::regex_match(lines[i], std::regex(sRegexNumDeclarationThenAssignmentThenOperation)))
		{
			handleDeclarationThenAssignmentThenOperation(lines[i]);
		}
		//Num declaration then assignment(variable or literal) line
		else if (std::regex_match(lines[i], std::regex(sRegexNumDeclarationThenAssignmentToVariable)))
		{
			auto words = split(lines[i]);
			handleNumDeclarationWithAssignmentToVariable(words[1], removeLastCharIfSemicolon(words[3]));
		}
		//Num declaration then assignment(variable or literal) line
		else if (std::regex_match(lines[i], std::regex(sRegexNumDeclarationThenAssignmentToLiteral)))
		{
			auto words = split(lines[i]);
			handleNumDeclarationWithAssignmentToLiteral(words[1], removeLastCharIfSemicolon(words[3]));
		}
		//Num declaration line
		else if (std::regex_match(lines[i], std::regex(sRegexNumDeclaration)))
		{
			handleNumDeclarationWithoutAssignment(removeLastCharIfSemicolon(split(lines[i])[1]));
		}
		else if (std::regex_match(lines[i], std::regex(sRegexNumAssignment)))
		{
			handleNumAssignment(lines[i]);
		}
		//num assignment(variable or literal) then operation(variable or literal) line
		else if (std::regex_match(lines[i], std::regex(sRegexAssignmentThenOperation)))
		{
			handleNumAssignmentToResultOfOperation(lines[i]);
		}
		//array declaration
		else if (std::regex_match(lines[i], std::regex(sRegexArrayDeclarationStatement)))
		{
			handleArrayDeclaration(removeLastCharIfSemicolon(lines[i]));
		}
		//array assignment
		else if (std::regex_match(lines[i], std::regex(sRegexArrayAssignmentStatement)))
		{
			handleArrayAssignment(removeLastCharIfSemicolon(lines[i]));
		}
		//write num line
		else if (std::regex_match(lines[i], std::regex(sRegexWriteNum)))
		{
			auto words = split(lines[i]);
			handleWriteNum(removeLastCharIfSemicolon(words[1]));
		}
		//write string line
		else if (std::regex_match(lines[i], std::regex(sRegexWriteStringLiteral)))
		{
			auto words = split(lines[i]);
			handleWriteStringLiteral(removeLastCharIfSemicolon(words[1]));
		}
		//write array num
		else if (std::regex_match(lines[i], std::regex(sRegexWriteArrayNum)))
		{
			handleWriteArrayNum(removeLastCharIfSemicolon(lines[i]));
		}
		//Num declaration then assignment then Ae line
		else if (std::regex_match(removeParentheses(lines[i]), std::regex(sRegexNumDeclarationThenAssignmentThenAe)))
		{
			handleDeclarationThenAssignmentThenAe(lines[i]);
		}
		//num assignment(variable or literal) then Ae(variable or literal) line
		else if (std::regex_match(removeParentheses(lines[i]), std::regex(sRegexAssignmentThenAe)))
		{
			handleNumAssignmentToResultOfAe(lines[i]);
		}
		else
		{
			failedToCompile(lines[i]);
		}
	}

	std::vector<std::string> ct = buildCompiledText();
	/*std::cout << std::endl;
	for (std::string ctLine : ct)
	{
		std::cout << ctLine << std::endl;
	}*/

	if (errorsDuringCompilation)
	{
		std::cout << "ERRORS DURING COMPILATION" << std::endl;
		return std::vector<std::string>{ std::string("") };
	}
	std::cout << "Done compiling: " + programName + ".asm" << std::endl;

	return ct;
}
#pragma endregion
