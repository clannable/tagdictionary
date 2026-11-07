#include "tagtreeitem.h"
#include "tagnode.h"
#include <QFileInfo>
#include <regex>

using json = nlohmann::json;

TagTreeItem::TagTreeItem(TagNode *node) : QTreeWidgetItem(), m_tag(node)
{
    if (m_tag == nullptr) return;
    refresh();
}

void TagTreeItem::refresh() {
    if (m_tag->id() <= 0) {
        QFont font = this->font(0);
        font.setBold(true);
        font.setPointSize(12);
        setFont(0, font);
        setText(0, "Tags");
        setData(0, Qt::UserRole, 0);
        setFlags(Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
        setExpanded(true);
    } else {
        setText(0, QString::fromStdString(m_tag->key()));
        setData(0, Qt::UserRole, m_tag->id());
        setIcon(0, QIcon(QString::fromStdString(m_tag->icon())));
        refreshFileIcons();
    }
}

void TagTreeItem::setTag(TagNode *tag) { m_tag = tag; }

void TagTreeItem::addChild(QTreeWidgetItem* child) {
    QTreeWidgetItem::addChild(child);
    if (dynamic_cast<TagTreeItem*>(child) != nullptr)
        m_tag->addChild(child->data(0, Qt::UserRole).toInt());
}

void TagTreeItem::removeChild(QTreeWidgetItem* child) {
    QTreeWidgetItem::removeChild(child);
    if (dynamic_cast<TagTreeItem*>(child) != nullptr)
        m_tag->removeChild(child->data(0, Qt::UserRole).toInt());
}

void TagTreeItem::refreshFileIcons() {
    bool images = false;
    bool videos = false;

    regex videoRegex("\\.(mov|mp4|wmv)$", regex_constants::icase);
    regex imageRegex("\\.(jpeg|png|gif|jpg|bmp|jfif|webp)$", regex_constants::icase);
    for (const string& file : m_tag->files()) {
        if (!videos && regex_search(file, videoRegex))
            videos = true;
        if (!images && regex_search(file, imageRegex))
            images = true;
        if (images && videos)
            break;
    }
    this->setIcon(1, QIcon(images ? ":/icons/linear/image.png" : ""));
    this->setIcon(2, QIcon(videos ? ":/icons/linear/video.png" : ""));
}

