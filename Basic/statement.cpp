/*
 * File: statement.cpp
 * -------------------
 * This file implements the constructor and destructor for
 * the Statement class itself.  Your implementation must do
 * the same for the subclasses you define for each of the
 * BASIC statements.
 */

#include "statement.hpp"


/* Implementation of the Statement class */


Statement::Statement() = default;

Statement::~Statement() = default;

Statement *createStatement(TokenScanner &scanner);
Statement *parseStatement(std::string line);

/* Implementation of the Statement subclasses */

// REM Statement
RemStatement::RemStatement() {}

RemStatement::RemStatement(TokenScanner &scanner) {}

void RemStatement::execute(EvalState &state, Program &program) {
    // REM does nothing
}

// LET Statement
LetStatement::LetStatement() : exp(nullptr) {}

LetStatement::LetStatement(TokenScanner &scanner) : exp(nullptr) {
    std::string var = scanner.nextToken();
    if (!scanner.hasMoreTokens()) {
        error("Invalid LET statement");
    }
    std::string equalSign = scanner.nextToken();
    if (equalSign != "=") {
        error("Invalid LET statement");
    }
    if (!scanner.hasMoreTokens()) {
        error("Invalid LET statement");
    }
    exp = parseExp(scanner);
    varName = var;
}

LetStatement::~LetStatement() {
    delete exp;
}

void LetStatement::execute(EvalState &state, Program &program) {
    int value = exp->eval(state);
    state.setValue(varName, value);
}

// PRINT Statement
PrintStatement::PrintStatement() : exp(nullptr) {}

PrintStatement::PrintStatement(TokenScanner &scanner) : exp(nullptr) {
    if (!scanner.hasMoreTokens()) {
        error("Invalid PRINT statement");
    }
    exp = parseExp(scanner);
}

PrintStatement::~PrintStatement() {
    delete exp;
}

void PrintStatement::execute(EvalState &state, Program &program) {
    int value = exp->eval(state);
    std::cout << value << std::endl;
}

// INPUT Statement
InputStatement::InputStatement() {}

InputStatement::InputStatement(TokenScanner &scanner) {
    std::string var = scanner.nextToken();
    varName = var;
}

void InputStatement::execute(EvalState &state, Program &program) {
    std::string input;
    std::getline(std::cin, input);

    // Handle empty input
    if (input.empty()) {
        error("Invalid INPUT value");
    }

    // Convert string to integer
    try {
        int value = stringToInteger(input);
        state.setValue(varName, value);
    } catch (...) {
        error("Invalid INPUT value");
    }
}

// END Statement
EndStatement::EndStatement() {}

EndStatement::EndStatement(TokenScanner &scanner) {}

void EndStatement::execute(EvalState &state, Program &program) {
    throw ErrorException("END");
}

// GOTO Statement
GotoStatement::GotoStatement() : lineNumber(-1) {}

GotoStatement::GotoStatement(TokenScanner &scanner) : lineNumber(-1) {
    std::string lineStr = scanner.nextToken();
    lineNumber = stringToInteger(lineStr);
}

void GotoStatement::execute(EvalState &state, Program &program) {
    if (program.getSourceLine(lineNumber).empty()) {
        error("Invalid GOTO target line number");
    }
    throw ErrorException("GOTO " + std::to_string(lineNumber));
}

// IF Statement
IfStatement::IfStatement() : exp1(nullptr), exp2(nullptr), lineNumber(-1) {}

IfStatement::IfStatement(TokenScanner &scanner) : exp1(nullptr), exp2(nullptr), lineNumber(-1) {
    exp1 = readE(scanner);
    if (!scanner.hasMoreTokens()) {
        error("Invalid IF statement");
    }
    op = scanner.nextToken();
    if (op != "=" && op != "<" && op != ">") {
        error("Invalid IF statement operator");
    }
    exp2 = readE(scanner);

    std::string thenToken = scanner.nextToken();
    if (thenToken != "THEN") {
        error("Invalid IF statement - missing THEN");
    }

    std::string lineStr = scanner.nextToken();
    lineNumber = stringToInteger(lineStr);
}

IfStatement::~IfStatement() {
    delete exp1;
    delete exp2;
}

void IfStatement::execute(EvalState &state, Program &program) {
    int leftValue = exp1->eval(state);
    int rightValue = exp2->eval(state);

    bool condition = false;
    if (op == "=") {
        condition = (leftValue == rightValue);
    } else if (op == "<") {
        condition = (leftValue < rightValue);
    } else if (op == ">") {
        condition = (leftValue > rightValue);
    }

    if (condition) {
        if (program.getSourceLine(lineNumber).empty()) {
            error("Invalid IF target line number");
        }
        throw ErrorException("GOTO " + std::to_string(lineNumber));
    }
}

// Helper function to create statements
Statement *createStatement(TokenScanner &scanner) {
    std::string token = scanner.nextToken();

    if (token == "REM") {
        return new RemStatement(scanner);
    } else if (token == "LET") {
        return new LetStatement(scanner);
    } else if (token == "PRINT") {
        return new PrintStatement(scanner);
    } else if (token == "INPUT") {
        return new InputStatement(scanner);
    } else if (token == "END") {
        return new EndStatement(scanner);
    } else if (token == "GOTO") {
        return new GotoStatement(scanner);
    } else if (token == "IF") {
        return new IfStatement(scanner);
    } else {
        error("Invalid statement type: " + token);
        return nullptr;
    }
}
