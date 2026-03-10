#include "icondialog.h"
#include "icontable.h"
#include "ui_icondialog.h"
#include <QScrollBar>
#include <QFileDialog>

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

    ui->iconTable->updateLayout(getAvailableColumns());
}

void IconDialog::setSelected(QString value) {
    iconPath = value;

    ui->iconTable->setCurrentItem(ui->iconTable->item(value));
}

void IconDialog::browseIcon() {
    QString filePath = QFileDialog::getOpenFileName(this,
        "Select icon",
        "/home",
        "Icon files (*.png *ico *svg)"
    );
    filePath.replace("\\", "/");
    emit iconSelected(filePath);
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
