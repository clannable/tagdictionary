#ifndef ICONTABLE_H
#define ICONTABLE_H

#include <QWidget>
#include "iconpanel.h"

class IconTable : public QWidget
{
    Q_OBJECT
public:
    explicit IconTable(QWidget *parent = nullptr);
    IconPanel* currentPanel() const { return u_currentPanel; }
    IconPanel* item(QString icon);

    void updateLayout(int cols);
    int columnCount() const;


public slots:
    void setCurrentItem(IconPanel* panel);
    void refresh();

signals:
    void selectionChanged(IconPanel* panel);
    void columnsChanged(int columns);

private:
    QStringList m_icons;
    QList<IconPanel*> u_panels;
    IconPanel* u_currentPanel = nullptr;
};

#endif // ICONTABLE_H
