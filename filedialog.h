#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QDialog>
#include <QtCore>
#include <QtGui>
#include <QFileSystemModel>

namespace Ui {
class fileDialog;
}

class fileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit fileDialog(QWidget *parent = 0);
    QString getPath() {return sPath; }
    void UpdateLabel(const QString text);
    ~fileDialog();

private slots:
    void on_path_treeView_clicked(const QModelIndex &index);

private:
    Ui::fileDialog *ui;
    QFileSystemModel *dirmodel;
    QString sPath;
};

#endif // FILEDIALOG_H
