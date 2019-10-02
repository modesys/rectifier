#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../libCam/include/stereocal.h"
#include "../libCam/include/stereolist.h"
#include "../Rectifier/src/qdockresizeeventfilter.h"
#include "../Rectifier/src/qfluidgridlayout.h"

#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QPixmap>

#include <QDockWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QWidget>
#include <QDial>
#include <QListWidgetItem>
#include <QIcon>
#include <QTextDocumentFragment>
#include <QGraphicsTextItem>

using ListItem = QListWidgetItem;  // Different name for the QListWidgetItem


using namespace cv;

QImage cvMat2QImage(cv::Mat image)
{
  // Convert the image to the RGB888 format
  cv::Mat image_display;
  switch (image.type())
  {
    case CV_8UC1:
      cvtColor(image, image_display, CV_GRAY2RGB);
      break;
    case CV_8UC3:
      cvtColor(image, image_display, CV_BGR2RGB);
      break;
  }
  // QImage needs the data to be stored continuously in memory
  assert(image_display.isContinuous());
  // Assign OpenCV's image buffer to the QImage. Note that the bytesPerLine parameter
  // (http://qt-project.org/doc/qt-4.8/qimage.html#QImage-6) is 3*width because each pixel
  // has three bytes.
  return QImage(image_display.data, image_display.cols, image_display.rows, image_display.cols * 3,
                QImage::Format_RGB888)
      .copy();
}

