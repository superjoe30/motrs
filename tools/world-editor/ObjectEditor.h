#ifndef OBJECTEDITOR_H
#define OBJECTEDITOR_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>

class ObjectView;

namespace Ui {
    class ObjectEditor;
}

class ObjectEditor : public QMainWindow {
    Q_OBJECT
public:
    ObjectEditor(QWidget *parent = 0);
    ~ObjectEditor();

    void open(QString file);

    // expose some widgets (used by ObjectView)
    QListWidget * artList();
    QListWidget * layersList();
    QListWidget * shapesList();
    QListWidget * surfaceTypesList();
    QTableWidget * propertiesTable();

    QPushButton * newLayerButton();
    QPushButton * deleteLayerButton();
    QPushButton * moveLayerUpButton();
    QPushButton * moveLayerDownButton();

    QAction * viewModeNormalAction();
    QAction * viewModeShapeAction();
    QAction * viewModeSurfaceTypeAction();
    QDockWidget * artDock();
    QDockWidget * shapesDock();
    QDockWidget * surfaceTypesDock();

    QAction * cutAction();
    QAction * copyAction();
    QAction * pasteAction();
    QAction * deleteAction();

    void createEmpty();
signals:
    void objectSaved();

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent * e);

private:
    Ui::ObjectEditor *m_ui;

    ObjectView * m_view;

private slots:
    void on_actionSave_triggered();
    void on_actionPaste_triggered();
    void on_actionCopy_triggered();
    void on_actionCut_triggered();
    void on_actionDelete_triggered();
    void on_tblProperties_cellChanged(int row, int column);
    void on_actionClose_triggered();
    void on_btnDeleteLayer_clicked();
    void on_btnNewLayer_clicked();
    void on_btnMoveLayerDown_clicked();
    void on_btnMoveLayerUp_clicked();
    void on_actionZoomOut_triggered();
    void on_actionZoomIn_triggered();
    void on_actionZoom100_triggered();
    void on_actionShape_triggered();
    void on_actionSurfaceType_triggered();
    void on_actionNormal_triggered();
    void on_lstLayers_currentRowChanged(int currentRow);
    void on_lstLayers_itemChanged(QListWidgetItem *);

};

#endif // OBJECTEDITOR_H
