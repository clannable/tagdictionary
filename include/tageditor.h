#ifndef TAGEDITOR_H
#define TAGEDITOR_H

#include <QWidget>
#include <QListWidgetItem>

#include "tagnode.h"
#include "taglist.h"
#include <nlohmann/json.hpp>

namespace Ui {
class TagEditor;
}

class TagListContainer : public QWidget
{
    Q_OBJECT
public:
    explicit TagListContainer(QString title, TagList* list, QWidget* parent = nullptr);

    void setLineVisible(bool visible);
    TagList* list() const { return m_list; }
    //! Returns `true` if the container's TagList widget is empty, `false` otherwise.
    bool empty() { return m_list->topLevelItemCount() == 0; }

private:
    TagList* m_list;
};

class TagEditor : public QWidget
{
    Q_OBJECT

public:
    explicit TagEditor(QWidget *parent = nullptr);
    ~TagEditor();

public slots:
    void onEditModeChange(bool editModeEnabled);
    void selectIcon();
    void iconSelected(QString icon);
    void setTag(TagNode *tag);
    void save();
    void cleanSublists();
    void checkEmptyLists();

protected:
    virtual void resizeEvent(QResizeEvent* event) override;

private slots:
    void onAnchorClick(const QUrl &link);

signals:
    void tagSaved(TagNode* tag);
    void displayLinkClicked(int index);

private:
    Ui::TagEditor *ui;

    TagList* u_requiredList;
    TagList* u_relatedList;
    TagList* u_frequentList;
    TagList* u_excludesList;
    QString m_iconPath;
    QList<TagListContainer*> lists;

    void hideList(int index);
};

#endif // TAGEDITOR_H
