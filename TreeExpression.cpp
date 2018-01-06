//
// Created by bokoch on 12/27/17.
//

#include "TreeExpression.h"

using namespace std;

Expression::Expression(string& expression) {
    this->expression = expression;
    srand(time(NULL));
}

bool Expression::isLetter(char symbol) {
    return (symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z') || symbol == '_';
}

bool Expression::isDigit(char symbol) {
    return symbol >= '0' && symbol <= '9';
}

bool Expression::isLetterOrDigit(char symbol) {
    return isLetter(symbol) || isDigit(symbol);
}

bool Expression::isOp(LexemeT lexT) {
    return lexT == LexemeT::op_minus ||	lexT == LexemeT::op_plus ||	lexT == LexemeT::op_mult ||	lexT == LexemeT::op_div;
}

// возвращает истину, если необходимо сдвинуть операцию в стеке полиза
bool Expression::needPop(LexemeT lexT1, LexemeT lexT2) {
    return ((lexT1 == LexemeT::op_plus || lexT1 == LexemeT::op_minus) && isOp(lexT2)) ||
           ((lexT1 == LexemeT::op_mult || lexT1 == LexemeT::op_div) && (lexT2 == LexemeT::op_mult || lexT2 == LexemeT::op_div));
}

// возвращает истину, если строка num является числом со знаком
bool Expression::isNum(const std::string& num) {
    int i = 0;

    while (num[i]) {
        if (!isDigit(num[i]) && num[i] != '.' && num[i] != '-')
            return false;

        i++;
    }

    return true;
}

// Формирует массив лексем
void Expression::getLexemes() {
    lexemes.push_back(Lexeme(LexemeT::start, ""));

    int i = 0;
    int brackets = 0;

    while (expression[i]) {
        if (isDigit(expression[i])) {
            // получаем числовую лексему
            string lexeme = "";
            int pointN = 0;

            while (expression[i] && (isDigit(expression[i]) || expression[i] == '.')) {
                lexeme += expression[i];

                if (expression[i] == '.')
                    pointN++;

                if (pointN > 1)
                    throw "A real number contains at most one point in the record";

                i++;
            }

            lexemes.push_back(Lexeme(LexemeT::number, lexeme));
        }
        else if (isLetter(expression[i])) {
            // получаем лексему переменную
            string lexeme = "";

            while (expression[i] && isLetterOrDigit(expression[i])) {
                lexeme += expression[i];

                i++;
            }

            lexemes.push_back(Lexeme(LexemeT::ident, lexeme));
        }
        else {
            // парсим на лексемы символы операций и скобок, с проверкой сбалансированности последних
            // пробельные символы пропускаем, а "неожиданные" означают ошибку в выражении
            switch (expression[i]) {
                case '+':
                    lexemes.push_back(Lexeme(LexemeT::op_plus, "+"));
                    break;
                case '-':
                    lexemes.push_back(Lexeme(LexemeT::op_minus, "-"));
                    break;
                case '*':
                    lexemes.push_back(Lexeme(LexemeT::op_mult, "*"));
                    break;
                case '/':
                    lexemes.push_back(Lexeme(LexemeT::op_div, "/"));
                    break;
                case '(':
                    lexemes.push_back(Lexeme(LexemeT::bracket_l, "("));
                    brackets++;
                    break;

                case ')':
                    lexemes.push_back(Lexeme(LexemeT::bracket_r, ")"));
                    brackets--;
                    break;

                case ' ':
                    break;

                default:
                    throw "undefined symbol in expression";
            }

            if (brackets < 0)
                throw "brackets are disbalanced";

            i++;
        }
    }

    if (brackets)
        throw "brackets are disbalanced";

    lexemes.push_back(Lexeme(LexemeT::end, ""));
}

// Проверка выражения на корректность
bool Expression::correct() {
    try {
        getLexemes();

        //setIdentifiers();

        // унарный минус
        // если встретилась последовательность '(' '-' 'number' или выражение начинается с '-' 'number',
        // то удаляем лексему минуса и изменяем знак числа на противоположный

        size_t index = 0;

        while (index < lexemes.size() - 2) {
            LexemeT t1 = lexemes[index].type;
            LexemeT t2 = lexemes[index + 1].type;
            LexemeT t3 = lexemes[index + 2].type;

            if ((t1 == LexemeT::start || t1 == LexemeT::bracket_l) && t2 == LexemeT::op_minus && t3 == LexemeT::number) {
                if (lexemes[index + 2].value[0] != '-')
                    lexemes[index + 2].value = string("-") + lexemes[index + 2].value;
                else
                    lexemes[index + 2].value.erase(0, 1);

                lexemes.erase(lexemes.begin() + index + 1);
            }

            index++;
        }

        // проверка на корректность самого выражения
        size_t len = lexemes.size();
        index = 1;

        while (index < len) {
            Lexeme prev = lexemes[index - 1];
            Lexeme lex = lexemes[index];

            if (prev.type == LexemeT::start && lex.type != LexemeT::number && lex.type != LexemeT::bracket_l)
                throw "Expression must begin with a number, variable or '('";

            if (lex.type == LexemeT::end && prev.type != LexemeT::number && prev.type != LexemeT::bracket_r)
                throw "Expression must end with a number, variable or ')'";

            if (isOp(prev.type) && lex.type != LexemeT::number && lex.type != LexemeT::bracket_l)
                throw "After operation can be only number, variable or '('";

            if (prev.type == LexemeT::bracket_l && lex.type != LexemeT::bracket_l && lex.type != LexemeT::number)
                throw "After '(' can be only '(', variable or number";

            if (prev.type == LexemeT::number && !isOp(lex.type) && lex.type != LexemeT::bracket_r && lex.type != LexemeT::end)
                throw "After number or variable can be only operation or ')'";

            index++;
        }
    }
    catch (const char* e) {
        cout << "Lexical error: " << e << endl;
        return false;
    }

    getRPN();

    return true;
}

// получение польской инверсной записи из лексем при помощи стека
void Expression::getRPN() {
    stack<Lexeme>stack;

    size_t size = lexemes.size();

    for (size_t i = 0; i < size; i++) {
        Lexeme lexeme = lexemes[i];

        switch (lexeme.type) {
            case LexemeT::number:
                rpn.push_back(lexeme);
                break;

            case LexemeT::bracket_l:
                stack.push(lexeme);
                break;

            case LexemeT::bracket_r:
                while (!stack.empty() && stack.top().type != LexemeT::bracket_l) {
                    rpn.push_back(stack.top());
                    stack.pop();
                }

                if (!stack.empty())
                    stack.pop();
                break;

            case LexemeT::op_plus:
            case LexemeT::op_minus:
            case LexemeT::op_mult:
            case LexemeT::op_div:
                if (!stack.empty()) {
                    Lexeme lex = stack.top();

                    while (!stack.empty() && needPop(lexeme.type, lex.type)) {

                        rpn.push_back(lex);
                        stack.pop();

                        if (!stack.empty())
                            lex = stack.top();
                    }
                }

                stack.push(lexeme);
                break;

            default:
                break;
        }
    }

    while (!stack.empty()) {
        rpn.push_back(stack.top());
        stack.pop();
    }
}

// процедура задания значений используемых переменных
void Expression::setIdentifiers() {
    size_t index = 0;

    // проходимся по всем лексемам
    while (index < lexemes.size()) {
        // если переменная, то задаём ей значение и присваиваем его всем таким же переменным в выражении
        if (lexemes[index].type == LexemeT::ident) {
            string identName = lexemes[index].value;

            cout << "Enter value of variable " << identName;
            double val;

            cout << "\nDo you want to set '" << identName << "'' by random? (y/n) ";
            string answer;
            getline(cin, answer);
            fflush(stdin);

            if (answer == "y") {
                val = (rand() % 1000 - 500) / 10.0;
                cout << val << endl;
            }
            else {
                cout << identName << ": ";
                getline(cin, answer);

                while (!isNum(answer)) {
                    cout << "Incorrect input. Try again: ";
                    getline(cin, answer);
                }

                val = atof(answer.c_str());
            }

            char res[50];
            sprintf(res, "%lf", val);

            // присваиваем полученное значение всем переменным с именем identName, заменяя тип на "число"
            for (size_t i = index; i < lexemes.size(); i++) {
                if (lexemes[i].type == LexemeT::ident && lexemes[i].value == identName) {
                    lexemes[i].type = LexemeT::number;
                    lexemes[i].value = res;
                }
            }
        }

        index++;
    }
}

// создание дерева по польской записи
Tree* Expression::makeTree(int &index) {
    Tree* tree = new Tree;
    tree->lex = rpn[index].value;

    if (index >= 0) {
        if (isOp(rpn[index].type)) {
            tree->right = makeTree(--index);
            tree->left = makeTree(--index);
        }
        else {
            tree->left = NULL;
            tree->right = NULL;
        }
    }

    return tree;
}

Tree* Expression::createTree() {
    int n = rpn.size() - 1;

    return makeTree(n);
}

void Expression::printTree(Tree* tree) {
    if (tree) {
        printTree(tree->left);
        cout << "[" << tree->lex << "] ";
        printTree(tree->right);
    }
}

// вычисление корректного дерева выражения
double Expression::calculate(Tree** tree) {
    if (!(*tree)->left) {
        double value = atof((*tree)->lex.c_str());
        //delete (*tree);
        return value;
    }

    double arg1 = calculate(&(*tree)->left);
    double arg2 = calculate(&(*tree)->right);
    double res;

    if ((*tree)->lex == "+")
        res = arg1 + arg2;
    else if ((*tree)->lex == "-")
        res = arg1 - arg2;
    else if ((*tree)->lex == "*")
        res = arg1 * arg2;
    else if (arg2)
        res = arg1 / arg2;
    else
        throw "Division by zero";

    char buf[50];
    sprintf(buf, "%lf", res);

    (*tree)->lex = buf;
    //delete (*tree);

    return res;
}

// удаление дерева
void Expression::deleteTree(Tree** tree) {
    if (*tree){
        deleteTree(&(*tree)->left);
        deleteTree(&(*tree)->right);
        delete(*tree);
    }
}

void Expression::ToDot(Tree *tree) {
    ofstream fileOut("./default.dot");
    fileOut << "digraph G {" << std::endl;
    BuildShapeDot(tree, fileOut, 1);
    fileOut << "}" << std::endl;
    fileOut.close();
}

void Expression::BuildShapeDot(Tree *tree, std::ofstream& fout, size_t ID) {
    fout << "\tNode" << ID << "[label=\"" << tree->lex << "\"];" << std::endl;
    if (!tree->left && !tree->right) {
        return;
    }
    BuildShapeDot(tree->left, fout, 2*ID);
    BuildShapeDot(tree->right, fout, 2*ID+1);
    fout << "\tNode" << ID << " -> Node" << 2*ID <</* "[dir=none];" <<*/ std::endl;
    fout << "\tNode" << ID << " -> Node" << 2*ID+1 <</* "[dir=none];" <<*/ std::endl;
}