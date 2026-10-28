#include "tokenparser.h"

#include "globals.h"
#include <QTreeWidgetItem>
#include <QFileInfo>

TokenParser::TokenParser() {}

QTreeWidgetItem* TokenParser::parseString(QString value) {
    for (const auto& match : QRegularExpression("^(\\d+([\\+\\|\\?]\\()?)|,|\\)").globalMatch(value)) {
        tokens.enqueue(Token(match.captured(0)));
    }

    return processToken(tokens.dequeue());
}

QTreeWidgetItem* TokenParser::processToken(Token token) {
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setData(1, Qt::UserRole, token.value);
    item->setData(0, Qt::UserRole, token.type);
    item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
    item->setExpanded(true);
    TagNode* tag = TAGS[token.value];
    item->setText(1, QString::fromStdString(tag->key()));
    QString icon = QString::fromStdString(tag->icon());
    if (QFileInfo::exists(icon) || icon.startsWith(":/icons/"))
        item->setIcon(1, QIcon(icon));
    else
        item->setIcon(1, QIcon(":/icons/" + icon));

    if (token.type == Token::IF_THEN)
        item->setText(0, "IF");

    if (token.type != Token::NODE) {
        Token nextToken;
        while ((nextToken = tokens.dequeue()).type != Token::END) {
            QTreeWidgetItem* child = processToken(nextToken);
            switch (token.type) {
                case Token::IF_THEN:
                    child->setIcon(0, QIcon(":/icons/linear/turn-down.png"));
                    break;
                case Token::AND:
                    child->setText(0, "AND");
                    break;
                case Token::OR:
                    child->setText(0, "OR");
                    break;
                case Token::SUB:
                    child->setText(1, child->text(1)+"*");
                    child->setData(1, Qt::UserRole+1, Token::SUB);
                default:
                    break;
            }
            item->addChild(child);
        }

    }
    return item;
}
