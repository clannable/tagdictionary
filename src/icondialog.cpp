#include "icondialog.h"
#include "ui_icondialog.h"
#include <QScrollBar>
#include "./icontable.h"

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
    if (value.indexOf(":/icons/") != -1)
        ui->iconTable->setCurrentItem(ui->iconTable->item(value.right(value.length() - 8)));
    else
        ui->iconTable->setCurrentItem(ui->iconTable->item(value));

}

void IconDialog::clearSelection() {
    ui->iconTable->setCurrentItem(nullptr);
}

void IconDialog::accept() {
    IconPanel *selected = ui->iconTable->currentItem();
    emit iconSelected(selected != nullptr ? selected->getIcon() : "");
    QDialog::accept();
}

int IconDialog::getAvailableColumns() const {
    return ui->scrollArea->viewport()->width() / 40;
}

IconDialog::~IconDialog()
{
    delete ui;
}
