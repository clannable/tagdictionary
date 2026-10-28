#ifndef TOKENPARSER_H
#define TOKENPARSER_H

#include <QTreeWidgetItem>
#include <QRegularExpression>
#include <QQueue>

struct Token {
    enum TokenType {
        NODE,
        IF_THEN,
        AND,
        OR,
        SUB,
        END
    };

    int value;
    TokenType type;

    Token() {}
    Token(QString str) {
        if (str == ")") type = TokenType::END;
        else {
            if (str.endsWith("*")) {
                value = str.left(str.length()-1).toInt();
                type = TokenType::SUB;
            } else if (str.endsWith("(")) {
                value = str.left(str.length()-2).toInt();
                QChar bType = str[str.length()-1];
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
