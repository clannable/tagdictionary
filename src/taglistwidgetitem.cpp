#include "taglistwidgetitem.h"
#include "globals.h"

TagListWidgetItem::TagListWidgetItem(QVariant value, QListWidget* parent) :
    QListWidgetItem(parent),
    value(value)
{
    this->tag = TAG_MAP[value.toInt()];
    setData(Qt::UserRole, value.toInt());
    setText(QString::fromStdString(tag->getKey()));
}

TagListWidgetItem::TagListWidgetItem(TagNode* node, QListWidget* parent) :
    QListWidgetItem(parent),
    tag(node),
    value(node->getId())
{
    setData(Qt::UserRole, value.toInt());
    setText(QString::fromStdString(tag->getKey()));
}

QVariant TagListWidgetItem::getValue() const {
    return value;
}
