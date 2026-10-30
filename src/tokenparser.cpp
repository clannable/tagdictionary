#include "tokenparser.h"

#include "globals.h"
#include <QTreeWidgetItem>
#include <QFileInfo>

TokenParser::TokenParser() {}

QTreeWidgetItem* TokenParser::parseString(QString value) {
    return processToken(Token(value));
}

QTreeWidgetItem* TokenParser::processToken(Token token) {
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setData(1, Qt::UserRole, token.value);
    item->setData(0, Qt::UserRole, token.type);
    item->setFlags(item->flags() & ~Qt::ItemIsDropEnabled);
    item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

    TagNode* tag = TAGS[token.value];
    if (tag == nullptr) return nullptr;
    item->setText(1, QString::fromStdString(tag->key()));
    if (token.sub) {
        item->setText(1, item->text(1)+"*");
        item->setData(1, Qt::UserRole+1, true);
    }
    QString icon = QString::fromStdString(tag->icon());
    if (QFileInfo::exists(icon) || icon.startsWith(":/icons/"))
        item->setIcon(1, QIcon(icon));
    else
        item->setIcon(1, QIcon(":/icons/" + icon));

    if (token.type == Token::IF_THEN)
        item->setText(0, "IF");
    else if (token.type == Token::AND)
        item->setText(0, "AND");
    else if (token.type == Token::OR)
        item->setText(0, "OR");
    // if (token.type != Token::NODE) {
    //     Token nextToken;
    //     while ((nextToken = tokens.dequeue()).type != Token::END) {
    //         QTreeWidgetItem* child = processToken(nextToken);
    //         if (child == nullptr) continue;
    //         switch (token.type) {
    //             case Token::IF_THEN:
    //                 child->setIcon(0, QIcon(":/icons/linear/turn-down.png"));
    //                 break;
    //             case Token::AND:
    //                 child->setText(0, "AND");
    //                 break;
    //             case Token::OR:
    //                 child->setText(0, "OR");
    //                 break;
    //             default:
    //                 break;
    //         }
    //         item->addChild(child);
    //     }
    //     item->setExpanded(true);
    // }
    return item;
}
