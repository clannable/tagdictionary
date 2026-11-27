#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tagnode.h"
#include "globals.h"
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
#include <QFileInfo>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDirIterator>
#include <QLineEdit>

using json = nlohmann::json;


// ----------- Initialize global values --------------------
std::list<std::string>* ICON_LIST = new std::list<std::string>();
std::string LAST_IMAGE_FOLDER_PATH = "/home";
std::string LAST_ICON_FOLDER_PATH = "/home";
int NEXT_TAG_ID = 1;
std::map<int, TagNode*> TAGS = {};
std::map<std::string, int> TAG_PATH_MAP = {};
std::chrono::milliseconds DEBOUNCE_TIME = 250ms;
bool AUTOSAVE_ENABLED = true;
bool HIDE_EMPTY_LISTS = false;
const int MAX_RECENT = 5;
AppState* APP_STATE = new AppState();
// ---------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Load user app settings
    QSettings settings("MyApp","Tag Viewer");
    AUTOSAVE_ENABLED = settings.value("autosave", true).toBool();
    HIDE_EMPTY_LISTS = settings.value("hideEmptySubLists", false).toBool();

    if (settings.contains("geometry")) // Restore previous window geometry
        this->restoreGeometry(settings.value("geometry").toByteArray());
    m_jsonFilePath = settings.value("data/lastOpened", "").toString();
    m_recentFiles = settings.value("data/recentFiles").toStringList();
    if (!m_jsonFilePath.isEmpty() && !QFileInfo::exists(m_jsonFilePath)) {
        QMessageBox::critical(this, "Failed to load file", "ERROR: Failed to load dictionary file: \n\tFile not found");
        m_jsonFilePath = "";
    }
    ui->a_save->setEnabled(!m_jsonFilePath.isEmpty());
    ui->a_autosave->setEnabled(AUTOSAVE_ENABLED);
    ui->a_hideEmptyLists->setChecked(HIDE_EMPTY_LISTS);

    setupRecentFileList();

    connect(APP_STATE, &AppState::selectedTagUpdated, this, &MainWindow::onTagUpdate);
    connect(APP_STATE, &AppState::selectedTagChanged, this, &MainWindow::onSelectedTagChanged);

    connect(ui->a_newTag, &QAction::triggered, ui->tagTree, &TagTree::newTag);
    connect(ui->a_modifyIcon, &QAction::triggered, ui->tagEditor, &TagEditor::selectIcon);
    connect(ui->tagEditor, &TagEditor::tagSaved, this, &MainWindow::onEditorSave);
    connect(ui->tagEditor, &TagEditor::displayLinkClicked, ui->mediaDisplay, &MediaDisplay::setFile);
    connect(ui->tagTree, &TagTree::tagsChanged, this, &MainWindow::onTagTreeChange);

    m_searchDebounce = new QTimer(this);
    m_searchDebounce->setSingleShot(true); // Only execute the debounce timeout event once
    connect(m_searchDebounce, &QTimer::timeout, this, &MainWindow::searchTree); // Execute the search once the debounce times out

    if (!m_jsonFilePath.isEmpty())
        reloadJson();
}

MainWindow::~MainWindow()
{
    QSettings settings("MyApp","Tag Viewer");
    settings.setValue("geometry", this->saveGeometry());
    settings.setValue("autosave", AUTOSAVE_ENABLED);
    settings.setValue("hideEmptySubLists", HIDE_EMPTY_LISTS);
    settings.setValue("data/recentFiles", m_recentFiles);
    settings.setValue("data/lastOpened", m_jsonFilePath);
    delete ui;
    delete ICON_LIST;
}

//--------- Menu Bar Slots -----------//

void MainWindow::newJson() {
    m_jsonFilePath = QFileDialog::getSaveFileName(
        this,
        "Select where to save JSON file",
        m_jsonFilePath.isEmpty() ? "" : QFileInfo(m_jsonFilePath).dir().absolutePath(),
        "Tag Dictionary (*.json)"
        );
    if (!m_jsonFilePath.isEmpty()) {
        std::ofstream of(m_jsonFilePath.toStdString());
        if (!of.is_open()) {
            // std::cout << "Failed to open output file\n" << std::flush;
            ui->a_save->setEnabled(false);
        } else {
            of << json({}).dump(2);
            of.close();
            pushToRecent();
            reloadJson();
            ui->a_save->setEnabled(!m_jsonFilePath.isEmpty());
        }
    }
}


void MainWindow::openJson() {
    QString filePath = QFileDialog::getOpenFileName(this,
        "Select JSON file",
        m_jsonFilePath.isEmpty() ? "" : QFileInfo(m_jsonFilePath).dir().absolutePath(),
        "Tag Dictionary (*.json)");
    if (!filePath.isEmpty()) {
        m_jsonFilePath = filePath;
        pushToRecent();
        reloadJson();
    }
}

