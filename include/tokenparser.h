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
    int value = 0;
    TokenType type;

    Token() {}
    Token(QString str) {
        int ix = str.length();
        if (str.endsWith("*")) {
            ix--;
            sub = true;
        }
        QChar fChar = str[0];
        if (!fChar.isDigit()) {
            value = str.sliced(1, ix-1).toInt();
            if (fChar == "?") type = TokenType::IF_THEN;
            else if (fChar == "+") type = TokenType::AND;
            else if (fChar == "|") type = TokenType::OR;
        } else {
            value = str.sliced(0, ix).toInt();
            type = TokenType::NODE;
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
