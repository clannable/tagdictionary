#ifndef ICONDIALOG_H
#define ICONDIALOG_H

#include <QDialog>
#include <QTableWidgetItem>
#include <QList>
#include <QGridLayout>

#include "iconpanel.h"
namespace Ui {
class IconDialog;
}

class IconDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IconDialog(QWidget *parent = nullptr);
    ~IconDialog();
    void setSelected(QString value);

public slots:
    virtual void accept() override;
    void clearSelection();
    void browseIcon();

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

signals:
    void iconSelected(QString iconPath);

private:
    Ui::IconDialog *ui;
    QString iconPath = "";
    int getAvailableColumns() const;
};

#endif // ICONDIALOG_H
