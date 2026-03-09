#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "jsonnode.h"
#include "tagtreeitem.h"

#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <QMenuBar>
#include <QString>
#include <QIcon>
#include <QPixmap>
#include <QMimeDatabase>
#include <QAudioOutput>
#include <QFileDialog>
#include <QDesktopServices>


using json = nlohmann::json;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QMenu *fileMenu = ui->menuBar->addMenu("File");
    QAction *saveAction = new QAction("Save JSON", this);
    QAction *openAction = new QAction("Open...", this);
    openAction->setShortcuts(QKeySequence::Open);
    saveAction->setShortcuts(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveJson);
    connect(openAction, &QAction::triggered, this, &MainWindow::openJson);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(openAction);

    connect(ui->tagEditor, &TagEditor::editModeChanged, ui->treeWidget, &TagTree::setEditMode);
    connect(ui->tagEditor, &TagEditor::editModeChanged, ui->mediaDisplay, &MediaDisplay::setEditMode);
    connect(ui->tagEditor, &TagEditor::listItemSelected, this, &MainWindow::onTagListSelect);
    connect(ui->tagEditor, &TagEditor::tagSaved, this, &MainWindow::onSave);
    connect(ui->tagEditor, &TagEditor::editModeChanged, this, &MainWindow::setEditMode);
    connect(ui->mediaDisplay, &MediaDisplay::fileAdded, this, &MainWindow::onAddFile);

    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this, &MainWindow::onTagDoubleClicked);
    connect(ui->treeWidget, &QTreeWidget::itemSelectionChanged, this, &MainWindow::onTagSelect);
    connect(ui->treeWidget, &TagTree::tagsChanged, this, &MainWindow::saveJson);
    connect(ui->treeWidget, &TagTree::addToRelated, ui->tagEditor, &TagEditor::addToRelated);
    connect(ui->treeWidget, &TagTree::addToRequired, ui->tagEditor, &TagEditor::addToRequired);

    QSettings settings("MyApp","Tag Viewer");
    jsonFilePath = settings.value("data/lastOpened", "").toString();

    reloadJson();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*---------  Tag Tree Slots ---------*/

void MainWindow::onTagSelect() {
    if (editModeEnabled) return;
    selectedItem = static_cast<TagTreeItem*>(ui->treeWidget->currentItem());

    JsonNode *node = selectedItem->getNode();
    ui->tagEditor->setTag(node);
    ui->mediaDisplay->setFilesFromNode(node);
}

void MainWindow::onTagDoubleClicked(QTreeWidgetItem *item, int column) {
    Q_UNUSED(column);
    if (editModeEnabled) return;
    if (item != selectedItem)
        selectedItem = static_cast<TagTreeItem*>(item);

    ui->tagEditor->toggleEditMode();
}

void MainWindow::onTagListSelect(QString tagPath) {
    if (editModeEnabled == true) return;

    TagTreeItem *tag = ui->treeWidget->findTag(tagPath);
    selectedItem->setSelected(false);
    selectedItem = tag;
    selectedItem->setSelected(true);
    ui->treeWidget->expandItem(selectedItem);
}

/*--------- Tag Editor Slots ---------*/

void MainWindow::setEditMode(bool mode) {
    editModeEnabled = mode;

    QFont font = selectedItem->font(0);
    font.setWeight(mode ? QFont::DemiBold : QFont::Normal);
    selectedItem->setFont(0, font);

    ui->treeWidget->setCurrentItem(selectedItem);
}

void MainWindow::onSave(json tag) {

    QString key = QString::fromStdString(tag.value("key", ""));
    tag.erase("key");
    tag["files"] = json(ui->mediaDisplay->getFileList().toList());
    selectedItem->setKey(key);
    selectedItem->getNode()->setData(tag);
    selectedItem->setIcon(0, QIcon(":/icons/" + QString::fromStdString(tag.value("icon", ""))));

    ui->mediaDisplay->setFilesFromNode(selectedItem->getNode());
    saveJson();
}

/*--------- Media Display Slots ---------*/

void MainWindow::onAddFile(QString filePath) {
    if (editModeEnabled) return;
    json data = selectedItem->getJson();
    data.emplace("files", json::array());
    data["files"].push_back(filePath.toStdString());
    selectedItem->getNode()->setData(data);
    saveJson();
}


/*--------- JSON Functions ---------*/

void MainWindow::saveJson() {
    if (jsonFilePath.isNull() || jsonFilePath.isEmpty()) return;
    json tags = ui->treeWidget->toJson();
    std::ofstream of(jsonFilePath.toStdString());
    if (!of.is_open()) {
        std::cout << "Failed to open output file\n" << std::flush;
    } else {
        of << tags.dump(2);
        of.close();
    }
}

void MainWindow::openJson() {
    jsonFilePath = QFileDialog::getOpenFileName(
        this,
        "Select JSON file",
        "/home",
        "Tag Dictionary (*.json)");
    QSettings settings("MyApp","Tag Viewer");
    settings.setValue("data/lastOpened", jsonFilePath);
    reloadJson();
}

void MainWindow::reloadJson() {
    if (jsonFilePath.isNull() || jsonFilePath.isEmpty()) return;
    ui->treeWidget->clear();
    editModeEnabled = false;

    selectedItem = nullptr;

    ui->mediaDisplay->clear();
    ui->tagEditor->setTag(nullptr);

    std::cout << "Loading JSON file " << jsonFilePath.toStdString() << "\n" << std::flush;

    std::ifstream f(jsonFilePath.toStdString());
    json tags = json::parse(f);
    ui->treeWidget->fromJson(tags);
    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);

    f.close();
}
