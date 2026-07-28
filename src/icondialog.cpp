#include "icondialog.h"
#include "icontable.h"
#include "ui_icondialog.h"
#include <QScrollBar>
#include <QFileDialog>
#include "globals.h"
IconDialog::IconDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::IconDialog)
{
    ui->setupUi(this);

}

void IconDialog::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    if (!isVisible()) return;
    if (ui->scrollArea->viewport() == nullptr) return;
    int cols = getAvailableColumns();
    if (cols+1 != ui->iconTable->columnCount()) {
        ui->iconTable->updateLayout(cols);
    }
}

void IconDialog::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
    ui->iconTable->refresh();
    ui->iconTable->updateLayout(getAvailableColumns());
}

void IconDialog::setSelected(QString value) {
    iconPath = value;

    ui->iconTable->setCurrentItem(ui->iconTable->item(value));
}

void IconDialog::browseIcon() {
    QString filePath = QFileDialog::getOpenFileName(this,
        "Select icon",
        QString::fromStdString(LAST_ICON_FOLDER_PATH),
        "Icon files (*.png *ico *svg)"
    );
    if (filePath.isNull()) return;

    filePath.replace("\\", "/");
    LAST_ICON_FOLDER_PATH = QFileInfo(filePath).absoluteDir().path().toStdString();
    emit iconSelected(filePath);
    ICON_LIST->push_back(filePath.toStdString());
    ICON_LIST->sort();
    ICON_LIST->unique();
    QDialog::accept();
}

void IconDialog::clearSelection() {
    ui->iconTable->setCurrentItem(nullptr);
}

void IconDialog::accept() {
    IconPanel *selected = ui->iconTable->currentItem();
    emit iconSelected(selected != nullptr ? selected->getIcon() : iconPath);
    QDialog::accept();
}

int IconDialog::getAvailableColumns() const {
    return ui->scrollArea->viewport()->width() / 40;
}

IconDialog::~IconDialog()
{
    delete ui;
}
