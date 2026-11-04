#ifndef TOKENPARSER_H
#define TOKENPARSER_H

#include <QTreeWidgetItem>
#include <QRegularExpression>
#include <QQueue>

struct Token {
    enum TokenType {
        NODE = 0,
        IF = 1,
        AND = 2,
        OR = 3,
        THEN = 4,
        END = 5
    };

    bool sub = false;
    bool sep = false;
    int value = 0;
    TokenType type;

    Token() {}
    Token(QString str) {
        int ix = str.length()-1;
        if (str[ix] == "*") {
            ix--;
            sub = true;
        }
        if (str[ix] == "_") {
            ix--;
            sep = true;
        }
        QChar fChar = str[0];
        if (!fChar.isDigit()) {
            value = str.sliced(1, ix).toInt();
            if (fChar == "?") type = TokenType::IF;
            else if (fChar == "+") type = TokenType::AND;
            else if (fChar == "|") type = TokenType::OR;
            else if (fChar == ">") type = TokenType::THEN;
        } else {
            value = str.sliced(0, ix+1).toInt();
            type = TokenType::NODE;
        }
    }
};

class TokenParser
{

public:
    TokenParser();
    static QTreeWidgetItem* parseString(QString value);
};

#endif // TOKENPARSER_H
