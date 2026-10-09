#ifndef TAGTREE_H
#define TAGTREE_H

#include <QTreeWidget>
#include <QMouseEvent>

#include "tagnode.h"
#include "tagtreeitem.h"
#include <nlohmann/json.hpp>

class TagTree : public QTreeWidget
{
    Q_OBJECT

public:
    TagTree(QWidget* parent);
    // ~TagTree();
    QTreeWidgetItem* findTag(int tagId);

    void fromJson(nlohmann::json json);
    json toJson();

public slots:

    void onCreateTag(TagTreeItem* item);
    void onNewTag(TagTreeItem* srcItem, TagNode* node);
    void onRemoveTag(TagTreeItem* item);

    void expandTreeTo(QTreeWidgetItem* item);
    void filterTree(QString search);

    // void onExpandSelected();
    // void onCollapseSelected();
    // void onExpandAll();
    // void onCollapseAll();

protected:

    // virtual void mouseMoveEvent(QMouseEvent *event) override;
    // virtual void mousePressEvent(QMouseEvent *event) override;
    // virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual void dropEvent(QDropEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    virtual QMimeData* mimeData(const QList<QTreeWidgetItem*> &items) const override;
    virtual QStringList mimeTypes() const override;
    // virtual void dragEnterEvent(QDragEnterEvent *event) override;

signals:
    void addToRelated(TagNode *node);
    void addToRequired(TagNode *node);
    void tagsChanged();
    void listsUpdated();

private:
    TagNode* m_rootNode = nullptr;
    TagTreeItem* m_rootItem = nullptr;

    // TagTreeItem* dragItem;
    // QPoint dragStartPosition;
    // TagTreeItem* findTag(TagTreeItem* item, QString tagPath);

    void createChildren(TagTreeItem* item, TagNode *node);
    bool filterTags(QString search, QTreeWidgetItem *node);
    void resetTagVisibility();

    void setExpandedRecursive(bool expanded, QTreeWidgetItem* root);

private slots:
    // void signalRelated();
    // void signalRequired();

    void onEditModeChanged(bool editModeEnabled);
    // void onSelectedTagChanged(TagNode* tag);

    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onItemSelected(QTreeWidgetItem* current, QTreeWidgetItem* previous);
};

#endif // TAGTREE_H
