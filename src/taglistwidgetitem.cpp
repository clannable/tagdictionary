#include "taglistwidgetitem.h"
#include <QFileInfo>
#include "globals.h"

TagListWidgetItem::TagListWidgetItem(QVariant value, QListWidget* parent) :
    QListWidgetItem(parent),
    m_value(value)
{
    TagNode* tag = TAGS[value.toInt()];
    setData(Qt::UserRole, value.toInt());
    setText(QString::fromStdString(tag->key()));
    QString icon = QString::fromStdString(tag->icon());
    if (QFileInfo::exists(icon) || icon.startsWith(":/icons/"))
        setIcon(QIcon(icon));
    else
        setIcon(QIcon(":/icons/" + icon));
}

TagListWidgetItem::TagListWidgetItem(TagNode* tag, QListWidget* parent) :
    QListWidgetItem(parent),
    m_value(tag->id())
{

}
