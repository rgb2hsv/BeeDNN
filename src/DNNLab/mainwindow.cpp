#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QClipboard>

#include <fstream>
using namespace std;

#include "SimpleCurveWidget.h"

#include "MLEngine.h"
#include "MLEngineBeeDnn.h"

#ifdef USE_TINYDNN
#include "MLEngineTinyDnn.h"
#endif

#include "DataSource.h"

#include "Activation.h"
#include "Optimizer.h"
#include "Loss.h"
#include "ConfusionMatrix.h"

//////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    vector<string> vsActivations;
    list_activations_available( vsActivations);

    ui->cbFunction->addItem("And");
    ui->cbFunction->addItem("Xor");
    ui->cbFunction->addItem("MNIST");
    ui->cbFunction->addItem("Fisher");
    ui->cbFunction->addItem("TextFiles");

    ui->cbFunction->insertSeparator(4);

    ui->cbFunction->addItem("Identity");
    ui->cbFunction->addItem("Sin");
    ui->cbFunction->addItem("Abs");
    ui->cbFunction->addItem("Parabolic");
    ui->cbFunction->addItem("Gamma");
    ui->cbFunction->addItem("Exp");
    ui->cbFunction->addItem("Sqrt");
    ui->cbFunction->addItem("Ln");
    ui->cbFunction->addItem("Gauss");
    ui->cbFunction->addItem("Inverse");
    ui->cbFunction->addItem("Rectangular");

    QStringList qsl;
    qsl+="LayerType";
    qsl+="InSize";
    qsl+="OutSize";
    qsl+="Arg1";

    ui->twNetwork->setHorizontalHeaderLabels(qsl);

    for(int i=0;i<10;i++)
    {
        QComboBox*  qcbType=new QComboBox;
        qcbType->addItem("");
        qcbType->addItem("DenseAndBias");
        qcbType->addItem("DenseNoBias");
        qcbType->addItem("Dropout");
        qcbType->addItem("GaussianNoise");
        qcbType->addItem("GlobalGain");
        qcbType->addItem("PoolAveraging1D");
        qcbType->addItem("SoftMax");

        qcbType->insertSeparator(8);

        for(unsigned int a=0;a<vsActivations.size();a++)
            qcbType->addItem(vsActivations[a].c_str());

        ui->twNetwork->setCellWidget(i,0,qcbType);

    }

    ui->twNetwork->setItem(0,1,new QTableWidgetItem("1")); //first input size is 1
    ui->twNetwork->adjustSize();

#ifdef USE_TINYDNN
    ui->cbEngine->addItem("tiny-dnn");
