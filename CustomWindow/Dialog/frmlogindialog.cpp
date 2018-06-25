#include "frmlogindialog.h"
#include "ui_frmlogindialog.h"
#include "myHelper.h"
#include <QPainter>
#include "macrodefs.h"
#include  <QStyleOption>
#include <QTextCodec>

#include "TraderCpMgr.h"
#include "app.h"
#include "Global.h"
#include "HJConstant.h"

#include <functional>
#include <QGraphicsDropShadowEffect>



#pragma execution_character_set("utf-8")



frmLoginDialog::frmLoginDialog(QWidget *parent) :
	StyleSheetDialog(parent),
    ui(new Ui::frmLoginDialog)
{
    ui->setupUi(this);
	initStyle();

	setLayout(ui->horizontalLayout_main);

	connect(ui->pushButton_login, SIGNAL(clicked()), this, SLOT(checkInfo()));
	connect(ui->pushButton_exit, SIGNAL(clicked()), this, SLOT(exitSys()));
	

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
	timer->start(400);

	//��ʼ����ʾ
	ui->label_note->setText("");
	QFont font("Microsoft YaHei UI", 10, QFont::Bold);
	ui->label_note->setFont(font);
	//pool = new ThreadPool();

	m_bLoginState = false;

	//����ģ��Ч��
	//QGraphicsDropShadowEffect *windowShadow = new QGraphicsDropShadowEffect;
	//windowShadow->setBlurRadius(9.0);
	//windowShadow->setColor(QColor("#20293a"));//palette().color(QPalette::Highlight));
	//windowShadow->setOffset(0.0);
	//this->setGraphicsEffect(windowShadow);

	connect(this, SIGNAL(notify(QString)), this, SLOT(OnNotify(QStirng)));
}


frmLoginDialog::~frmLoginDialog()
{
	//pool->Stop();

	timer->stop();

    delete ui;
}


void frmLoginDialog::initStyle()
{


#ifdef  _WIN32
        setWidgetStyleFromQssFile(this, _RES_STYLES_PATH + _MY_TEXT("DeepBlack\\logindialog.css"));
#else
        //kenny  20180305  ?????????
        QString strPath = QCoreApplication::applicationDirPath();
        QDir  dir;
        dir.setCurrent(strPath);

        setWidgetStyleFromQssFile(this, _RES_STYLES_PATH + _MY_TEXT("DeepBlack/logindialog.css"));
#endif

        this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
        this->setProperty("Form", true);
        this->setProperty("CanMove", true);

        ui->horizontalLayout_main->setGeometry(QRect(0,0,width(),height()));


        ui->label_login_png->setPixmap(QPixmap::fromImage(QImage(":/res/login")));
        ui->label_login_png->setFixedWidth(389);
        ui->label_login_png->setFixedHeight(598);
        QStringList list;

	list << tr("���ױ���") << tr("�����ʺ�");//<< tr("����֤") << tr("�۰�ͨ��֤");
	ui->comboBox_login_type->addItems(list);

	ui->textEdit_user_pswd->setEchoMode(QLineEdit::Password);
	ui->textEdit_userId->setText(App::user);
	
}

void frmLoginDialog::OnNotify(QString  str)
{
	ui->label_note->setText(str);
};

void frmLoginDialog::checkLogic()
{
	QString strPath = g_Global.GetSystemIniPath();
	QString strVal;
	App::GetPriProfileString(strPath, "info", "demo", strVal);
	if (strVal.contains("1"))
	{
		m_bLoginState = true;
		accept();
		return;
	}


	mNote = "��������֤��...";
	emit notify(mNote);

	QString userType;
	if (ui->comboBox_login_type->currentText() == "�ͻ���" || ui->comboBox_login_type->currentText() == "���ױ���")
	{
		userType = "1";
	}
	else
	{
		userType = "2";
	}



	bool bSigned = ui->checkBox->isChecked();

	mNote = "��ʼ��֤...";
	emit notify(mNote);
	//��֤
	int ret = g_TraderCpMgr.Authenticate(ui->textEdit_userId->text().toLatin1(),
								ui->textEdit_user_pswd->text().toLatin1(),
								App::ip.toLatin1(),
								App::port.toInt(),
								userType,
								bSigned);

	////��¼���ţ�����֤
	//ret = 0;
	if (YLINK_ERROR_NETWORK == ret)
	{
		mNote = "�û���¼��֤ʧ��";
	}
	else if (-1 == ret || -2 == ret)
	{
		mNote = "�û���¼��֤ʧ��";
	}
	else
	{
		//�������˻�����Ҫ���µ����˻�������
		bool bFind=false;
		for (size_t i = 0; i < App::accMgr.size(); i++)
		{
			if (ui->textEdit_userId->text() == App::accMgr.value(i).user)
				bFind = true;
		}
		if (!bFind)
		{
			int size = App::accMgr.size();
			App::WriteServerProperty("account", "ACCOUNT.count", QString::number(size+1));
			AccountMgr acc;
			acc.alias = ui->textEdit_userId->text();
			acc.user  = ui->textEdit_userId->text();
			acc.psw   = ui->textEdit_user_pswd->text();

			App::WriteServerProperty("account", QString("ACCOUNT.account%1.alias").arg(QString::number(size)), acc.alias);
			App::WriteServerProperty("account", QString("ACCOUNT.account%1.user").arg(QString::number(size)), acc.user);
			App::WriteServerProperty("account", QString("ACCOUNT.account%1.pswd").arg(QString::number(size)), acc.psw);

			App::accMgr.push_back(acc);
		}

		mNote = "��������¼...";
		emit notify(mNote);
		int iLoginResult = g_TraderCpMgr.UserLogin(userType, bSigned);
		if (iLoginResult == -1)
		{
			ui->label_note->setText("�û���¼ʧ��");
		}
		else if (iLoginResult == -2)
		{
			ui->label_note->setText("�û�����Ŀ¼ʧ�ܣ��޷���½");
		}
		// ��Ҫ�޸�����
		else if (iLoginResult == 1)
		{
			//Ӧ�ø�Ϊ��������ʾ���ã�������Ķ���
			ui->label_note->setText("��Ҫ�޸�������ܵ�½���Ƿ��޸ģ�");
			accept();
			m_bLoginState = true;
		}
		else
		{
			//timer->stop();

			mNote = "�û���¼�ɹ�";
			emit notify(mNote);
			m_bLoginState = true;
			accept();
			return;
		}


		////��¼ʧ�ܣ�ˢ����֤��
		ui->label_ensure_png->ShowCode();
		ui->textEdit_ensure_code->setText("");
		ui->textEdit_ensure_code->setFocus();

		return;

	}



}

