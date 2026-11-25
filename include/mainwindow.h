#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QMap>
#include <QSettings>
#include <QTimer>

#include "tagnode.h"

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

private:
    Ui::MainWindow *ui;

    QString m_jsonFilePath;
    QStringList m_recentFiles;
    QTimer* m_searchDebounce;

    void pushToRecent();
    void setupRecentFileList();

private slots:
    // Menu bar slots
    void newJson();
    void openJson();
    void openRecentFile(QAction* action);
    void saveJson();
    void setAutosaveEnabled(bool checked);
    void setHideEmptyLists(bool checked);
    // Search slots
    void restartSearchDebounce();
    void searchTree();

    void onSelectedTagChanged(TagNode* selectedTag);
    void onTagTreeChange();
    void onEditorSave();
    void onTagUpdate();

    void reloadJson();
};
#endif // MAINWINDOW_H
