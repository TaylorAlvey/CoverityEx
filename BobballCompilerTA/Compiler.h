#pragma once
#pragma region includes
#include "BobballArray.h"
#include <map>
#include <memory>
#include <string>
#include <vector>
#pragma endregion

#pragma region CompilerClass
class Compiler
{
private:
	std::string programName = "defaultProgramName";
	std::vector<std::string> listVariables;
	int numberOfNumberVariables = 0;
	int numberOfTempNumberVariables = 0;
	int numberOfStringVariables = 0;
	int numberOfArrayVariables = 0;
	int numberOfExponentiationLoops = 0;
	bool errorsDuringCompilation = false;
	std::map<std::string, std::string> mapCodeNamesToAsmNames;
	std::map<std::string, std::shared_ptr<BobballArray>> mapArrayNamesToArrays;

	int nextLineNumberToCompile = 0;
	bool stringPrinterCreated = false;
	bool numberPrinterCreated = false;

#pragma region Regex
	static std::string sRegexVariableName;
	static std::string sRegexNumLiteral;
	static std::string sRegexFloatLiteral;
	static std::string sRegexVariableNameOrNumLiteral;
	static std::string sRegexOperators;
	static std::string sRegexZeroOrMoreWhiteSpace;
	static std::string sRegexEndsInSemicolon;
	static std::string sRegexAe;

	static std::string sRegexProgramLine;
	static std::string sRegexBeginLine;
	static std::string sRegexRawEndLine;
	static std::string sRegexEndLine;

	static std::string sRegexNumDeclaration;
	static std::string sRegexNumDeclarationThenAssignmentToVariable;
	static std::string sRegexNumDeclarationThenAssignmentToLiteral;
	static std::string sRegexNumDeclarationThenAssignmentThenOperation;
	static std::string sRegexNumDeclarationThenAssignmentThenAe;

	static std::string sRegexNumAssignment;
	static std::string sRegexAssignmentThenOperation;
	static std::string sRegexAssignmentThenAe;

	static std::string sRegexArrayRange;
	static std::string sRegexArrayDeclarationStatement;
	static std::string sRegexArraySpecificIndex;
	static std::string sRegexArrayAssignmentStatement;

	static std::string sRegexWriteNum;
	static std::string sRegexWriteRaw;
	static std::string sRegexWriteStringLiteral;
	static std::string sRegexWriteArrayNum;
#pragma endregion

#pragma region PrivateFunctions
	std::string buildAsmNumVariableName(std::string& declaredWord);
	std::string buildAsmTempNumVariableName(std::string& declaredWord);
	std::string buildAsmStringLiteralVariableName();
	std::string buildAsmArrayVariableName(std::string& declaredWord);
	void asmWriteNumDirectAssignmentToVariable(std::string declaredWord, std::string assignedVariable);
	void asmWriteNumDeclarationWithAssignmentToLiteral(std::string declaredWord, std::string assignedLiteral);
	void asmWriteNumDeclarationWithoutAssignment(std::string word);
	void asmWriteTempNumDeclarationWithoutAssignment(std::string word);
	void asmWriteNumAssignment(std::vector<std::string> words, bool secondNumIsVariable);
	void asmWriteNumAssignmentToResultOfAddition(std::vector<std::string> words, bool secondNumIsVariable, bool thirdNumIsVariable);
	void asmWriteNumAssignmentToResultOfSubtraction(std::vector<std::string> words, bool secondNumIsVariable, bool thirdNumIsVariable);
	void asmWriteNumAssignmentToResultOfMultiplication(std::vector<std::string> words, bool secondNumIsVariable, bool thirdNumIsVariable);
	void asmWriteNumAssignmentToResultOfExponentiation(std::vector<std::string> words, bool secondNumIsVariable, bool thirdNumIsVariable);
	void asmWriteArrayDeclaration(std::string declaredWord, std::shared_ptr<BobballArray> newArray);
	void asmWriteArrayAssignment(std::string variableName, std::vector<int> indices, std::string numToAssign);
	void asmWriteNumberPrinter();
	void asmWriteStringPrinter();
	void asmWriteInitializedStringVariable(std::string sLiteral);
	void asmWriteNumWrite(std::string sNum, bool isVariable);
	void asmWriteStringLiteralWrite(std::string sLiteral, bool isVariable);
	void asmWriteArrayNumWrite(std::string variableName, std::vector<int> indices);
	void asmWrite(std::string wordToPrint, std::string printer);

	void handleProgram(std::vector<std::string>& lines);
	void handleBegin(std::vector<std::string>& lines);
	void handleDeclarationThenAssignmentThenOperation(std::string line);
	void handleDeclarationThenAssignmentThenAe(std::string line);
	void handleNumDeclarationWithAssignmentToVariable(std::string declaredWord, std::string assignedVariable);
	void handleNumDeclarationWithAssignmentToLiteral(std::string declaredWord, std::string assignedLiteral);
	void handleNumDeclarationWithoutAssignment(std::string lines);
	void handleTempNumDeclarationWithoutAssignment(std::string word);
	void handleNumAssignment(std::string line);
	void handleNumAssignmentToResultOfOperation(std::string line);
	void handleNumAssignmentToResultOfAe(std::string line);
	void handleArrayDeclaration(std::string line);
	void handleArrayAssignment(std::string line);
	void handleWriteNum(std::string sNum);
	void handleWriteStringLiteral(std::string sLiteral);
	void handleWriteArrayNum(std::string line);

	bool isRedeclaration(std::string word);
	void failedToCompile(std::string strWhereFailed);
	std::string removeLastCharIfSemicolon(std::string word);
	std::vector<std::string> buildCompiledText();
	std::string removeParentheses(std::string line);
	int getPriority(std::string sOperator, bool fromInput = false);
	std::vector<std::string> getPostfixFromInfix(std::vector<std::string>& infix);
#pragma endregion

#pragma region AsmHeaders
	std::string sExports = ";-----------------------------\n; exports\n; ----------------------------\nglobal _main\nEXPORT _main\n";
	std::string sImports = ";-----------------------------\n; imports\n; ----------------------------\nextern _printf\nextern _ExitProcess@4\n";
	std::string sInitializedData = ";-----------------------------\n; initialized data\n; ----------------------------\nsection .data USE32\n\n";
	std::string sUninitializedData = "; ----------------------------\n; uninitialized data\n; ----------------------------\nsection .bss USE32\n\n";
	std::string sCompiledMain = "; ----------------------------\n; Code!(execution starts at _main)\n; ----------------------------\nsection .code USE32\n\n_main :\n\n";
	std::string sCompiledExit = "_exit:\n\n\tmov eax, 0x0\n\tcall	_ExitProcess@4\n\n; (eof)\n";
	std::vector<std::string> compiledText;
#pragma endregion

public:
#pragma region PublicFunctions
	std::string getProgramName() { return programName; }
	static std::vector<std::string> split(std::string str, std::string delimiters = "+^=;\t\n\r ");
	std::vector<std::string> compileLines(std::vector<std::string>& linesFromFile);
	void testRegex();
#pragma endregion
};
#pragma endregion