void MainWindow::saveJson() {
    if (m_jsonFilePath.isNull() || m_jsonFilePath.isEmpty()) return;

    json tags = json({{ "tags", json() }});
    for (const auto [id, tag] : TAGS) {
        if (id <= 0 || tag == nullptr) continue;
        tags["tags"][tag->key()] = tag->toJson();
    }
    std::ofstream of(m_jsonFilePath.toStdString());
    if (!of.is_open()) {
        std::cout << "Failed to open output file\n" << std::flush;
    } else {
        of << tags.dump(2);
        of.close();
    }
}

void MainWindow::setAutosaveEnabled(bool checked) {
    AUTOSAVE_ENABLED = checked;
}

void MainWindow::setHideEmptyLists(bool checked) {
    HIDE_EMPTY_LISTS = checked;
    ui->tagEditor->checkEmptyLists();
}
/*---------  Tag Tree Slots ---------*/

void MainWindow::onSelectedTagChanged(TagNode* selectedTag) {
    ui->a_modifyIcon->setEnabled(selectedTag && !selectedTag->isRoot());
}

//! Triggers whenever a tag in the tree is added, removed, or moved.
void MainWindow::onTagTreeChange() {
    ui->tagEditor->cleanSublists();
    if (AUTOSAVE_ENABLED) saveJson();
}

//! Triggers on each keystroke in the search field, resetting the search debounce timer.
void MainWindow::restartSearchDebounce() {
    m_searchDebounce->start(DEBOUNCE_TIME);
}

//! Triggers when the search debounce timer finishes, executing the text search on the tag tree.
void MainWindow::searchTree() {
    ui->tagTree->filterTree(ui->searchInput->text());
}

//--------- Tag Editor Slots ---------

void MainWindow::onEditorSave() {
    ui->mediaDisplay->save();
    QStringList files = ui->mediaDisplay->files();
    APP_STATE->selectedTag()->setFiles(files);

    ui->tagTree->sortItems(0, Qt::AscendingOrder);

    APP_STATE->signalSelectedTagUpdated();
    APP_STATE->setEditModeEnabled(false);
}

void MainWindow::onTagUpdate() {
    ui->tagTree->setCurrentTag(APP_STATE->selectedTag());
    ui->tagTree->refreshTag(APP_STATE->selectedTag());
    if (AUTOSAVE_ENABLED)
        saveJson();
}

//--------- JSON Functions ---------

void MainWindow::reloadJson() {
    if (m_jsonFilePath.isNull() || m_jsonFilePath.isEmpty()) return;
    TAGS.clear();
    APP_STATE->setEditModeEnabled(false);
    APP_STATE->setSelectedTag(nullptr);
    ui->searchInput->clear();
    std::ifstream* f;

    try {
        f = new std::ifstream(m_jsonFilePath.toStdString());
        ICON_LIST->clear();

        QDirIterator it(":/icons/", QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString path = it.next();
            if (path.lastIndexOf(".") == -1) continue;
            ICON_LIST->push_back(path.toStdString());
        }

        json tags = json::parse(*f);

        for (const auto& [k, t] : tags["tags"].items()) {
            TagNode* tag = new TagNode(t, k);
            if (!tag->icon().empty())
                ICON_LIST->push_back(tag->icon());
            TAGS[tag->id()] = tag;
        }
        for (const auto [id, t] : TAGS) {
            list<int> children = t->children();
            for (auto itr = children.begin(); itr != children.end(); itr++)
                if (TAGS.find(*itr) != TAGS.end())
                   TAGS[*itr]->setParent(t);
        }
        ICON_LIST->sort();
        ICON_LIST->unique();
        ui->tagTree->loadTags();
        ui->tagTree->sortByColumn(0, Qt::AscendingOrder);
        ui->a_newTag->setEnabled(true);
        setWindowTitle("Tag Dictionary - " + QFileInfo(m_jsonFilePath).fileName());
    } catch (std::exception e) {
        QMessageBox::critical(this, "An error has occurred", "Error loading dictionary");
        ui->tagTree->clear();
        ui->a_newTag->setEnabled(false);
        setWindowTitle("Tag Dictionary");
    }

    if (f->is_open())
        f->close();
}

void MainWindow::openRecentFile(QAction* action) {
    m_jsonFilePath = action->data().toString();
    pushToRecent();
    reloadJson();
}

void MainWindow::pushToRecent() {
    if (m_recentFiles.contains(m_jsonFilePath))
        m_recentFiles.removeAt(m_recentFiles.indexOf(m_jsonFilePath));
    else if (m_recentFiles.size() == MAX_RECENT)
        m_recentFiles.removeLast();

    m_recentFiles.push_front(m_jsonFilePath);

    setupRecentFileList();
}

void MainWindow::setupRecentFileList() {
    ui->m_openRecent->clear();
    ui->m_openRecent->menuAction()->setEnabled(!m_recentFiles.isEmpty());

    for (int i = 0; i < m_recentFiles.length(); i++) {
        QString filePath = m_recentFiles.at(i);
        QAction* a = new QAction(QString::number(i+1) + ". \t\t" + QFileInfo(filePath).fileName(), this);
        a->setToolTip(filePath);
        a->setData(filePath);
        ui->m_openRecent->addAction(a);
    }
}
