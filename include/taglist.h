#ifndef TAGLIST_H
#define TAGLIST_H

#include <QListWidget>
#include <QWidget>
#include <QMouseEvent>
#include <QMimeData>
#include "nlohmann/json.hpp"

class TagList : public QListWidget
{
    Q_OBJECT

public:
    TagList(QWidget* parent = nullptr);
    std::list<int> values();

    void createListItem(int id, int index = -1);
    void removeTag(int id);
    void setValues(nlohmann::json::array_t ids);
signals:
    void tagRemoved();

public slots:
    void onEditModeChanged(bool editModeEnabled);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual QMimeData* mimeData(const QList<QListWidgetItem*>& items) const override;
    virtual QStringList mimeTypes() const override;
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;

protected slots:
    void onItemDoubleClicked(QListWidgetItem* item);

};

#endif // TAGLIST_H
