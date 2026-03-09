#ifndef ICONTABLE_H
#define ICONTABLE_H

#include <QWidget>
#include "iconpanel.h"

class IconTable : public QWidget
{
    Q_OBJECT
public:
    explicit IconTable(QWidget *parent = nullptr);
    IconPanel* currentItem() const;

    IconPanel* item(QString icon);

    void updateLayout(int cols);
    int columnCount() const;

public slots:
    void setCurrentItem(IconPanel *panel);

signals:
    void selectionChanged(IconPanel *panel);
    void columnsChanged(int columns);

private:
    QList<IconPanel*> panels;
    IconPanel* current;

};

#endif // ICONTABLE_H
