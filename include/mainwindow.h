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

    QAction* a_save;
    QAction* a_open;
    QAction* a_newTag;
    QAction* a_icon;

    QMenu* u_recent;
    QTimer* m_searchDebounce;

    void pushToRecent();
    void setupRecentFileList();

private slots:
    void onSearchChange();
    void onSearchTimeout();

    // void setEditMode(bool mode);
    void onTagTreeChange();
    void onEditorSave();
    void onTagUpdate();
    void onToggleAutoSave(bool checked);

    void saveJson();
    void openJson();
    void newJson();
    void reloadJson();
};
#endif // MAINWINDOW_H
