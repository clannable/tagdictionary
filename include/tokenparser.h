#ifndef TOKENPARSER_H
#define TOKENPARSER_H

#include <QTreeWidgetItem>
#include <QRegularExpression>
#include <QQueue>

struct Token {
    enum TokenType {
        NODE = 0,
        IF_THEN = 1,
        AND = 2,
        OR = 3,
        END = 5
    };

    bool sub = false;
    int value;
    TokenType type;

    Token() {}
    Token(QString str) {
        if (str == ")") type = TokenType::END;
        else {
            if (str.contains("*")) {
                value = str.left(str.indexOf("*")).toInt();
                sub = true;
            } else if (str.endsWith("(")) {
                value = str.left(str.length()-2).toInt();
                QChar bType = str[str.length()-2];
                if (bType == "?") type = TokenType::IF_THEN;
                else if (bType == "+") type = TokenType::AND;
                else if (bType == "|") type = TokenType::OR;
            } else {
                value = str.toInt();
                type = TokenType::NODE;
            }
        }
    }
};

class TokenParser
{

public:
    TokenParser();
    QTreeWidgetItem* parseString(QString value);
    QTreeWidgetItem* processToken(Token token);

private:
    QQueue<Token> tokens;

    void splitTokens(QString value);
};

#endif // TOKENPARSER_H
