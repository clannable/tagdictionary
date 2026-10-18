#ifndef TAGLISTWIDGET_H
#define TAGLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <nlohmann/json.hpp>
#include "tagnode.h"
#include <qevent.h>
#include <QPushButton>
#include <list>
#include <QLabel>
#include "taglist.h"

using json = nlohmann::json;

class TagListWidget : public QWidget
{
    Q_OBJECT

public:
    TagListWidget(QString title, QWidget *parent=nullptr);

    std::list<int> values();

public slots:
    void insertTag(int tagId);
    void setTag(TagNode* node);
    void onItemSelected(QListWidgetItem* item);
    void clear();
    // void toggleExpanded();

protected:
    // virtual void dropEvent(QDropEvent *event) override;
    // virtual void dragEnterEvent(QDragEnterEvent *event) override;
    // virtual void contextMenuEvent(QContextMenuEvent *event) override;
    // virtual void mousePressEvent(QMouseEvent *event) override;

private:
    TagList* u_list;
    QString m_title;
};

#endif // TAGLISTWIDGET_H
