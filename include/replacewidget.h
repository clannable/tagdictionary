#ifndef REPLACEWIDGET_H
#define REPLACEWIDGET_H

#include "tagnode.h"
#include <QWidget>


class ReplaceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ReplaceWidget(QWidget *parent = nullptr);

    bool isReplaceChecked();
    void setReplaceChecked(bool replace);

    void useParent(const TagNode* tag);

public slots:
    void onEditModeChange(bool editModeEnabled);

signals:

};

#endif // REPLACEWIDGET_H
