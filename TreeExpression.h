//
// Created by bokoch on 12/27/17.
//
#pragma once
#ifndef PARSEEXPRESSIONTODOT_TREEEXPRESSSION_H
#define PARSEEXPRESSIONTODOT_TREEEXPRESSSION_H

#include <iostream>
#include <vector>
#include <cstdio>
#include <stack>
#include <fstream>
enum class LexemeT { start, end, number, ident, op_plus, op_minus, op_mult, op_div, bracket_l, bracket_r };
struct Lexeme {
    LexemeT type;
    std::string value;

    Lexeme(LexemeT type, std::string value) {
        this->type = type;
        this->value = value;
    }
};

struct Tree {
    std::string lex;

    Tree *left;
    Tree *right;
};

class Expression {
private:
    std::string expression;
    std::vector<Lexeme> lexemes;
    std::vector<Lexeme> rpn;

    bool isLetter(char symbol);
    bool isDigit(char symbol);
    bool isLetterOrDigit(char symbol);

    bool isOp(LexemeT lexT);
    bool needPop(LexemeT lexT1, LexemeT lexT2);

    void getLexemes();
    void getRPN();

    void setIdentifiers();

    Tree* makeTree(int &index);

    bool isNum(const std::string& num);

public:
    Expression(std::string& expression);

    bool correct();
    void ToDot(Tree *tree);
    void BuildShapeDot(Tree *tree, std::ofstream& fileOut, size_t ID);
    Tree* createTree();

    void printTree(Tree* tree);
    double calculate(Tree** tree);
    void deleteTree(Tree** tree);
};

#endif //PARSEEXPRESSIONTODOT_TREEEXPRESSSION_H