QPixmap cvMat2QPixmap(cv::Mat image)
{
  return QPixmap::fromImage(cvMat2QImage(image)).copy();
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mLeftScene = new QGraphicsScene();
    ui->graphicsViewLX->setScene(mLeftScene);
    mRightScene = new QGraphicsScene();
    ui->graphicsViewRX->setScene(mRightScene);

    listPtr.reset(new StereoList);
    imageLoaded = false;
    ui->progressBarRect_A->setValue(0);
    ui->progressBarRect_B->setValue(0);
    ui->progressBarSaveA->setValue(0);
    ui->progressBarSaveB->setValue(0);

    ui->labelOrigImageA->setText("<b>No Image Set!</b>");
    ui->labelOrigImageB->setText("<b>No Image Set!</b>");

    mDockWidget_A = new QDockWidget(QLatin1String("Command Log"));
    mDockWidget_A->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    mDockWidget_A->setMinimumHeight(30);
    // Adding object to the DockWidget
    mNewText = new QTextEdit;
    mNewText->setReadOnly(true);
    mNewText->setStyleSheet("background-color: light grey;");
    mNewText->setMinimumHeight(50);
    mNewText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mDockWidget_A->setWidget(mNewText);
    addDockWidget(Qt::BottomDockWidgetArea, mDockWidget_A);
    resizeDocks({mDockWidget_A}, {200}, Qt::Horizontal);

    mDockWidget_B = new QDockWidget(QLatin1String("SQL Log"));
    mDockWidget_B->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mDockWidget_B->setMinimumHeight(30);

    mNewTextSQLLog = new QPlainTextEdit;
    mNewTextSQLLog->setStyleSheet("background-color: light grey;");
    mNewTextSQLLog->setMinimumHeight(50);
    mNewTextSQLLog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mDockWidget_B->setWidget(mNewTextSQLLog);
    addDockWidget(Qt::BottomDockWidgetArea, mDockWidget_B);
    resizeDocks({mDockWidget_B}, {200}, Qt::Horizontal);

//    mDockWidget_A->installEventFilter(new QDockResizeEventFilter(mNewText,dynamic_cast<QFluidGridLayout*>(mNewText->layout())));


    ui->saveStep_A->setEnabled(false);
    ui->saveStep_A->setStyleSheet("QPushButton{ background-color: gray }");
    ui->saveStep_B->setEnabled(false);
    ui->saveStep_B->setStyleSheet("QPushButton{ background-color: gray }");

    ui->saveToFileBtnCamA->setEnabled(false);
    ui->saveToFileBtnCamA->setStyleSheet("QPushButton{ background-color: gray }");
    ui->saveToFileBtnCamB->setEnabled(false);
    ui->saveToFileBtnCamB->setStyleSheet("QPushButton{ background-color: gray }");

    ui->saveToFile_A->setEnabled(false);
    ui->saveToFile_A->setStyleSheet("QPushButton{ background-color: gray }");
    ui->saveToFile_B->setEnabled(false);
    ui->saveToFile_B->setStyleSheet("QPushButton{ background-color: gray }");

    ui->loadPairA->setEnabled(false);
    ui->loadPairA->setStyleSheet("QPushButton{ background-color: gray }");
    ui->loadPairB->setEnabled(false);
    ui->loadPairB->setStyleSheet("QPushButton{ background-color: gray }");

    ui->loadYMLFile->setEnabled(true);
    ui->loadYMLFile->setStyleSheet("QPushButton{ background-color: green }");


    for(const QByteArray & format : QImageWriter::supportedImageFormats())
    {
        ui->comboBoxFormat_A->addItem(format);
    }
    for(const QByteArray & format : QImageWriter::supportedImageFormats())
    {
        ui->comboBoxFormat_B->addItem(format);
    }
    for(const QByteArray & format : QImageWriter::supportedImageFormats())
    {
        ui->printCScreenCBox_A->addItem(format);
    }
    for(const QByteArray & format : QImageWriter::supportedImageFormats())
    {
        ui->printCScreenCBox_B->addItem(format);
    }


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::imageOriginlUploadA()
{
    dir_Original_A = QFileDialog::getExistingDirectory(this, tr("Choose an image directory to load"),
                                                     filesListLeft, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir_Original_A.length() > 0){
        QImage image;
        QDir dirAObj(dir_Original_A);
        QStringList filesListLeft = dirAObj.entryList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
        ui->labelOrigImageA->setPixmap(QPixmap::fromImage(image.scaled(125,125,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
        for ( int i = 0 ; i < filesListLeft.size() ; i++ )
        {
            ui->listWidgetOriginalImgA->addItem(filesListLeft.at(i));
        }
        ui->listWidgetOriginalImgA->update();
        ui->labelOrigImageA->show();
    }
}


void MainWindow::on_originalmgA_clicked()
{
    imageOriginlUploadA();
    QSize s{23, 23};
    QTextDocumentFragment fragment;
    fragment = QTextDocumentFragment::fromHtml(
                QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                .arg(s.width())
                .arg(s.height()));
    mNewText->textCursor().insertFragment(fragment);
    mNewText->insertPlainText("    Images Correctly Updated");
    // Now differentiate betweeen a succesfull action and a failed action
    mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
    mNewText->setVisible(true);
//    item1 = new QGraphicsTextItem("Some Text");
//    item1->setTextInteractionFlags(Qt::TextEditorInteraction);
    QFont f = mNewText->font();
    f.setPointSize(13);
    mNewText->setFont(f);
}

void MainWindow::imageOriginlUploadB()
{
    dir_Original_B = QFileDialog::getExistingDirectory(this, tr("Choose an image directory to load"),
                                                     filesListRight, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir_Original_B.length() > 0){
        QImage image;
        QDir dirBObj(dir_Original_B);
        QStringList filesListRight = dirBObj.entryList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
        ui->labelOrigImageB->setPixmap(QPixmap::fromImage(image.scaled(125,125,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
        for ( int i = 0 ; i < filesListRight.size() ; i++ )
        {
            ui->listWidgetOriginalImgB->addItem(filesListRight.at(i));
        }
        ui->listWidgetOriginalImgB->update();
        ui->labelOrigImageB->show();
    }
}


void MainWindow::on_originalmgB_clicked()
{
    imageOriginlUploadB();
    QSize s{32, 32};
    QTextDocumentFragment fragment;
    fragment = QTextDocumentFragment::fromHtml(
                QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                .arg(s.width())
                .arg(s.height()));
    mNewText->textCursor().insertFragment(fragment);
    mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
    mNewText->setVisible(true);
}

void MainWindow::on_listWidgetOriginalImgA_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QImage image;
    QString dir = dir_Original_A + QString("/") + current->text();
    if(QString::compare(dir, QString()) != 0) {
        image = QImage(dir);
        currentLeftImagePath = dir;
        ui->labelOrigImageA->setScaledContents(true);
        ui->labelOrigImageA->setPixmap(QPixmap::fromImage(image.scaled(125,125,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
        Q_UNUSED(previous);
   }
}

void MainWindow::on_listWidgetOriginalImgB_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QImage image;
    QString dir = dir_Original_B + QString("/") + current->text();
    if(QString::compare(dir, QString()) != 0) {
        image = QImage(dir);
        currentRightImagePath = dir;
        ui->labelOrigImageB->setScaledContents(true);
        ui->labelOrigImageB->setPixmap(QPixmap::fromImage(image.scaled(125,125,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
        Q_UNUSED(previous);
   }
}

void MainWindow::on_loadPairA_clicked()
{
    boost::filesystem::path pa("/home/emanuele/Desktop/printScreenA");

    main_graphFirst = QFileDialog::getExistingDirectory(this, tr("Choose an image directoy to load"),
            fileGraphLeft, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    imgadir = main_graphFirst;
    QDir drA(main_graphFirst);
    QStringList lA = drA.entryList(QDir::Files);
    ui->listWidgetPairA->addItems(lA);

//    for (int i = 0; i < lA.size(); i++)
//    {
//        ui->listWidgetPairA->setCurrentRow(i);
//        loadImagesGraphics(i);
//    }

    ui->loadPairB->setEnabled(true);
    ui->loadPairB->setStyleSheet("QPushButton{ background-color: green }");

//    ui->saveStep_A->setEnabled(true);
//    ui->saveStep_A->setStyleSheet(styleSheet());
//    ui->saveStep_B->setEnabled(true);
//    ui->saveStep_B->setStyleSheet(styleSheet());

    ui->saveToFileBtnCamA->setEnabled(true);
    ui->saveToFileBtnCamA->setStyleSheet(styleSheet());
    ui->saveToFileBtnCamB->setEnabled(true);
    ui->saveToFileBtnCamB->setStyleSheet(styleSheet());

//    ui->saveToFile_A->setEnabled(true);
//    ui->saveToFile_A->setStyleSheet(styleSheet());
//    ui->saveToFile_B->setEnabled(true);
//    ui->saveToFile_B->setStyleSheet(styleSheet());
}

void MainWindow::on_loadPairB_clicked()
{
    main_graphSecond = QFileDialog::getExistingDirectory(this, tr("Choose an image directoy to load"),
            fileGraphRight, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QDir drB(main_graphSecond);
    QStringList lB = drB.entryList(QDir::Files);
    ui->listWidgetPairB->addItems(lB);
    loadImagesGraphics(0);

//    for (int i = 0; i < lB.size(); i++)
//    {
//        ui->listWidgetPairB->setCurrentRow(i);
//        loadImagesGraphics(i);
//    }

    QSize s{32, 32};
    QTextDocumentFragment fragment;
    fragment = QTextDocumentFragment::fromHtml(
                QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                .arg(s.width())
                .arg(s.height()));
    mNewText->textCursor().insertFragment(fragment);
    mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
    mNewText->setVisible(true);

}

void MainWindow::yamlFileUpload(QString& filename)
{
    if(filename != QString(""))
    {
        loadedDirectory = filename;
        match.readFile(filename.toStdString());
        ui->labelYML_A->setText("Yml file loaded");
        ui->labelYML_B->setText("Yml file loaded");

        ui->loadPairA->setEnabled(true);
        ui->loadPairA->setStyleSheet("QPushButton{ background-color: green }");

    }
    else if(filename == QString(""))
    {
        QMessageBox::StandardButton q = QMessageBox::warning(this,"Error","Please upload .yml calibration file");
        return;
        Q_UNUSED(q);
    }
}

void MainWindow::loadImagesGraphics(int row)
{
    if(main_graphFirst.toStdString().size()>0&&main_graphSecond.toStdString().size()>0){
        if (main_graphFirst.toStdString().size() != main_graphSecond.toStdString().size())
        {
            QMessageBox::StandardButton q = QMessageBox::warning(this,"Error","Both list have different size");
            return;
            Q_UNUSED(q);
        }
        listPtr->setListsLoad(boost::filesystem::path(main_graphFirst.toStdString()),
                              boost::filesystem::path(main_graphSecond.toStdString()),BGR);

//        boost::filesystem::path pa("/home/emanuele/Desktop/printScreenA");
//        boost::filesystem::path pb("/home/emanuele/Desktop/printScreenB");

        yamlFileUpload(loadedDirectory);

        StereoImage im = listPtr->getImage(row);
        im.getA().toGrey();
        im.getB().toGrey();
        match.rectifyImages(im);

        StereoImage imageRectified;
        imageRectified = match.getRectImages();

        //finishedImages.append(imageRectified);

        QImage image = cvMat2QImage(imageRectified.getA().get8Bitmap());
        QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
        item->setZValue(0);
        mLeftScene->addItem(item);
        QImage imageB = cvMat2QImage(imageRectified.getB().get8Bitmap());
        QGraphicsPixmapItem* itemB = new QGraphicsPixmapItem(QPixmap::fromImage(imageB));
        itemB->setZValue(0);
        mRightScene->addItem(itemB);

        ui->graphicsViewLX->fitInView(mLeftScene->sceneRect(), Qt::KeepAspectRatio);
        ui->graphicsViewRX->fitInView(mRightScene->sceneRect(), Qt::KeepAspectRatio);

        ui->progressBarRect_A->setRange(0, finishedImages.size());
        //ui->progressBarRect_B->setValue(0, finishedImages.size());
        ui->graphicsViewLX->show();
        ui->graphicsViewRX->show();
        imageLoaded = true;
    }
    return;
}

void MainWindow::on_loadYMLFile_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Camera Cal File"),
                                       loadedDirectory, tr("yml (*.yml)"));
    yamlFileUpload(filename);

    QSize s{32, 32};
    QTextDocumentFragment fragment;
    fragment = QTextDocumentFragment::fromHtml(
                QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                .arg(s.width())
                .arg(s.height()));
    mNewText->textCursor().insertFragment(fragment);
    mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
    mNewText->setVisible(true);
}

void MainWindow::on_listWidgetPairA_currentRowChanged(int currentRow)
{
    if(ui->listWidgetPairB->count() > 0)
        ui->listWidgetPairB->setCurrentRow(currentRow);
}

void MainWindow::on_listWidgetPairB_currentRowChanged(int currentRow)
{
    if(ui->listWidgetPairA->count() > 0)
        ui->listWidgetPairA->setCurrentRow(currentRow);
    loadImagesGraphics(currentRow);
}

void MainWindow::on_saveToFileBtnCamA_clicked()
{
    recSaveA();

    QSize s{32, 32};
    QTextDocumentFragment fragment;
    fragment = QTextDocumentFragment::fromHtml(
                QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                .arg(s.width())
                .arg(s.height()));
    mNewText->textCursor().insertFragment(fragment);
    mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
    mNewText->setVisible(true);

}

void MainWindow::recSaveA()
{
    QString filenameRecA = QFileDialog::getExistingDirectory(this, tr("Save images"),
                                       loadedDirectory_RecA, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!filenameRecA.isEmpty())
    {
         loadedDirectory_RecA = filenameRecA;
         filenameRecA.toStdString();
         ui->labelDestFile_A->setText("Rectified images are saved in" + filenameRecA);
         ui->rectifyBtnA->setEnabled(true);
         ui->rectifyBtnA->setStyleSheet("QPushButton{ background-color: green }");
    }
    else if(filenameRecA == QString(""))
    {
        QMessageBox::StandardButton q = QMessageBox::warning(this,"Error","Please define destination folder");
        return;
        Q_UNUSED(q);
    }
}


void MainWindow::recSaveB()
{
    QString filenameRecB = QFileDialog::getExistingDirectory(this, tr("Save cropped images"),
                                       loadedDirectory_RecB, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!filenameRecB.isEmpty())
    {
         loadedDirectory_RecB = filenameRecB;
         filenameRecB.toStdString();
         ui->labelDestFile_B->setText("Rectified images are saved in" + filenameRecB);
         ui->rectifyBtnB->setEnabled(true);
         ui->rectifyBtnB->setStyleSheet("QPushButton{ background-color: green }");
    }
    else if(filenameRecB == QString(""))
    {
        QMessageBox::StandardButton q = QMessageBox::warning(this,"Error","Please define destination folder");
        return;
        Q_UNUSED(q);
    }
}


void MainWindow::on_saveToFileBtnCamB_clicked()
{
    recSaveB();

    QSize s{32, 32};
    QTextDocumentFragment fragment;
    fragment = QTextDocumentFragment::fromHtml(
                QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                .arg(s.width())
                .arg(s.height()));
    mNewText->textCursor().insertFragment(fragment);
    mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
    mNewText->setVisible(true);

}


void MainWindow::on_rectifyBtnA_clicked()
{
    boost::filesystem::path pa("/home/emanuele/Desktop/pA");

    for(int i=0;i<finishedImages.size();i++)
    {
        finishedImages[i].getA().setSavePath(pa);
        finishedImages[i].getA().save();
        ui->progressBarSaveA->setValue(i);
    }
}


void MainWindow::on_rectifyBtnB_clicked()
{
//    for(int i=0;i<finishedImages.size();i++)
//    {
//        finishedImages[i].getB().setSavePath(pb);
//        finishedImages[i].getB().save();
//        ui->progressBarSaveA->setValue(i);
//    }
}


void MainWindow::on_fitViewBtn_clicked()
{
    ui->graphicsViewLX->fitInView(mLeftScene->sceneRect(), Qt::KeepAspectRatio);
    ui->graphicsViewRX->fitInView(mRightScene->sceneRect(), Qt::KeepAspectRatio);    QSize s{32, 32};

    QTextDocumentFragment fragment;
    fragment = QTextDocumentFragment::fromHtml(
                QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                .arg(s.width())
                .arg(s.height()));
    mNewText->textCursor().insertFragment(fragment);
    mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
    mNewText->setVisible(true);

}






// Checking if the file-A and file-B exists already

bool MainWindow::fileExists(QString path) {
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    if (check_file.exists() && check_file.isFile()) {
        return true;
    } else {
        return false;
    }
}

void MainWindow::bothPrintScreenBtn(const QString& pathImg, bool checkFolder)
{
    QString suffix = ui->comboBoxFormat_A->currentText();
    QString outA;
    do{
       // outA = pathImg+"/printScreenA/"+ QString::number(counterA)+".png";
        outA = QString("%1/printScreenA/%2.%3").arg(pathImg).arg(counterA).arg(suffix);
        counterA++;
    }
    while((checkFolder && fileExists(outA)));
    QPixmap pixmapA = ui->graphicsViewLX->grab();
    //QImage imageA = ui->graphicsViewLX->grab().toImage();
    // QPixmap pixmapA = ui->graphicsViewLX->grab().toImage();
    pixmapA.save(outA);
    //imageA.save(outA);

    QString suffixB = ui->comboBoxFormat_B->currentText();
    QString outB;
    do{
        //outB = pathImg+"/printScreenB/"+ QString::number(counterB)+".png";
        outB = QString("%1/printScreenB/%2.%3").arg(pathImg).arg(counterA).arg(suffixB);
        counterB++;
    }
    while((checkFolder && fileExists(outB)));
    QPixmap pixmapB = ui->graphicsViewRX->grab();
    //QImage imageB = ui->graphicsViewRX->grab().toImage();
    //imageB.save(outB);
    pixmapB.save(outB);
}

void MainWindow::on_bothPrintScreenBtn_clicked()
{
    bothPrintScreenBtn("/home/emanuele/Desktop", !Lwrite);

    QSize s{32, 32};
    QTextDocumentFragment fragment;
    fragment = QTextDocumentFragment::fromHtml(
                QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                .arg(s.width())
                .arg(s.height()));
    mNewText->textCursor().insertFragment(fragment);
    mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
    mNewText->setVisible(true);

}


////////////////////////////////////////////////////////////////
// Print screen only for image A

bool MainWindow::fileExistsForSaveStep_A(QString path) {
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    if (check_file.exists() && check_file.isFile()) {
        return true;
    } else {
        return false;
    }
}

void MainWindow::printScreenA(const QString& pathImg, bool checkFolder)
{
    //QString outA;
    QString outA = QFileDialog::getExistingDirectory(this, tr("Save cropped images"),
                                       printA, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    do{
        outA = pathImg+ QString::number(counterAA)+".png";
        counterAA++;
    }
    while((checkFolder && fileExists(outA)));
    QImage imageA = ui->graphicsViewLX->grab().toImage();
    imageA.save(outA);
}

void MainWindow::on_saveStep_A_clicked()
{
    printScreenA("/home/emanuele/Desktop", !savePrintScreenA);

    QSize s{32, 32};
    QTextDocumentFragment fragment;
    fragment = QTextDocumentFragment::fromHtml(
                QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                .arg(s.width())
                .arg(s.height()));
    mNewText->textCursor().insertFragment(fragment);
    mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
    mNewText->setVisible(true);

}

//////////////////////////////////////////////
// Print screen only for image B

bool MainWindow::fileExistsForSaveStep_B(QString path) {
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    if (check_file.exists() && check_file.isFile()) {
        return true;
    } else {
        return false;
    }
}

void MainWindow::printScreenB(const QString& pathImg, bool checkFolder)
{
    //QString outB;
    QString outB = QFileDialog::getExistingDirectory(this, tr("Save cropped images"),
                                       printB, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    do{
        outB = pathImg+ QString::number(counterBB)+".png";
        counterBB++;
    }
    while((checkFolder && fileExists(outB)));
    QImage imageB = ui->graphicsViewRX->grab().toImage();
    imageB.save(outB);
}

void MainWindow::on_saveStep_B_clicked()
{
    printScreenB("/home/emanuele/Desktop", !savePrintScreenB);

    QSize s{32, 32};
    QTextDocumentFragment fragment;
    fragment = QTextDocumentFragment::fromHtml(
                QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                .arg(s.width())
                .arg(s.height()));
    mNewText->textCursor().insertFragment(fragment);
    mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
    mNewText->setVisible(true);

}
//////////////////////////////////////////////////////////////


void MainWindow::on_saveToFile_A_clicked()
{
    printScreenToFile_A("/home");

    QSize s{32, 32};
    QTextDocumentFragment fragment;
    fragment = QTextDocumentFragment::fromHtml(
                QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                .arg(s.width())
                .arg(s.height()));
    mNewText->textCursor().insertFragment(fragment);
    mNewText->append("\n"); // This will create a new line each time the user upload a different set of images

    mNewText->setVisible(true);

}

void MainWindow::printScreenToFile_A(const QString& pathImg)
{
    QString filename = QFileDialog::getExistingDirectory(this, tr("Save cropped images"),
                                       loadedDirectory, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!filename.isEmpty())
    {
         loadedDirectory = filename;
         filename.toStdString();
         ui->lblSaveA->setText(filename);
         ui->saveStep_A->setEnabled(true);
         ui->saveStep_A->setStyleSheet("QPushButton{ background-color: green }");
    }
    else if(filename == QString(""))
    {
        QMessageBox::StandardButton q = QMessageBox::warning(this,"Error","Please define destination folder");
        return;
        Q_UNUSED(q);
    }
}

void MainWindow::on_saveToFile_B_clicked()
{
    printScreenToFile_B("/home");

    QSize s{32, 32};
    QTextDocumentFragment fragment;
    fragment = QTextDocumentFragment::fromHtml(
                QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                .arg(s.width())
                .arg(s.height()));
    mNewText->textCursor().insertFragment(fragment);
    mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
    mNewText->setVisible(true);

}

void MainWindow::printScreenToFile_B(const QString& pathImg)
{
    QString filenameB = QFileDialog::getExistingDirectory(this, tr("Save cropped images"),
                                       loadedDirectoryB, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!filenameB.isEmpty())
    {
         loadedDirectoryB = filenameB;
         filenameB.toStdString();
         ui->lblSaveB->setText(filenameB);
         ui->saveStep_B->setEnabled(true);
         ui->saveStep_B->setStyleSheet("QPushButton{ background-color: green }");
    }
    else if(filenameB == QString(""))
    {
        QMessageBox::StandardButton q = QMessageBox::warning(this,"Error","Please define destination folder");
        return;
        Q_UNUSED(q);
    }
}


void MainWindow::on_fit_A_clicked()
{
    ui->graphicsViewLX->fitInView(mLeftScene->sceneRect(), Qt::KeepAspectRatio);

    QSize s{32, 32};
    QTextDocumentFragment fragment;
    fragment = QTextDocumentFragment::fromHtml(
                QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                .arg(s.width())
                .arg(s.height()));
    mNewText->textCursor().insertFragment(fragment);
    mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
    mNewText->setVisible(true);

}

void MainWindow::on_fit_B_clicked()
{
    ui->graphicsViewRX->fitInView(mRightScene->sceneRect(), Qt::KeepAspectRatio);

    QSize s{32, 32};
    QTextDocumentFragment fragment;
    fragment = QTextDocumentFragment::fromHtml(
                QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                .arg(s.width())
                .arg(s.height()));
    mNewText->textCursor().insertFragment(fragment);
    mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
    mNewText->setVisible(true);

}

void MainWindow::on_checkBoxScreen_A_toggled(bool checked)
{
    if(ui->checkBoxScreen_A->isEnabled()) {
        if(checked)
        {
            ui->checkBoxScreen_A->setText("Active");
            ui->saveToFile_A->setEnabled(true);
            ui->saveToFile_A->setStyleSheet("QPushButton{ background-color: green }");

            QTextDocumentFragment fragment;
            fragment = QTextDocumentFragment::fromHtml("<img src='/home/emanuele/Desktop/working.png'>");
            mNewText->textCursor().insertFragment(fragment);
            mNewText->setVisible(true);
        }
        else {
            ui->checkBoxScreen_A->setText("Inactive");
            ui->saveToFile_A->setEnabled(false);
            ui->saveToFile_A->setStyleSheet("QPushButton{ background-color: grey }");

            // Else add not active icon
            QSize s{32, 32};
            QTextDocumentFragment fragment;
            fragment = QTextDocumentFragment::fromHtml(
                        QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                        .arg(s.width())
                        .arg(s.height()));
            mNewText->textCursor().insertFragment(fragment);
            mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
            mNewText->setVisible(true);

        }
    }
}

void MainWindow::on_checkBoxScreen_B_toggled(bool checked)
{
    if(ui->checkBoxScreen_B->isEnabled())
    {
        if(checked)
        {
            ui->checkBoxScreen_B->setText("Active");
            ui->saveToFile_B->setEnabled(true);
            ui->saveToFile_B->setStyleSheet("QPushButton{ background-color: green }");

            QSize s{32, 32};
            QTextDocumentFragment fragment;
            fragment = QTextDocumentFragment::fromHtml(
                        QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                        .arg(s.width())
                        .arg(s.height()));
            mNewText->textCursor().insertFragment(fragment);
            mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
            mNewText->setVisible(true);

        }
        else
        {
            ui->checkBoxScreen_B->setText("Inactive");
            ui->saveToFile_B->setEnabled(false);
            ui->saveToFile_B->setStyleSheet("QPushButton{ background-color: grey }");

            // Else add not active icon
            QSize s{32, 32};
            QTextDocumentFragment fragment;
            fragment = QTextDocumentFragment::fromHtml(
                        QString(R"(<img src='/home/emanuele/Desktop/working.png' height="%1" width="%2">)")
                        .arg(s.width())
                        .arg(s.height()));
            mNewText->textCursor().insertFragment(fragment);
            mNewText->append("\n"); // This will create a new line each time the user upload a different set of images
            mNewText->setVisible(true);

        }
    }
}
