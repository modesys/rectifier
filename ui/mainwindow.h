#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QListWidgetItem>
#include <QDir>
#include <QDockWidget>
#include <QPlainTextEdit>
#include <QImageWriter>

#include "../libCam/include/stereocal.h"
#include "../libCam/include/stereolist.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void yamlFileUpload(QString& filename);

    bool fileExists(QString path);

    bool fileExistsForSaveStep_A(QString path);
    bool fileExistsForSaveStep_B(QString path);
    void bothPrintScreenBtn(const QString &pathImg, bool checkFolder);

    void printScreenA(const QString& pathImg, bool checkFolder);
    void printScreenB(const QString& pathImg, bool checkFolder);

    void printScreenToFile_A(const QString& pathImg);
    void printScreenToFile_B(const QString &pathImg);

    void recSaveA();
    void recSaveB();




private slots:
    void imageOriginlUploadA();
    void imageOriginlUploadB();
    void on_originalmgA_clicked();
    void on_originalmgB_clicked();
    void on_loadPairA_clicked();
    void on_loadPairB_clicked();
    void on_listWidgetOriginalImgA_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_listWidgetOriginalImgB_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_saveToFileBtnCamA_clicked();
    void on_loadYMLFile_clicked();
    void on_listWidgetPairA_currentRowChanged(int currentRow);
    void on_listWidgetPairB_currentRowChanged(int currentRow);
    void on_saveToFileBtnCamB_clicked();
    void on_fitViewBtn_clicked();
    void on_bothPrintScreenBtn_clicked();
    void on_saveStep_A_clicked();
    void on_saveStep_B_clicked();

    void on_saveToFile_A_clicked();

    void on_saveToFile_B_clicked();

    void on_fit_A_clicked();

    void on_fit_B_clicked();

    void on_checkBoxScreen_A_toggled(bool checked);

    void on_checkBoxScreen_B_toggled(bool checked);

    void on_rectifyBtnA_clicked();

    void on_rectifyBtnB_clicked();


private:
    Ui::MainWindow *ui;
    QList<QGraphicsPixmapItem*> leftPix, rightPix;
    QGraphicsScene *mLeftScene, *mRightScene;
    QDockWidget *mDockWidget_A, *mDockWidget_B;
    QPlainTextEdit *mNewTextSQLLog/* ,*mNewTextSQLLog */;
    QTextEdit *mNewText;

    QString currentLeftImagePath, dir_Original_A, filesListLeft;
    QString currentRightImagePath, dir_Original_B, filesListRight;
    QDir dir;
    QDir dirB;
    QDir loadImagesCamA;
    QString loadImagesCamB;


    StereoCal match;
    QList<StereoImage> finishedImages;
    QString fileCamRectA;
    QString fileCamRectB;
    QString loadedDirectory_RecA;
    QString loadedDirectory_RecB;

    QString main_graphFirst;
    QString main_graphSecond;
    QString fileGraphLeft;
    QString fileGraphRight;
    QString imgadir;
    void loadImagesGraphics(int row);
    std::shared_ptr<StereoList> listPtr;
    QString loadedDirectory;
    QString loadedDirectoryB;
    QString printA;
    QString printB;

    //QGraphicsTextItem *item1;


    bool imageLoaded;
    int counterA=0;
    int counterB=0;

    int counterAA=0;
    int counterBB=0;



//    int counterB=0;

    bool Lwrite = true;
    bool savePrintScreenA = true;
    bool savePrintScreenB = true;

    bool saveToFileA = true;
    bool saveToFileB = true;

    int currentImagesUploaded = -1;
//    bool Rwrite = true;












};

#endif // MAINWINDOW_H