//���߳���ִ���߼�����ʱ��ˢ�½�����ʾ
void frmLoginDialog::checkInfo()
{	

	//��ʼУ���û���,���룬������
	//ui->label_note->clear();
	//ui->label_note->adjustSize();
	mNote = "";
	if (CheckInput())
	{
		//�ɹ�
		success = true;

		// ��ʼ��
		g_TraderCpMgr.Init();

		//const std::function<void()> func1 = std::bind(&frmLoginDialog::checkLogic, this);
		doUpdateUiTask(std::bind(&frmLoginDialog::checkLogic, this));

		mNote = "��¼��...";
	}
	else
	{
		success = false;
		//StyleSheetDialog::reject();
	}


}

void frmLoginDialog::exitSys()
{
	success = false;
	close();
}

void frmLoginDialog::refreshCode()
{
	int test = 0;
}



bool frmLoginDialog::CheckInput()
{
	
	if (ui->textEdit_userId->text().isEmpty())
	{
		ui->label_note->setText("�û������ܺ��пո�");
		ui->label_note->adjustSize();
		this->repaint();
		ui->textEdit_userId->setFocus();
		return false;
	}

	
	if (ui->textEdit_user_pswd->text().isEmpty())
	{
		ui->label_note->setText("�û����벻�ܺ��пո�");
		ui->label_note->adjustSize();
		this->repaint();
		ui->textEdit_user_pswd->setFocus();
		return false;
	}


	if (ui->textEdit_ensure_code->text().isEmpty())
	{
		ui->label_note->setText("��֤�벻��Ϊ�գ�");
		ui->label_note->adjustSize();
		this->repaint();
		ui->textEdit_ensure_code->setFocus();
		return false;
	}

	int len = ui->textEdit_user_pswd->text().length();

	// ����ͨ�汾���루��¼���ʽ����룩ֻ��Ϊ6λ����
	if (len != 6)
	{
		ui->label_note->setText("��¼����ֻ��Ϊ6λ������");
		ui->label_note->adjustSize();
		this->repaint();
		return false;
	}


	// �ж���֤�������Ƿ���ȷ
	QString strTmp = ui->textEdit_ensure_code->text().toLower();
	QString strTmp2 = ui->label_ensure_png->m_strCode.toLower();
	
	if (strTmp != strTmp2)
	{
		ui->label_note->setText("��֤��������������룡");
		ui->label_note->adjustSize();
		this->repaint();

		ui->label_ensure_png->ShowCode();
		ui->textEdit_ensure_code->setText("");
		ui->textEdit_ensure_code->setFocus();
		return false;
	}

	return true;
}




void  frmLoginDialog::customEvent(QEvent *e)
{
	if (e->type() == WM_MYTASK)   //������Ϣ
	{
		OnUITask(0, 0);
	}
}

template<class F, class... Args>
auto frmLoginDialog::doUpdateUiTask(F&& f, Args&&... args)
->std::future<typename std::result_of<F(Args...)>::type>
{
	using return_type = typename std::result_of<F(Args...)>::type;

	auto task = std::make_shared< std::packaged_task<return_type()> >(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);

	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(queue_mutex);

		tasks.emplace([task]() { (*task)(); });
	}


	QApplication::postEvent(this,new QEvent(WM_MYTASK));

	return res;
}



int frmLoginDialog::OnUITask(void * wparam, void* lparam)
{
	std::unique_lock<std::mutex> lock(queue_mutex);
	while (tasks.size() > 0)
	{
		std::function<void()> task = std::move(this->tasks.front());
		tasks.pop();
		task();
	}

	return 1;
}



void frmLoginDialog::timerUpdate()
{
	ui->label_note->setText(mNote);
	//ui->label_note->update();
	update();
}



void frmLoginDialog::SetLoginMessage(const QString &Message)
{
	//
	mNote = Message;
}


TradeType  frmLoginDialog::GetType()
{
	return  TradeType::Real;
}


bool  frmLoginDialog::GetState()
{
	return  true;
}