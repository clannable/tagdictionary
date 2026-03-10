#include "tagtreeitem.h"
#include "jsonnode.h"
#include <QFileInfo>

using json = nlohmann::json;

TagTreeItem::TagTreeItem(JsonNode *node) :
    QTreeWidgetItem()
{
    this->node = node;

    if (this->node == nullptr) return;
    setText(0, QString::fromStdString(this->node->getKey()));
    setData(0, Qt::UserRole, QString::fromStdString(node->getFullPath()));

    nlohmann::json json = this->node->getData();

    QString icon = QString::fromStdString(json.value("icon", ""));
    if (QFileInfo::exists(icon) || icon.startsWith(":/icons/")) {
        setIcon(0, QIcon(icon));
    } else {
        setIcon(0, QIcon(":/icons/" + icon));
    }
}

JsonNode* TagTreeItem::getNode() const {
    return node;
}

json TagTreeItem::getJson() const {
    return this->node->getData();
}

void TagTreeItem::setNode(JsonNode *node) {
    this->node = node;
    setData(0, Qt::UserRole, QString::fromStdString(node->getFullPath()));
}

void TagTreeItem::setKey(QString str) {
    if (node != nullptr) {
        node->setKey(str.toStdString());
        setData(0, Qt::UserRole, QString::fromStdString(node->getFullPath()));
    }
    setText(0, str);
}

void TagTreeItem::jsonUpdated() {
    nlohmann::json json = node->getData();

    setText(0, QString::fromStdString(node->getKey()));
    QString icon = QString::fromStdString(json.value("icon", ""));
    if (QFileInfo::exists(icon) || icon.startsWith(":/icons/")) {
        setIcon(0, QIcon(icon));
    } else {
        setIcon(0, QIcon(":/icons/" + icon));
    }
    setData(0, Qt::UserRole, QString::fromStdString(node->getFullPath()));
}


