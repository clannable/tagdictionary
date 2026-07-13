#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QMap>
#include <QSettings>
#include <QTimer>

#include "tagnode.h"
#include "tagtreeitem.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onTagSelect();
    void onTagListSelect(QString tagPath);
    void onTagDoubleClicked(QTreeWidgetItem* item, int column);
    void onSearchChange();
    void onSearchTimeout();

    void setEditMode(bool mode);

    void onSave(TagNode* tag, std::string oldPath);
    void onAddFile(QString filePath);
    void onToggleAutoSave(bool checked);
    void saveJson();
    void openJson();
    void newJson();
    void reloadJson();

private:
    Ui::MainWindow *ui;
    TagTreeItem* selectedItem;

    QString jsonFilePath;
    QAction* saveAction;
    QAction* openAction;
    QAction* newTagAction;
    QMenu* recentMenu;
    QTimer* searchDebounce;

    bool editModeEnabled = false;

    void pushToRecent();
    void setupRecentFileList();
};
#endif // MAINWINDOW_H
