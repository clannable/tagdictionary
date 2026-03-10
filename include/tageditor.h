#ifndef TAGEDITOR_H
#define TAGEDITOR_H

#include <QWidget>
#include <QListWidgetItem>

#include "./jsonnode.h"
#include "./icondialog.h"
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

public slots:
    void toggleEditMode();
    void selectIcon();
    void iconSelected(QString icon);
    void setTag(JsonNode *node);
    void save();
    void onListItemSelect(QListWidgetItem* item);
    void addToRelated(JsonNode* node);
    void addToRequired(JsonNode* node);

signals:
    void editModeChanged(bool mode);
    void tagSaved(nlohmann::json tag);
    void listItemSelected(QString tagPath);

private:
    Ui::TagEditor *ui;
    IconDialog *iconDialog;
    JsonNode *currentTag = nullptr;
    bool editModeEnabled = false;

    QString iconPath;
    QString description;
};

#endif // TAGEDITOR_H
