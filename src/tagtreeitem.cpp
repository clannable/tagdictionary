#include "tagtreeitem.h"
#include "./jsonnode.h"

using json = nlohmann::json;

TagTreeItem::TagTreeItem(JsonNode *node) :
    QTreeWidgetItem()
{
    this->node = node;

    if (this->node == nullptr) return;
    setText(0, QString::fromStdString(this->node->getKey()));

    nlohmann::json json = this->node->getData();

    if (json.contains("icon"))
        setIcon(0, QIcon(QString::fromStdString(":/icons/" + json["icon"].get<std::string>())));
}

JsonNode* TagTreeItem::getNode() const {
    return node;
}

json TagTreeItem::getJson() const {
    return this->node->getData();
}

void TagTreeItem::setNode(JsonNode *node) {
    this->node = node;
}

void TagTreeItem::setKey(QString str) {
    if (node != nullptr) {
        node->setKey(str.toStdString());
    }
    setText(0, str);
}

void TagTreeItem::jsonUpdated() {
    nlohmann::json json = node->getData();

    setText(0, QString::fromStdString(node->getKey()));
    if (json.contains("icon"))
        setIcon(0, QIcon(QString::fromStdString(json["icon"].get<std::string>())));
}


