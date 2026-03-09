#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QListWidgetItem>

#include <QMap>
#include <QSettings>
#include "./jsonnode.h"
#include "./tagtreeitem.h"
#include "./icondialog.h"

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

    void setEditMode(bool mode);

    void onSave(json tag);
    void onAddFile(QString filePath);
    void saveJson();
    void openJson();
    void reloadJson();

private:
    Ui::MainWindow *ui;
    TagTreeItem* selectedItem;

    QString jsonFilePath;

    bool editModeEnabled = false;
};
#endif // MAINWINDOW_H
