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
    ~TagTree();
    QTreeWidgetItem* findTag(QString tagPath, QTreeWidgetItem* root=nullptr);

    void fromJson(nlohmann::json json);
    json toJson();

public slots:
    void setEditMode(bool mode);
    void onCreateTag();
    void onNewTag(TagNode* node);
    void onRemoveTag();

    void expandTreeTo(QTreeWidgetItem* item);

protected:

    // virtual void mouseMoveEvent(QMouseEvent *event) override;
    // virtual void mousePressEvent(QMouseEvent *event) override;
    // virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual void dropEvent(QDropEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    // virtual void dragEnterEvent(QDragEnterEvent *event) override;

signals:
    void addToRelated(TagNode *node);
    void addToRequired(TagNode *node);
    void tagsChanged();
    void listsUpdated();

private:
    TagNode* rootNode;

    TagTreeItem* menuItem;

    TagTreeItem* dragItem;
    QPoint dragStartPosition;

    bool editModeEnabled;
    // TagTreeItem* findTag(TagTreeItem* item, QString tagPath);

    void createChildren(TagTreeItem* item, TagNode *node);

private slots:
    void signalRelated();
    void signalRequired();
};

#endif // TAGTREE_H
