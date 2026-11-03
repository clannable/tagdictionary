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
    return item;
}