#endif

    _pEngine=0;
    _pDataSource=0;

    //setup loss
    vector<string> vsloss;
    list_loss_available(vsloss);
    for(unsigned int i=0;i<vsloss.size();i++)
        ui->cbLossFunction->addItem(vsloss[i].data());

    //setup optimizer
    vector<string> vsOptimizers;
    list_optimizers_available( vsOptimizers);
    for(unsigned int i=0;i<vsOptimizers.size();i++)
        ui->cbOptimizer->addItem(vsOptimizers[i].data());

    resizeDocks({ui->dockWidget},{1},Qt::Horizontal);
    _qsRegression=new SimpleCurveWidget;
    _qsRegression->addXAxis();
    _qsRegression->addYAxis();
    ui->layoutRegression->addWidget(_qsRegression);

    _qsLoss=new SimpleCurveWidget;
    _qsLoss->addXAxis();
    _qsLoss->addYAxis();
    ui->layoutLossCurve->addWidget(_qsLoss);

    _qsAccuracy=new SimpleCurveWidget;
    _qsAccuracy->addXAxis();
    _qsAccuracy->addYAxis();
    ui->layoutAccuracyCurve->addWidget(_qsAccuracy);

    init_all();
}
//////////////////////////////////////////////////////////////////////////
void MainWindow::init_all()
{
    ui->cbFunction->setCurrentIndex(6);
    ui->cbLossFunction->setCurrentIndex(0);
    ui->cbOptimizer->setCurrentText("Adam");

    ui->cbEngine->setCurrentIndex(0);
    ui->cbProblem->setCurrentIndex(0);

    for(int i=0;i<10;i++)
    {
        ((QComboBox*)(ui->twNetwork->cellWidget(i,0)))->setCurrentIndex(0);
        ui->twNetwork->setItem(i,1,new QTableWidgetItem(""));
        ui->twNetwork->setItem(i,2,new QTableWidgetItem(""));
    }

    _qsAccuracy->clear();
    _qsLoss->clear();
    _qsRegression->clear();
    ui->twConfusionMatrixTrain->resize(0,0);
    ui->peDetails->clear();

    _bMustSave=false;
    //   _sFileName="";

    _curveColor=0xff0000; //red
    set_input_size(1);

    delete _pEngine;
    _pEngine=new MLEngineBeeDnn;

    delete _pDataSource;
    _pDataSource=new DataSource;

    updateTitle();
}
//////////////////////////////////////////////////////////////////////////
MainWindow::~MainWindow()
{
    delete ui;
    delete _pEngine;
    delete _pDataSource;
}
//////////////////////////////////////////////////////////////////////////
void MainWindow::on_pushButton_clicked()
{
    train_and_test(true);
}
//////////////////////////////////////////////////////////////////////////
void MainWindow::train_and_test(bool bReset)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    compute_truth();

    if(bReset)
        ui_to_net();

    DNNTrainOption dto;
    dto.epochs=ui->leEpochs->text().toInt();
    dto.learningRate=ui->leLearningRate->text().toFloat();
    dto.batchSize=ui->leBatchSize->text().toInt();
    dto.keepBest=ui->cbKeepBest->isChecked();
    dto.decay=ui->leDecay->text().toFloat();
    dto.momentum=ui->leMomentum->text().toFloat();
    dto.optimizer=ui->cbOptimizer->currentText().toStdString();
    dto.lossFunction=ui->cbLossFunction->currentText().toStdString();
    dto.testEveryEpochs=ui->sbTestEveryEpochs->value();
    dto.reboostEveryEpoch=ui->leReboost->text().toInt();
    //dto.observer=nullptr;//&lossCB;

    if(bReset)
        _pEngine->init();

    _pEngine->set_problem(ui->cbProblem->currentText()=="Classification");
    DNNTrainResult dtr =_pEngine->learn(_pDataSource->train_data(),_pDataSource->train_annotation(),dto);

    float fLoss=_pEngine->compute_loss(_pDataSource->train_data(),_pDataSource->train_annotation()); //final loss
    ui->leMSE->setText(QString::number((double)fLoss));
    ui->leComputedEpochs->setText(QString::number(dtr.computedEpochs));
    ui->leTimeByEpoch->setText(QString::number(dtr.epochDuration));

    drawLoss(dtr.loss);
    drawAccuracy(dtr.accuracy);
    drawRegression();
    update_classification_tab();
    update_details();

    QApplication::restoreOverrideCursor();
}
//////////////////////////////////////////////////////////////////////////
void MainWindow::drawLoss(vector<double> vdLoss)
{
    if(!ui->cbHoldOn->isChecked())
        _qsLoss->clear();

    vector<double> x;
    for(unsigned int i=0;i<vdLoss.size();i++)
        x.push_back(i);

    _qsLoss->addHorizontalLine(0.);
    _qsLoss->addCurve(x,vdLoss,_curveColor);
}
//////////////////////////////////////////////////////////////////////////
void MainWindow::drawAccuracy(vector<double> vdAccuracy)
{
    if(!ui->cbHoldOn->isChecked())
        _qsAccuracy->clear();

    if(_pEngine->is_classification_problem())
    {
        ui->gbTrainAccuracy->setTitle("Train accuracy");

        // add 0%, 25% , 50%, 100%
        _qsAccuracy->addHorizontalLine(0.);
        _qsAccuracy->addHorizontalLine(25.);
        _qsAccuracy->addHorizontalLine(50.);
        _qsAccuracy->addHorizontalLine(75.);
        _qsAccuracy->addHorizontalLine(100.);

        //draw accuracy
        vector<double> x;
        for(unsigned int i=0;i<vdAccuracy.size();i++)
            x.push_back(i);

        _qsAccuracy->addCurve(x,vdAccuracy,_curveColor);
    }
    else
    {
        //draw euclidian distance
        ui->gbTrainAccuracy->setTitle("Train Euclidian distance");
    }
}
//////////////////////////////////////////////////////////////////////////
void MainWindow::drawRegression()
{
    _qsRegression->clear();

    if(_pEngine->is_classification_problem()==true)
    {   //not a regression problem
        return;
    }

    //create ref sample hi-res and net output
    unsigned int iNbPoint=100;
    float fInputMin=-4.f;
    float fInputMax=4.f;
    vector<double> vTruth;
    vector<double> vSamples;
    vector<double> vRegression;
    MatrixFloat mIn(1,1),mOut;

    compute_truth();

    float fVal=fInputMin;
    float fStep=(fInputMax-fInputMin)/(iNbPoint-1.f);

    for(unsigned int i=0;i<iNbPoint;i++)
    {
        mIn(0,0)=fVal;
        vTruth.push_back((double)(_pDataSource->train_annotation()(i,0)));
        vSamples.push_back((double)(fVal));
        _pEngine->predict(mIn,mOut);

        if(mOut.size()==0)
            return; //todo

        vRegression.push_back((double)(mOut(0)));
        fVal+=fStep;
    }

    _qsRegression->addHorizontalLine(0.);
    _qsRegression->addCurve(vSamples,vTruth,0xFF0000);
    _qsRegression->addCurve(vSamples,vRegression,0xFF);
}
//////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionQuit_triggered()
{
    if(ask_save())
        close();
}
//////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionAbout_triggered()
{
    QMessageBox mb;
    QString qsText="DNNLab";
    qsText+= "\n";
    qsText+= "\n GitHub: https://github.com/edeforas/BeeDNN";
    qsText+= "\n by Etienne de Foras";
    qsText+="\n email: etienne.deforas@gmail.com";

    mb.setText(qsText);
    mb.exec();
}
//////////////////////////////////////////////////////////////////////////
void MainWindow::compute_truth()
{  
    string sFunction=ui->cbFunction->currentText().toStdString();

    _pDataSource->load(sFunction);
    set_input_size(_pDataSource->data_cols());
}
//////////////////////////////////////////////////////////////////////////
void MainWindow::resizeEvent( QResizeEvent *e )
{
    (void)e;

    QMainWindow::resizeEvent(e);

    //todo resize curves

}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::update_details()
{
    if(_pEngine==nullptr)
    {
        ui->peDetails->clear();
        return;
    }

    string s;
    _pDataSource->write(s);
    s+="\n";
    _pEngine->write(s);

    ui->peDetails->setPlainText(s.c_str());
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::on_cbEngine_currentTextChanged(const QString &arg1)
{
    delete _pEngine;
    _pEngine=nullptr;

    if(arg1=="BeeDNN")
        _pEngine=new MLEngineBeeDnn;

#ifdef USE_TINYDNN
    if(arg1=="tiny-dnn")
        _pEngine=new DNNEngineTinyDnn;
#endif
    _bMustSave=true;
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::on_btnTrainMore_clicked()
{
    train_and_test(false);
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::net_to_ui()
{
    ui->cbFunction->setCurrentText(_pDataSource->name().c_str());

 //   for(int i=0;i<_pEngine->learn)



    //todo
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::ui_to_net()
{
    bool bOk;
    float fArg1=0.f;
    _pEngine->clear();
    int iLastOut=_iInputSize;
    for(int iRow=0;iRow<10;iRow++) //todo dynamic size
    {
        QComboBox* pCombo=(QComboBox*)(ui->twNetwork->cellWidget(iRow,0));
        if(!pCombo)
            continue;
        string sType=pCombo->currentText().toStdString();

        QTableWidgetItem* pwiInSize=ui->twNetwork->item(iRow,1); //todo not used in activation
        int iInSize;
        if(!pwiInSize)
            iInSize=iLastOut; //use last out
        else
        {
            int iIn=pwiInSize->text().toInt(&bOk);
            if(bOk)
                iInSize=iIn;
            else
                iInSize=iLastOut;
        }

        QTableWidgetItem* pwiOutSize=ui->twNetwork->item(iRow,2); //todo not used in activation
        int iOutSize;
        if(!pwiOutSize)
            iOutSize=iInSize; //same size (i.e. activation case)
        else
        {
            int iOut=pwiOutSize->text().toInt(&bOk);
            if(bOk)
                iOutSize=iOut;
            else
                iOutSize=iInSize;
        }

        iLastOut=iOutSize;

        QTableWidgetItem* pwArg1=ui->twNetwork->item(iRow,3);
        if(pwArg1)
            fArg1=pwArg1->text().toFloat(&bOk);
        else
            bOk=false;

        if(!sType.empty())
        {
            if(sType=="Dropout")
            {
                float fRatio=0.2f; //by default
                if(bOk)
                    fRatio=fArg1;
                _pEngine->add_dropout_layer(iInSize,fRatio);
            }
            else if(sType=="GaussianNoise")
            {
                float fStd=1.f; //by default
                if(bOk)
                    fStd=fArg1;
                _pEngine->add_gaussian_noise_layer(iInSize,fStd);
            }
            else if(sType=="GlobalGain")
            {
                float fGain=0.f; //by default, 0.f mean learned
                if(bOk)
                    fGain=fArg1;
                _pEngine->add_globalgain_layer(iInSize,fGain);
            }
            else if(sType=="PoolAveraging1D")
                _pEngine->add_poolaveraging1D_layer(iInSize,iOutSize);
            else if(sType=="DenseAndBias")
                _pEngine->add_dense_layer(iInSize,iOutSize,true);
            else if(sType=="DenseNoBias")
                _pEngine->add_dense_layer(iInSize,iOutSize,false);
            else
                _pEngine->add_activation_layer(sType);
        }
    }

    _pEngine->init();
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::on_cbYLogAxis_stateChanged(int arg1)
{
    (void)arg1;
    _qsLoss->setYLogAxis(ui->cbYLogAxis->isChecked());
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::on_buttonColor_clicked()
{
    QColorDialog qcd;
    qcd.setCurrentColor(_curveColor);
    qcd.exec();
    _curveColor=qcd.currentColor().rgb();
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::on_pushButton_2_clicked()
{
    _qsLoss->clear();
    _qsAccuracy->clear();
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::set_input_size(int iSize)
{
    _iInputSize=iSize;
    //ui->twNetwork->item(0,1)->setText(to_string(_iInputSize).data());
    ui->twNetwork->setItem(0,1,new QTableWidgetItem(to_string(_iInputSize).data())); //first input size is 1
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::update_classification_tab()
{
    if(!_pEngine->is_classification_problem())
    { // not a classification problem
        ui->twConfusionMatrixTrain->clearContents();
        ui->leTrainAccuracy->setText("n/a");
        ui->leTestAccuracy->setText("n/a");
        return;
    }

    float fAccuracy=0.f;
    if(_pDataSource->has_test_data())
    {
        _pEngine->compute_confusion_matrix(_pDataSource->test_data(),_pDataSource->test_annotation(),_mConfusionMatrix,fAccuracy);
        ui->leTestAccuracy->setText(QString::number((double)fAccuracy,'f',2));
    }
    else
        ui->leTestAccuracy->setText("n/a");

    _pEngine->compute_confusion_matrix(_pDataSource->train_data(),_pDataSource->train_annotation(),_mConfusionMatrix,fAccuracy);
    ui->leTrainAccuracy->setText(QString::number((double)fAccuracy,'f',2));

    drawConfusionMatrix(); //for now on train data
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::drawConfusionMatrix()
{
    if(_pEngine->is_classification_problem()==false)
    { // not a classification problem
        ui->twConfusionMatrixTrain->clearContents();
        ui->leTrainAccuracy->setText("n/a");
        ui->leTestAccuracy->setText("n/a");
        return;
    }

    ui->twConfusionMatrixTrain->setColumnCount((int)_mConfusionMatrix.cols());
    ui->twConfusionMatrixTrain->setRowCount((int)_mConfusionMatrix.rows());

    if(ui->cbConfMatPercent->isChecked())
    {
        MatrixFloat mConfMatPercent;
        ConfusionMatrix::toPercent(_mConfusionMatrix,mConfMatPercent);

        for(int c=0;c<_mConfusionMatrix.cols();c++)
            for(int r=0;r<_mConfusionMatrix.rows();r++)
                ui->twConfusionMatrixTrain->setItem(r,c,new QTableWidgetItem(QString::number((double)mConfMatPercent(r,c),'f',1)));
    }
    else
    {
        for(int c=0;c<_mConfusionMatrix.cols();c++)
            for(int r=0;r<_mConfusionMatrix.rows();r++)
                ui->twConfusionMatrixTrain->setItem(r,c,new QTableWidgetItem(to_string( (int)(_mConfusionMatrix(r,c))).data() ));
    }

    //colorize in yellow the diagonal
    for(int c=0;c<_mConfusionMatrix.cols();c++)
        ui->twConfusionMatrixTrain->item(c,c)->setBackgroundColor(Qt::yellow);
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::on_cbFunction_currentIndexChanged(int index)
{
    (void)index;
    string sFunction=ui->cbFunction->currentText().toStdString();

    if(sFunction=="And")
    {
        set_input_size(2);
        return;
    }

    if(sFunction=="Xor")
    {
        set_input_size(2);
        return;
    }

    if(sFunction=="MNIST")
    {
        set_input_size(784);
        return;
    }

    if(sFunction=="Fisher")
    {
        set_input_size(4);
        return;
    }

    _bMustSave=true;
    updateTitle();
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::on_cbConfMatPercent_stateChanged(int arg1)
{
    (void)arg1;
    drawConfusionMatrix();
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionSave_as_triggered()
{
    string sFileName = QFileDialog::getSaveFileName(this,tr("Save DNNLab File as"), ".", tr("DNNLab Files (*.dnnlab)")).toStdString();
    if(sFileName.empty())
        return;

    _sFileName=sFileName;

    save();

    _bMustSave=false;

    updateTitle();
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionNew_triggered()
{
    if(ask_save())
    {
        _sFileName="";
        init_all();
    }
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionOpen_triggered()
{
    if(ask_save())
    {
        string sFileName = QFileDialog::getOpenFileName(this,tr("Open DNNLab File"), ".", tr("DNNLab Files (*.dnnlab)")).toStdString();
        if(sFileName.empty())
            return;

        _sFileName=sFileName;

        load();
    }
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionSave_triggered()
{
    if(_sFileName.empty())
    {
        string sFileName = QFileDialog::getSaveFileName(this,tr("Save DNNLab File"), ".", tr("DNNLab Files (*.dnnlab)")).toStdString();
        if(sFileName.empty())
            return;

        _sFileName=sFileName;
    }

    save();
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionClose_triggered()
{
    if(ask_save())
    {
        _sFileName="";
        init_all();
    }
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionSave_with_Score_triggered()
{
    if(_sFileName.empty())
    {
        string sFileName = QFileDialog::getSaveFileName(this,tr("Save DNNLab File"), ".", tr("DNNLab Files (*.dnnlab)")).toStdString();
        if(sFileName.empty())
            return;

        _sFileName=sFileName;
    }

    //score=to_string() todo

    save();
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::on_pushButton_3_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->peDetails->toPlainText());
}
//////////////////////////////////////////////////////////////////////////////
bool MainWindow::save()
{
    string s;
    _pDataSource->write(s);
    _pEngine->write(s);

    ofstream out(_sFileName,ios::binary); //todo test

    out << s;

    _bMustSave=false;
    return true; //for now
}
//////////////////////////////////////////////////////////////////////////////
bool MainWindow::load()
{
    //todo use a file I/O class, properties?
    init_all();

    string s;
    ifstream fIn(_sFileName);
    string line;
    while(std::getline(fIn, line))
    {
        //concat lines without "="
        if(line.find("=")!=string::npos)
            s+="\n"+line;
        else
            s+=" "+line;
    }

    _pEngine->read(s);
    _pDataSource->read(s);

    net_to_ui();

    return true; //for now
}
//////////////////////////////////////////////////////////////////////////////
bool MainWindow::ask_save()
{
    if(!_bMustSave)
        return true;

    //todo

    {



    }
    /*
    string sFileName = QFileDialog::getSaveFileName(this,tr("Save DNNLab File"), ".", tr("DNNLab Files (*.dnnlab)")).toStdString();
    if(sFileName.empty())
        return;
*/
    //_sFileName=sFileName;


    _bMustSave=false;
    return true;
}
//////////////////////////////////////////////////////////////////////////////
void MainWindow::updateTitle()
{
    string sTitle="DNNLab ";

    if(!_sFileName.empty())
        sTitle+=_sFileName;

    if(_bMustSave)
        sTitle+=" *";

    setWindowTitle(sTitle.c_str());
}
//////////////////////////////////////////////////////////////////////////////
