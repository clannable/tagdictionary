#ifndef TAGEDITOR_H
#define TAGEDITOR_H

#include <QWidget>
#include <QListWidgetItem>

#include "tagnode.h"
#include "icondialog.h"
#include "tagtree.h"
#include "taglistwidget.h"
#include <nlohmann/json.hpp>

namespace Ui {
class TagEditor;
}

class TagEditor : public QWidget
{
    Q_OBJECT

public:
    explicit TagEditor(QWidget *parent = nullptr);
    ~TagEditor();

    void linkTagTreeToLists(const TagTree* tree);

public slots:
    void toggleEditMode();
    void selectIcon();
    void iconSelected(QString icon);
    void setTag(TagNode *node);
    void save();
    void onListItemSelect(QString tag);
    void addToRelated(TagNode* node);
    void addToRequired(TagNode* node);
    void refreshLists();


signals:
    void editModeChanged(bool mode);
    void tagSaved(TagNode* tag, std::string oldPath);
    void listItemSelected(QString tagPath);

private:
    Ui::TagEditor *ui;
    IconDialog *iconDialog;
    TagNode *currentTag = nullptr;
    bool editModeEnabled = false;

    TagListWidget *requiredList;
    TagListWidget *relatedList;

    QString iconPath;
    QString description;
};

#endif // TAGEDITOR_H
