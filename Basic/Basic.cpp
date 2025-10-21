/*
 * File: Basic.cpp
 * ---------------
 * This file is the starter project for the BASIC interpreter.
 */

#include <cctype>
#include <iostream>
#include <string>
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "Utils/error.hpp"
#include "Utils/tokenScanner.hpp"
#include "Utils/strlib.hpp"


/* Function prototypes */

void processLine(std::string line, Program &program, EvalState &state);
void executeProgram(Program &program, EvalState &state);
void listProgram(Program &program);
Statement *parseStatement(std::string line);

/* Main program */

int main() {
    EvalState state;
    Program program;
    while (true) {
        try {
            std::string input;
            getline(std::cin, input);
            if (input.empty())
                continue;
            processLine(input, program, state);
        } catch (ErrorException &ex) {
            std::cout << ex.getMessage() << std::endl;
        }
    }
    return 0;
}

/*
 * Function: processLine
 * Usage: processLine(line, program, state);
 * -----------------------------------------
 * Processes a single line entered by the user.  In this version of
 * implementation, the program reads a line, parses it as an expression,
 * and then prints the result.  In your implementation, you will
 * need to replace this method with one that can respond correctly
 * when the user enters a program line (which begins with a number)
 * or one of the BASIC commands, such as LIST or RUN.
 */

void processLine(std::string line, Program &program, EvalState &state) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);

    if (!scanner.hasMoreTokens()) {
        return;
    }

    std::string firstToken = scanner.nextToken();

    // Check if first token is a line number
    if (scanner.getTokenType(firstToken) == NUMBER) {
        int lineNumber = stringToInteger(firstToken);

        if (!scanner.hasMoreTokens()) {
            // Empty line number - remove this line
            program.removeSourceLine(lineNumber);
        } else {
            // Store the complete line
            program.addSourceLine(lineNumber, line);

            // Parse and store the statement
            Statement *stmt = parseStatement(line.substr(firstToken.length() + 1));
            program.setParsedStatement(lineNumber, stmt);
        }
        return;
    }

    // Process direct commands
    scanner.saveToken(firstToken);

    if (!scanner.hasMoreTokens()) {
        return;
    }

    std::string command = scanner.nextToken();

    if (command == "RUN") {
        executeProgram(program, state);
    } else if (command == "LIST") {
        listProgram(program);
    } else if (command == "CLEAR") {
        program.clear();
        state.Clear();
    } else if (command == "QUIT") {
        exit(0);
    } else if (command == "HELP") {
        std::cout << "BASIC Interpreter - Available commands:" << std::endl;
        std::cout << "RUN - Execute the program" << std::endl;
        std::cout << "LIST - List the program" << std::endl;
        std::cout << "CLEAR - Clear the program and variables" << std::endl;
        std::cout << "QUIT - Exit the interpreter" << std::endl;
        std::cout << "REM - Comment" << std::endl;
        std::cout << "LET var = exp - Assignment" << std::endl;
        std::cout << "PRINT exp - Print expression" << std::endl;
        std::cout << "INPUT var - Input variable" << std::endl;
        std::cout << "END - End program" << std::endl;
        std::cout << "GOTO line - Jump to line" << std::endl;
        std::cout << "IF exp1 op exp2 THEN line - Conditional jump" << std::endl;
    } else {
        // Direct execution of statements
        scanner.saveToken(command);
        Statement *stmt = parseStatement(line);
        if (stmt) {
            stmt->execute(state, program);
            delete stmt;
        }
    }
}

void executeProgram(Program &program, EvalState &state) {
    int currentLine = program.getFirstLineNumber();

    while (currentLine != -1) {
        Statement *stmt = program.getParsedStatement(currentLine);
        if (stmt == nullptr) {
            // Parse the line if not already parsed
            std::string sourceLine = program.getSourceLine(currentLine);
            if (!sourceLine.empty()) {
                // Find the first space after line number
                size_t spacePos = sourceLine.find(' ');
                if (spacePos != std::string::npos) {
                    stmt = parseStatement(sourceLine.substr(spacePos + 1));
                    program.setParsedStatement(currentLine, stmt);
                }
            }
        }

        if (stmt) {
            try {
                stmt->execute(state, program);
            } catch (ErrorException &ex) {
                std::string errorMsg = ex.getMessage();
                if (errorMsg == "END") {
                    return; // Normal program termination
                } else if (errorMsg.substr(0, 4) == "GOTO") {
                    // Extract line number from "GOTO nnn"
                    currentLine = stringToInteger(errorMsg.substr(5));
                    continue;
                } else {
                    // Re-throw other errors
                    throw ex;
                }
            }
        }

        currentLine = program.getNextLineNumber(currentLine);
    }
}

void listProgram(Program &program) {
    int currentLine = program.getFirstLineNumber();

    while (currentLine != -1) {
        std::string line = program.getSourceLine(currentLine);
        if (!line.empty()) {
            std::cout << line << std::endl;
        }
        currentLine = program.getNextLineNumber(currentLine);
    }
}

Statement *parseStatement(std::string line) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);

    if (!scanner.hasMoreTokens()) {
        return nullptr;
    }

    std::string firstToken = scanner.nextToken();

    if (firstToken == "REM") {
        return new RemStatement(scanner);
    } else if (firstToken == "LET") {
        return new LetStatement(scanner);
    } else if (firstToken == "PRINT") {
        return new PrintStatement(scanner);
    } else if (firstToken == "INPUT") {
        return new InputStatement(scanner);
    } else if (firstToken == "END") {
        return new EndStatement(scanner);
    } else if (firstToken == "GOTO") {
        return new GotoStatement(scanner);
    } else if (firstToken == "IF") {
        return new IfStatement(scanner);
    } else {
        error("Invalid statement: " + firstToken);
        return nullptr;
    }
}

