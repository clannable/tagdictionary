#include "tagtreeitem.h"
#include "tagnode.h"
#include <QFileInfo>
#include <regex>

using json = nlohmann::json;

TagTreeItem::TagTreeItem(TagNode *node) : QTreeWidgetItem(), m_tag(node)
{
    if (m_tag == nullptr) return;
    m_tag->setLeaf(this);
    if (m_tag->isRoot()) {
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

        QString icon = QString::fromStdString(m_tag->icon());
        if (QFileInfo::exists(icon) || icon.startsWith(":/icons/"))
            setIcon(0, QIcon(icon));
        else
            setIcon(0, QIcon(":/icons/" + icon));

        refreshFileIcons();
    }
}

void TagTreeItem::setTag(TagNode *tag) { m_tag = tag; }

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
    this->setIcon(1, images ? QIcon::fromTheme(QIcon::ThemeIcon::CameraPhoto) : QIcon());
    this->setIcon(2, videos ? QIcon::fromTheme(QIcon::ThemeIcon::CameraVideo) : QIcon());
}

