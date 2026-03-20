#include "tagtreeitem.h"
#include "tagnode.h"
#include <QFileInfo>

using json = nlohmann::json;

TagTreeItem::TagTreeItem(TagNode *node) : QTreeWidgetItem()
{
    this->node = node;

    if (this->node == nullptr) return;

    setText(0, QString::fromStdString(this->node->getKey()));
    setData(0, Qt::UserRole, QString::fromStdString(node->getFullPath()));

    QString icon = QString::fromStdString(this->node->getIcon());
    if (QFileInfo::exists(icon) || icon.startsWith(":/icons/"))
        setIcon(0, QIcon(icon));
    else
        setIcon(0, QIcon(":/icons/" + icon));
}

TagNode* TagTreeItem::getNode() const {
    return node;
}

void TagTreeItem::setNode(TagNode *node) {
    this->node = node;
}

void TagTreeItem::setKey(QString str) {
    if (node != nullptr)
        node->setKey(str.toStdString());
    setText(0, str);
}

void TagTreeItem::jsonUpdated() {
    setText(0, QString::fromStdString(node->getKey()));
    QString icon = QString::fromStdString(node->getIcon());
    if (QFileInfo::exists(icon) || icon.startsWith(":/icons/"))
        setIcon(0, QIcon(icon));
    else
        setIcon(0, QIcon(":/icons/" + icon));

}


