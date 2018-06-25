/******************************************************************************
模块描述:各个报盘机进程主控调度模块
主要函数:Init(...)初始化函数
Finish() 结束清理
Run()主控线程函数
修改记录:
******************************************************************************/
#include <iostream>
//#include "Logger.h"
//#include "ConfigImpl.h"

#include "TraderCpMgr.h"
#include "TradePacket.h"
#include <QMessageBox>
#include "app.h"

#include "ProtocolConnectPoint.h"
#include "ProcessInterfaceYC.h"
#include <sstream>
#include <iomanip>
//
#include "TcpShortCp.h"
#include "ProcessInterfaceB1C.h"
#include "ProcessInterfaceB2C.h"
#include "ProcessInterfaceB3C.h"
#include "ProcessInterfaceB4C.h"
//
#include "ArrayListMsg.h"
//
//#include "MD5.h"
#include "Const.h"
#include "TradePacketAndStruct.h"
//
//#include "HsmClient.h"
//
#include "Mgr/VarietyMgr.h"
#include "Mgr/SystemParaMgr.h"
#include "Mgr/CodeTableMgrOld.h"
#include "Mgr/CodeTableMgr.h"
#include "Mgr/ProdCodeMgr.h"
//#include "PreOrder/StopPLMgr.h"
#include "Mgr/QuotationMgr.h"
#include "Mgr/PosiMgr.h"
#include "Mgr/TiHuoMgr.h"
#include "DelayReqHandler.h"
//
#include "Global.h"
#include "HJGlobalFun.h"
#include "TranMessage.h"
#include "HJCommon.h"
//#include "Log/MyLogEx.h"
#include "CustomInfoMgrEx.h"
#include "ClientTips.h"
#include "myhelper.h"
#include "app.h"
#include "Struct.h"
#include "HJGlobalFun.h"
#include <QMessageBox>
#include "IniFilesManager.h"
#include "QParamEvent.h"
#include <QTextCodec>

#ifdef _WIN32
    #include "MD5.h"
#else
    #include <openssl/md5.h>
#endif


CTraderCpMgr  g_TraderCpMgr;
HEADER_REQ    g_HeaderReq;
quint64       g_SeqNo = 0x9a1f3241;


#pragma execution_character_set("utf-8")

#define MenuIdByHand "M030100"
#define MenuIdByAuto "M030000"

// 模拟解冻iCancelHand手
//bool OrderFrozeInfo::CancelBal(int iCancelHand, double &dCancelBal)
//{
//	if(iTotal > 0)
//	{
//		iRemainAmt -= iCancelHand;
//		double dCancelBuy = CHJGlobalFun::DoubleFormat(dEntrBuy * iCancelHand / iTotal, 4);
//		double dReserve = CHJGlobalFun::DoubleFormat(dEntrReserve * iCancelHand / iTotal, 4);
//		double dExchFare = CHJGlobalFun::DoubleFormat(dEntrExchFare * iCancelHand / iTotal, 4);
//
//		// 模拟存储过程，这一步是否可以考虑去掉
//		dCancelBal = CHJGlobalFun::DoubleFormat(dCancelBuy+dReserve+dExchFare, 4);
//		//dCancelBal = dCancelBuy+dReserve+dExchFare;
//		// 模拟存储过程中执行p_trade_fund_unfroze的数据转换
//		dCancelBal = CHJGlobalFun::DoubleFormat(dCancelBal);
//	}
//	else
//	{
//		dCancelBal = 0.00;
//	}
//
//	return iRemainAmt > 0 ? false : true;
//}
//
//// 模拟解冻iCancelHand手
//bool OrderFrozeInfo::CancelBalForward(int iCancelHand, double &dCancelBal)
//{
//	if(iTotal > 0)
//	{
//		iRemainAmt -= iCancelHand;
//		double dCancelBuy = CHJGlobalFun::DoubleFormat(dEntrBuy * iCancelHand / iTotal, 4);
//		double dReserve = CHJGlobalFun::DoubleFormat(dEntrReserve * iCancelHand / iTotal, 4);
//		double dExchFare = CHJGlobalFun::DoubleFormat(dEntrExchFare * iCancelHand / iTotal, 4);
//
//		// 模拟存储过程，这一步是否可以考虑去掉
//		dCancelBal = CHJGlobalFun::DoubleFormat(dCancelBuy+dReserve+dExchFare, 4);
//		// 模拟存储过程中执行p_trade_fund_unfroze的数据转换
//		dCancelBal = CHJGlobalFun::DoubleFormat(dCancelBal);
//	}
//	else
//	{
//		dCancelBal = 0.00;
//	}
//
//	return iRemainAmt > 0 ? false : true;
//}
//
///*
//判断一个业务的报单流水里面是否有有效报单
//有则返回true，否则返回false
//*/
template<class QMapType>
bool HasValidOrder(const QMap<QString, QMapType> &QMapOrder)
{
	for (auto it = QMapOrder.begin(); it != QMapOrder.end(); it++)
	{
		if (it.value().status != "d")
			return true;
	}
	return false;
}
//
////去掉初始化结构内最后一行后面的逗号，并将EnumKeyUnknown修改为实际的接口ID
CTraderCpMgr::IfRouterCfg CTraderCpMgr::m_tblIfRouterCfg[] =
{
	//CmdID         To                      Obj
	{ "1001",        EnumKeyIfB2C,           0 },
	{ "1002",        EnumKeyIfB2C,           0 },
	{ "1004",        EnumKeyIfB2C,           0 },
	{ "1005",        EnumKeyIfB2C,           0 },
	{ "1006",        EnumKeyIfB2C,           0 },
	{ "1007",        EnumKeyIfB2C,           0 },
	{ "1020",        EnumKeyIfB2C,           0 },//客户信息
	{ "2022",        EnumKeyIfB2C,           0 },
	{ "2040",        EnumKeyIfB2C,           0 },
	{ "2201",        EnumKeyIfB2C,           0 },
	{ "3010",        EnumKeyIfB2C,           0 }, //项目中没有该结构
	{ "3021",        EnumKeyIfB2C,           0 },
	{ "3024",        EnumKeyIfB2C,           0 },
	{ "3064",        EnumKeyIfB4C,           0 }, // 风险通知单查询
	{ "3069",        EnumKeyIfB4C,           0 }, // 强平单查询
	{ "3099",        EnumKeyIfB4C,           0 }, // 风险度查询
	{ "3101",        EnumKeyIfB2C,           0 }, // 金联通出入金转账
	{ "4001",        EnumKeyIfB2C,           0 },
	{ "4011",        EnumKeyIfB2C,           0 },//add by xiao 用于报单 begin
	{ "4012",        EnumKeyIfB2C,           0 },
	{ "4021",        EnumKeyIfB2C,           0 },
	{ "4022",        EnumKeyIfB2C,           0 },
	{ "4041",        EnumKeyIfB2C,           0 },
	{ "4042",        EnumKeyIfB2C,           0 },
	{ "4043",        EnumKeyIfB2C,           0 },
	{ "4044",        EnumKeyIfB2C,           0 },
	{ "4045",        EnumKeyIfB2C,           0 },
	{ "4046",        EnumKeyIfB2C,           0 },
	{ "4047",        EnumKeyIfB2C,           0 },
	{ "4048",        EnumKeyIfB2C,           0 }, // end
	{ "4061",        EnumKeyIfB2C,           0 },
	{ "4071",        EnumKeyIfB2C,           0 },
	{ "5041",        EnumKeyIfB2C,           0 },
	{ "5101",        EnumKeyIfB2C,           0 },
	{ "5103",        EnumKeyIfB2C,           0 },
	{ "6001",        EnumKeyIfB3C,           0 },
	{ "6002",        EnumKeyIfB3C,           0 },
	{ "6005",        EnumKeyIfB2C,           0 },
	{ "6007",        EnumKeyIfB2C,           0 },
	{ "8001",        EnumKeyIfB2C,           0 },
	{ "8006",        EnumKeyIfB1C,           0 },
	{ "8007",        EnumKeyIfB2C,           0 },
	{ "8031",        EnumKeyIfB2C,           0 },
	{ "9010",        EnumKeyIfB2C,           0 },
	{ "9030",        EnumKeyIfB2C,           0 },
	{ "4074",        EnumKeyIfB2C,           0 },
	{ "3201",        EnumKeyIfB2C,           0 },
	{ "8002",        EnumKeyIfB2C,           0 },

#ifdef RUN_EMULATOR
	{ "3999",        EnumKeyIfB2C,           0 },
	{ "5999",        EnumKeyIfB2C,           0 },
	{ "2997",        EnumKeyIfB2C,           0 },
	{ "2996",        EnumKeyIfB2C,           0 },
	{ "2995",        EnumKeyIfB2C,           0 },
	{ "2994",        EnumKeyIfB2C,           0 },
	{ "8999",        EnumKeyIfB2C,           0 },
	{ "8998",        EnumKeyIfB2C,           0 },
	{ "8997",        EnumKeyIfB2C,           0 },
	{ "8897",        EnumKeyIfB2C,           0 },
	{ "2998",        EnumKeyIfB1C,           0 },
	{ "2999",        EnumKeyIfB1C,           0 },
#endif
};
//
CBroadcaster::CBroadcaster()
{

}
CBroadcaster::~CBroadcaster()
{

}
CBroadcaster::CBroadcaster(const CBroadcaster& bdr)
{
	*this = bdr;
}
CBroadcaster& CBroadcaster::operator=(const CBroadcaster& bdr)
{
	if (this != &bdr)
	{
		//m_arrint.RemoveAll();
		m_arrint.clear();

		for (int i = 0; i<bdr.m_arrint.size(); i++)
		{
			//m_arrint.insert(bdr.m_arrint.keys()[i], bdr.m_arrint.values()[i]);
			m_arrint = bdr.m_arrint;
			// m_arrint.(bdr.m_arrint[i]);
		}


	}

	return *this;
}


// 窗口句柄为int的窗口订阅消息
void CBroadcaster::Subscribe(QWidget * val)
{
	if (Find(val) < 0)
	{
		m_arrint.append(val);
	}
}
// 窗口句柄为int的窗口取消订阅消息
void CBroadcaster::Unsubscribe(QWidget * val)
{
	int i = Find(val);
	if (i >= 0)
	{
		m_arrint.removeAt(i);
	}
}

void CBroadcaster::Broadcast(UINT uMsg, void * wParam,void * lParam,BOOL bAsyn)
{
	//std::shared_ptr<QParamEvent> event = make_shared<QParamEvent>(QEvent::Type(uMsg));
	for (int i = 0; i<m_arrint.size(); i++)
	{
		QParamEvent *event = new QParamEvent(QEvent::Type(uMsg));
		event->setWParam(wParam);
		event->setLParam(lParam);
		if(wParam != nullptr)
			event->setOrderId(*(int *)wParam);
		if(lParam != nullptr)
			event->setInt(*(int *)lParam);
		//if (bAsyn)
		{
			QApplication::postEvent((QWidget *)m_arrint[i], event);
		}
		//else
		{
			//QApplication::sendEvent((QWidget *)m_arrint[i], event);
		}
	}

}

int CBroadcaster::Find(QWidget * val)
{
	int nRet = -1;
	for (int i = 0; i<m_arrint.size(); i++)
	{
		if (val == m_arrint[i])
		{
			nRet = i;
			break;
		}
	}

	return nRet;
}

//
CTraderCpMgr::CTraderCpMgr()
{
	m_pConfig = new CConfigImpl();

	m_nTimeOut = 30;
}

CTraderCpMgr::~CTraderCpMgr(void)
{
	if (0 != m_pConfig)
	{
	    delete m_pConfig;
	    m_pConfig = 0;
	}

	m_cPreOrder.bRun = false;

	CCommHandler::Instance()->Unsubscribe(this);
}

//报盘机连接点管理器初始化
int CTraderCpMgr::Init()
{
	m_bLoginSucceed = false;

	//将pCfg（QMap类型）保存的配置文件中key和value的映射关系放到m_Props
	QString csPath;
	myHelper::GetSystemIniPath(csPath);
	
	if (m_pConfig->Load(csPath.toStdString()) != 0)
	{
		return -1;
	}

	//参数设置
	g_Global.m_strNetAgent      = App::net_agent;
	g_Global.m_strBankNo        = App::bank_no;
	g_Global.m_strNetEnvionment = App::net_envionment;

	// 初始化公共请求报文头
	memset(&g_HeaderReq, 0, sizeof(HEADER_REQ));
	g_HeaderReq.SetMsgType("1");
	g_HeaderReq.SetMsgFlag("1");
	g_HeaderReq.SetTermType("01");
	g_HeaderReq.SetUserType("1");
	g_HeaderReq.SetAreaCode("    ");
	g_HeaderReq.SetBranchID("            ");
	g_HeaderReq.SetCTellerID1("          ");
	g_HeaderReq.SetCTellerID2("          ");

	//
	CCommHandler::Instance()->Init(csPath);
	CCommHandler::Instance()->Subscribe(this);

	//
	return 0;
}

//登录认证，登陆时用户名超过10位则截取前10位，此函数还要传递账户类型
int CTraderCpMgr::Authenticate(const char* pszUsr, const char* pszPwd, const char* pszAddr,
	int nPort, QString useridtype /* = "1"*/, bool bSign /*= false*/)
{
	// 检查是否已经通过认证
	if (g_Global.m_strSessionID != "" && g_Global.m_strSessionKey != "")
	{
		//return 0;
	}

	// 打开认证端口
	QString acPort = QString::number(nPort);

	// mod by Jerry Lee, 2013-4-2, 不使用代理时，转换域名到IP地址
	QString strIPAddr = pszAddr;

	//if (g_Global.m_nProxyType == 0)
	//{
	//    strIPAddr = GetIPFromDomain(pszAddr); 
	//}

	if (CCommHandler::Instance()->OpenInterfaceB1C(std::move(strIPAddr), std::move(acPort)) != 0)
	{
		return -1;
	}

	// 初始化路由表，将m_tblIfRouterCfg数组中的配置加载到m_tblIfRouter（QMap类型），对每个报文加载对应的接口类
	InitRouterTbl();

	// 请求报文头设置
	g_HeaderReq.SetUserID(pszUsr);
	g_HeaderReq.SetExchCode("8006");
	g_HeaderReq.SetSeqNo(g_SeqNo++);

	g_HeaderReq.SetTermType("03"); //表示登陆渠道（03为交易终端）
	g_HeaderReq.SetUserType("2");  //表示用户类型（2为客户）
	g_HeaderReq.SetUserID(pszUsr); //用户名
	g_HeaderReq.SetMsgFlag("1");   //请求报文标识（未知）
	g_HeaderReq.SetMsgType("1");   //交易报文类型（1为交易）

								   //8006请求报文初始化
	Req8006 stBodyReq;
	//memset(&stBodyReq, 0, sizeof(Req8006));//打开就报错

	myHelper::initLocalNet();

	stBodyReq.oper_flag = 1; // 操作标志    （1为登录认证）
	stBodyReq.user_type ="2"; // 用户类型，（1为操作员，2为客户，由于没有操作员的可能，所以没有定义到全局变量）
	stBodyReq.user_id_type = useridtype.toLatin1(); // 用户ID类型    当登录用户为客户时，此值有效。1：客户号2：银行账号
	stBodyReq.user_id = pszUsr; // 用户ID   
	stBodyReq.login_ip = myHelper::getHostIpAddress().toStdString(); //获取本机的ip地址     
	stBodyReq.net_agent = App::net_agent.toLatin1(); //  网络运营商id                   
	stBodyReq.bank_no = App::bank_no.toLatin1(); // 银行账号
	stBodyReq.net_envionment = App::net_envionment.toLatin1(); // 网络环境id 
	stBodyReq.user_pwd = GetEncryptPSW(pszPwd).toStdString();

	// add for 金联通
	stBodyReq.isSign = bSign ? "1" : "0";// 用户密码

	/*char cLog[100] = { 0 };
	sprintf(cLog, "登录ip：%s，登录端口：%d", pszAddr, nPort);
	qDebug("开始认证", cLog);*/

	Rsp8006 stBodyRsp;
	if (HandlePacket(stBodyRsp, stBodyReq, "8006") == 0)
	{
		if (CheckRspCode(stBodyRsp.rsp_code.c_str(), CHJGlobalFun::str2qstr(stBodyRsp.rsp_msg)))
		{
			//多点登陆
			ArrayListMsg moreLoginInfo = stBodyRsp.htm_more_login_list;
			int curLinkNum = 0;
			int maxLinkNum = 0;
			if (moreLoginInfo.size() > 0)
			{
				ArrayListMsg s = (ArrayListMsg)moreLoginInfo.GetValue(0);
				curLinkNum = CHJGlobalFun::str2qstr(s.GetStringEx(0)).toInt();
				maxLinkNum = CHJGlobalFun::str2qstr(s.GetStringEx(1)).toInt();
				if (curLinkNum != 0 && maxLinkNum != 0 && curLinkNum >= maxLinkNum)
				{
					QString  strTip;
					strTip = QString("当前您多点登录的在线数为%1d个，已超过最大连接数%2d个，为了让您正常登录，将强制签退一个在线交易端,是否继续？").arg(curLinkNum).arg(maxLinkNum);
					/*if (IDNO == ::QMessageBox(NULL, strTip, "提示", MB_YESNO|MB_ICONWARNING))
					return -1;*/

					if (QMessageBox::NoButton == QMessageBox::warning(NULL, "warning", strTip, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
						return -1;
				}
			}

			// 记录服务器信息
			SetServerList(stBodyRsp.htm_server_list);

			// 记录服务器信息到日志
			CCommHandler::Instance()->LogServerInfo();

			// 保存会话ID和会话密钥以及加密后的用户密码和用户ID（用户名称）
			g_Global.m_strUserID = pszUsr;
			g_Global.m_strSessionID = stBodyRsp.session_id.c_str();  //会话ID
			g_Global.m_strSessionKey = stBodyRsp.session_key.c_str(); //会话密钥
			g_Global.m_strPwdMD5 = stBodyReq.user_pwd.c_str();

			qDebug("认证成功");

			return 0;
		}
		else
		{
			qDebug("认证失败", stBodyRsp.rsp_msg);
			return -2;
		}
	}
	else //added by Jerry Lee, 2013-6-14, 网络数据处理失败
	{
		qDebug("认证失败", LOG_CONNECT_SERVER_FAIL_TIPS);
		return YLINK_ERROR_NETWORK;
	}


	return -1;
}

//#include "CBlockHouse.h"
// 用户登录，此函数还要传递账户类型
int CTraderCpMgr::UserLogin(const QString &sLoginMode /* = CONSTANT_LOGIN_TYPE_CODE*/, bool bSign /*= false*/)
{
	//检查是否已经通过认证
	if (g_Global.m_strSessionID == "" || g_Global.m_strSessionKey == "")
	{
		return -1;
	}

	//8006请求报文初始化
	Req8001 stBodyReq;
	memset(&stBodyReq, 0, sizeof(Req8001));

	// 读取配置文件
	QString csPath = g_Global.GetSystemIniPath();

	stBodyReq.oper_flag			= 1; // 操作标志    1：用户登录
	stBodyReq.user_type			= "2"; // 用户类型，（1为操作员，2为客户，由于没有操作员的可能，所以没有定义到全局变量）
	stBodyReq.login_server_code = "M006"; // 如何填充？ 服务器代码    客户当前处理业务使用的服务器代码
	stBodyReq.user_id           = g_Global.m_strUserID.toStdString(); // 用户ID    
	stBodyReq.user_pwd          = g_Global.m_strPwdMD5.toStdString(); // 用户密码   
	stBodyReq.bank_no           = g_Global.m_strBankNo.toStdString(); // 银行账号
	stBodyReq.ver_num           = App::ver_num.toStdString(); // 客户端版本号 
	stBodyReq.is_down_para      = App::is_down_para.toStdString(); // 是否下载参数信息    如果客户端的版本号与服务器端的最新版本号不一致，强制下载。响应报文的字段20开始

	stBodyReq.user_id_type      = sLoginMode.toStdString(); //用户ID类型    当登录用户为客户时，此值有效。1：客户号2：银行账号
	stBodyReq.login_ip          = myHelper::getHostIpAddress().toStdString(); //获取本机的ip地址
	stBodyReq.session_id        = g_Global.m_strSessionID.toStdString();

	stBodyReq.isSign = "0";


	CCommHandler* pCommHandler = CCommHandler::Instance();

	pCommHandler->OpenInterfaceB2C();

	Rsp8001 stBodyRsp;
	int iRet = -1;
	// 遍历服务器列表，尝试登录
	for (int i = 0; i < pCommHandler->GetServerCount(); i++)
	{
		iRet = HandlePacket(stBodyRsp, stBodyReq, "8001");
		if (iRet == 0)
		{
			break;
		}
	}

	if (iRet == 0)
	{
		if (CCommHandler::Instance()->OpenInterfaceB3C() == 0
			&& CCommHandler::Instance()->OpenInterfaceB4C() == 0)
		{
			if (CheckRspCode(stBodyRsp.rsp_code.c_str(), CHJGlobalFun::str2qstr(stBodyRsp.rsp_msg)))
			{
				// 唤起升级程序 
				CallUpdateExe();

				// 记录登陆用户名
				//::WritePrivateProfileQString("Display", "cust_code", stBodyReq.user_id, g_Global.GetUserIniPath());
				//::WritePrivateProfileQString("Display", "LoginMode", sLoginMode, g_Global.GetUserIniPath());

				// added by Jerry Lee, 2013-02-28
				//  g_Global.ChangeUserIniFile(stBodyReq.user_id);

				g_Global.m_strExchDate = stBodyRsp.exch_date.c_str();         // 系统交易日
				g_Global.m_strLast_Exch_Date = stBodyRsp.last_exch_date.c_str();    // 上一交易日
				g_Global.m_strSysState = stBodyRsp.m_sys_stat.c_str();        // 系统状态ID
				g_Global.m_strUserName = stBodyRsp.user_name.c_str();         // 客户简称，比如"张零一"
				g_Global.m_strBranchName = CHJGlobalFun::str2qstr( stBodyRsp.branch_name); ;//stBodyRsp.branch_name.c_str();       // 客户所属代理机构名称，比如"乌商行２"
				g_Global.m_almTake_Man_List = stBodyRsp.alm_take_man_list; // 客户提货人列表
				g_Global.m_strAcctType = stBodyRsp.acct_type.c_str();         // 账户类型

				g_Global.m_strNo_Affirm_Rpt_Num = stBodyRsp.no_affirm_rpt_num; //
				g_Global.m_almNoRead_Bulletin_List = stBodyRsp.alm_noread_bulletin_list;
				g_Global.m_strUserID = stBodyRsp.user_id.c_str();

				g_HeaderReq.SetBranchID(stBodyRsp.branch_id); //所属代理机构
				g_HeaderReq.SetUserID(stBodyRsp.user_id);     //如果请求报文中为银行号的话，后台会将银行号转换为交易编码

															  // 记录登陆用户名
															  //::WritePrivateProfileQString("Display", "cust_code", stBodyReq.user_id, g_Global.GetUserIniPath());
															  //::WritePrivateProfileQString("Display", "LoginMode", sLoginMode, g_Global.GetUserIniPath());

//#if (defined _VERSION_JSZX) || (defined _VERSION_ZHLHY)
//															  // 设置出入金模式
//				SetTransferMode(stBodyRsp.alm_menu_info);
//#endif
				// 是否有权限采用预埋单
				g_Global.m_bUseSerPreOrder = stBodyRsp.auto_entry_menu_id == "M000000" ? true : false;

				// 是否需要升级
				bool bNeedUpdate = (stBodyReq.is_down_para == "1") ? true : CompareVersion(App::ver_num, stBodyRsp.version.c_str(), 3);

				// 更新版本信息
				if (bNeedUpdate)
					RecordVersionInfo(stBodyRsp.version.c_str());

				// 将必要数据写入内存
				{
					// 初始化交割品种代码
					g_VarietyMgr.Initialize(m_vVariety, bNeedUpdate ? &stBodyRsp.alm_variety_list : NULL);

					// 初始化码表
					g_CodeTableMgrOld.Initialize(bNeedUpdate ? &stBodyRsp.alm_code_table_list : NULL);

					g_CodeTableMgr.Initialize(bNeedUpdate ? &stBodyRsp.alm_code_table_list : NULL);

					// 初始化品种信息
					g_ProdCodeMgr.Initialize(m_vProdCode, bNeedUpdate ? &stBodyRsp.alm_prod_code_list : NULL);

					// 委托管理m_QMapSystemPara变量，临时方案，以后m_QMapSystemPara要录入到g_SysParaMgr管理
					g_SysParaMgr.Initialize( m_QMapSystemPara );

					g_PosiMgr.Initialize(m_QMapDeferPosi, m_QMapStoreInfo);
				}

				// 初始化报单序列号
				IniOrderSerial();

				// 获取费率模板信息
				GetRsp2040();

				// 初始化系统配置
				IniSystemSetting();

				// 初始化行情品种
				IniQuotationInsID();

				// 初始化止盈止损管理类
				//g_StopPLMgr.Initialize();

				// 初始化行情管理类
				g_QuoMgr.Initialize(m_QMapQuotation);
				/*if (g_Global.m_bForceTerminate)
				{
				return -2;
				}*/
				// 请求服务器预埋单最大数量，解决在广播调用交易接口堵死后主界面请求数据假死的问题
				GetParaValue("MaxEntrAmount", "0");

				// 启动延迟刷新线程
				QString sBroadcastDelayActionTime = GetParaValue("BroadcastDelayActionTime", "700");
				
///////////请求客户初始化信息///////////////////////////////////////////////////////////////
				g_DelayReqHandler.Initlize(CHJGlobalFun::qstr2str(sBroadcastDelayActionTime));

				// 登陆成功后请求一次客户信息
				g_DelayReqHandler.AddCustomInfoReq();
//////////////////////////////////////////////////////////////////////////
				 //打开广播端口，要在获取客户昨日信息后才能请求广播，否则自动计算会出错
				if(CCommHandler::Instance()->OpenInterfaceYCEx() != 0)
				{
					QMessageBox::warning(nullptr, "登录提示", "打开广播端口失败!", QMessageBox::Ok);
					return -1;
				}

				// 启动预埋单管理线程
				//m_cAutoSendThread.Set(&m_cPreOrder);
				//m_cAutoSendThread.Start();
				//通过线程池解决自动线程
				pool.AddTask(std::bind(&CPreOrder::CommitAskAll, &m_cPreOrder));

				if (g_Global.m_bShowLastLoginInfo)
				{
					//警告信息  
					if (stBodyRsp.last_login_date.length() > 0 && stBodyRsp.last_lan_ip.length() > 0)
					{
						QString strMsg;
						if (stBodyRsp.exist_login == CONSTANT_YES_NO_YES)
						{

							strMsg = QString("您已有一个相同的用户登陆！\n对方登录时间：%1 %2\n对方登录IP地址：%3")\
								.arg( CHJGlobalFun::str2qstr(stBodyRsp.last_login_date))\
								.arg(CHJGlobalFun::str2qstr(stBodyRsp.last_login_time))\
								.arg(CHJGlobalFun::str2qstr(stBodyRsp.last_lan_ip ));
						
						}
						else
						{
							strMsg = QString("上次登录时间：%1 %2\n上次登录IP地址：%3")\
								.arg(CHJGlobalFun::str2qstr(stBodyRsp.last_login_date))\
								.arg(CHJGlobalFun::str2qstr(stBodyRsp.last_login_time))\
								.arg(CHJGlobalFun::str2qstr(stBodyRsp.last_lan_ip));

						}
						QMessageBox::warning(nullptr,"登录提示",strMsg, QMessageBox::Ok);

					}
					if (stBodyRsp.more_login_msg.length() > 0)
					{
						QString str = QString("多点登录用户登录记录：\n %1 ").arg(CHJGlobalFun::str2qstr(stBodyRsp.more_login_msg));

						QMessageBox::warning(nullptr, "登录提示", str, QMessageBox::Ok);

						//QParamEvent *msg = new QParamEvent(WM_SHOW_ORDER_TIPS);
						//msg->setWParam(&str);

						//QApplication::sendEvent(m_hMain, new QEvent(WM_SHOW_ORDER_TIPS));


					}
				}

				m_bLoginSucceed = true;

				//WLog(E_LOG_NORMAL,"客户%s登录成功，登陆SessionID：%s", g_Global.m_strUserID,g_Global.m_strSessionID);

				return stBodyRsp.need_change_pwd == CONSTANT_YES_NO_YES ? 1 : 0;
			}
			else
			{
				qDebug("登录失败", stBodyRsp.rsp_msg);
			}
		}
	}
	else
	{
		qDebug("登录失败", LOG_CONNECT_SERVER_FAIL_TIPS);
	}
	return -1;
}
//
//#ifdef Q_OS_WIN
//
//int CTraderCpMgr::setMainwnd(HWND wnd)
//{
//	// 记录主窗口句柄，解决线程发消息直接调用AfxGetMainWnd()可能导致句柄错误的问题
//	m_hMain = wnd;
//
//	return 1;
//}
//#elif  defined Q_OS_UNIX
//int CTraderCpMgr::setMainwnd(mainWindow* wnd)
//{
//	// 记录主窗口句柄，解决线程发消息直接调用AfxGetMainWnd()可能导致句柄错误的问题
//	m_hMain = wnd;
//
//	return 1;
//}
//
//#endif


// 发送验密报文，屏幕解锁
int CTraderCpMgr::UnlockScreen(const QString &pwd, const QString &useridtype /* = CONSTANT_LOGIN_TYPE_CODE */)
{
#ifdef _VERSION_GF
	Req8007 stBodyReq;
	Rsp8007 stBodyRsp;

	stBodyReq.oper_flag = 1;
	stBodyReq.user_type = "2";
	stBodyReq.user_id_type = (useridtype == "1") ? "1" : "2";
	stBodyReq.user_id = g_Global.m_strUserID;
	// 	char *szPWD = NULL;
	// 	szPWD = pwd.GetBuffer();
	stBodyReq.user_pwd = GetEncryptPSW(pwd);
	// 	pwd.ReleaseBuffer();
	stBodyReq.login_ip = CCommHandler::Instance()->GetLocalIP();
	stBodyReq.bank_no = g_Global.m_strBankNo;
	stBodyReq.isSign = "0";

	if (HandlePacket(stBodyRsp, stBodyReq, "8007") == 0)
	{
		return CheckRspCode(stBodyRsp.rsp_code, stBodyRsp.rsp_msg) ? 0 : -1;
	}
	else
	{
		return YLINK_ERROR_NETWORK;
	}

#else
	if (GetEncryptPSW(pwd.toLatin1()) == g_Global.m_strPwdMD5)
		return 0;
	else
		return -1;
#endif
}

// 获取代码表中某一类型下的某一id下对应的值(codedesc)，由于现不保存码表，废弃
QString CTraderCpMgr::GetCode_desc(QString codetype, QString codeid)
{
	for (size_t i = 0; i < m_vCode.size(); i++)
	{
		MyCodeInfo mystruct = m_vCode.at(i);
		if (mystruct.code_type == codetype && mystruct.code_id == codeid)
		{
			return mystruct.code_desc.toLatin1();
		}
	}
	return "";
}

// 获取代码表中某一类型下的某一名称下对应的id(codedesc)，由于现不保存码表，废弃
QString CTraderCpMgr::GetCode_id(QString codetype, QString codedesc)
{
	for (size_t i = 0; i < m_vCode.size(); i++)
	{
		MyCodeInfo mystruct = m_vCode.at(i);
		if (mystruct.code_type == codetype && mystruct.code_desc == codedesc)
		{
			return mystruct.code_id;
		}
	}
	return "";
}

void CTraderCpMgr::ClearSession()
{
	g_Global.m_strSessionID = "";
	g_Global.m_strSessionKey = "";
}

//用户退出
//CProcessInterfaceYC 关闭长连接 先close 在finish
//释放资源
int CTraderCpMgr::UserLogout()
{
	//检查是否已经通过认证
	if (g_Global.m_strSessionID != "" && g_Global.m_strSessionKey != "")
	{
		//return 0;
	}

	Req8002 stBodyReq;
	memset(&stBodyReq, 0, sizeof(Req8002));
	stBodyReq.oper_flag = 1; //操作标志    1：用户登录   原来是0
	stBodyReq.user_type = "1"; //用户类型    user_type
	stBodyReq.user_id = g_Global.m_strUserID.toStdString(); //用户ID    
	stBodyReq.session_id = g_Global.m_strSessionID.toStdString(); //用户密码    

	Rsp8002 stBodyRsp;
	if (HandlePacket(stBodyRsp, stBodyReq, "8002", 20) == 0)
	{
		//LOG("请求退出成功");
		m_bLoginSucceed = false;
		return 0;
	}
	else
	{
		//qDebug("请求退出失败",stBodyRsp.rsp_msg);
	}

	return -1;
}

// 
QString CTraderCpMgr::GetSessionID()
{
	return g_Global.m_strSessionID;
}


// 初始化路由表
int CTraderCpMgr::InitRouterTbl()
{
	//配置表
	int nSize = sizeof(m_tblIfRouterCfg) / sizeof(IfRouterCfg);

	//根据路由配置表初始化内存路由表
	for (int m = 0; m < nSize; m++)
	{
		QString sCmdID = m_tblIfRouterCfg[m].sCmdID;
		switch (m_tblIfRouterCfg[m].ulIfTo)
		{
		case EnumKeyIfB1C:
			m_tblIfRouter[sCmdID] = &CCommHandler::Instance()->m_pCpInterfaceB1C;
			break;
		case EnumKeyIfB2C:
			m_tblIfRouter[sCmdID] = &CCommHandler::Instance()->m_pCpInterfaceB2C;
			break;
		case EnumKeyIfB3C:
			m_tblIfRouter[sCmdID] = &CCommHandler::Instance()->m_pCpInterfaceB3C;
			break;
		case EnumKeyIfB4C:
			m_tblIfRouter[sCmdID] = &CCommHandler::Instance()->m_pCpInterfaceB4C;
			break;
		default:
			m_tblIfRouter[sCmdID] = NULL;
			break;
		}
	}

	return 0;
}

CConnectPointSync* CTraderCpMgr::FindRoute(const QString &sCmdID)
{
	auto it = m_tblIfRouter.find(sCmdID);
	if (it != m_tblIfRouter.end())
	{
		return *(it.value());
	}
	return 0;
}



// 交易 接口 [6002]普通查询 的业务实现
int CTraderCpMgr::Tran6002Handle(HEADER_REQ& stHeaderReq, Req6002& stBodyReq, HEADER_RSP& stHeaderRsp,
	Rsp6002& stBodyRsp, QVector< pair<QString, QString> > & v, unsigned int uiTimeout)
{
	CTradePacket oPktReq;
	oPktReq.SetHeader(stHeaderReq);
	CTradePacketAndStruct::Struct2Packet(stBodyReq, oPktReq);

	for (size_t i = 0;i<v.size();i++)
	{
		QByteArray ba = v.at(i).first.toLatin1();
		QByteArray bb = v.at(i).second.toLatin1();
		oPktReq.AddParameter(ba.data(), bb.data());
	}

	CConnectPointSync* pCp = 0;
	QString sCmdID = CHJGlobalFun::str2qstr(oPktReq.GetCmdID());
	pCp = FindRoute(sCmdID);
	if (0 == pCp)
		return -2;

	//发送报文
	CTradePacket oPktRsp;
	int nRtn = 0;
	//qDebug( sCmdID, "发送报文" );
	nRtn = pCp->SendPacket(oPktReq,oPktRsp,uiTimeout);
	if (0 != nRtn)
	{
		CConnectPointSync** pCp = m_tblIfRouter[sCmdID];
		nRtn =  CCommHandler::Instance()->SendPacket(pCp, &oPktReq, &oPktRsp, uiTimeout);
		if (0 != nRtn)
		{
			//qDebug( sCmdID, LOG_CONNECT_SERVER_FAIL_TIPS );
			return nRtn;
		}
	}

	oPktRsp.GetHeader(stHeaderRsp);
	CTradePacketAndStruct::Packet2Struct(stBodyRsp, oPktRsp);

	//qDebug(sCmdID,stHeaderRsp.rsp_code);
	return 0;
};

const QUOTATION* CTraderCpMgr::Translate(CBroadcastPacket& oPktSrc)
{
	QUOTATION *qt = NULL;

	string sZipVal;
	string sInstID;
	string sName;
	oPktSrc.GetParameterVal("instID", sInstID);
	oPktSrc.GetParameterVal("name", sName);

	auto it = m_QMapQuotation.find(sInstID);
	if (it != m_QMapQuotation.end())
	{
		QUOTATION& stQuotation = it->second;
		qt = &stQuotation;

		// 记录原行情，如果新行情有误，则用其来恢复
		/*QUOTATION stTempQt = stQuotation;*/
		if (0 != oPktSrc.GetParameterVal("sZipBuff", sZipVal))
		{
			auto vKey = oPktSrc.GetKeys();
			if (vKey.size() > 2)
			{
				TranslateUnzipPacket(oPktSrc, stQuotation);
			}
			else
			{
				return NULL;
			}
		}
		else
		{
			TranslateZipPacket(oPktSrc, stQuotation);
		}
		/*if (!CheckQt(stQuotation))
		{
		m_QMapQuotation[sInstID] = stTempQt;
		return NULL;
		}*/
	}
	else
	{
		QUOTATION stQuotation;
		if (0 != oPktSrc.GetParameterVal("sZipBuff", sZipVal))
		{
			auto vKey = oPktSrc.GetKeys();
			if (vKey.size() > 2)
			{
				TranslateUnzipPacket(oPktSrc, stQuotation);
			}
			else
			{
				return NULL;
			}
		}
		else
		{
			TranslateZipPacket(oPktSrc, stQuotation);
		}

		stQuotation.instID = sInstID;
		stQuotation.name = sName;
		m_QMapQuotation[sInstID] = stQuotation;
		qt = &m_QMapQuotation[sInstID];
	}
	return qt;
}

bool CTraderCpMgr::CheckQt(const QUOTATION& stQuotation)
{
	// 价格从低到高添加到数组（买2 买1 卖1 卖2）
	QVector<unsigned int> vPrice;
	AddPrice(vPrice, stQuotation.m_Bid[1].m_uiPrice);
	AddPrice(vPrice, stQuotation.m_Bid[0].m_uiPrice);
	AddPrice(vPrice, stQuotation.m_Ask[0].m_uiPrice);
	AddPrice(vPrice, stQuotation.m_Ask[1].m_uiPrice);
	if (vPrice.size() > 1)
	{
		for (size_t i = 0; i < vPrice.size() - 1; i++)
		{
			if (vPrice[i] >= vPrice[i + 1])
			{
				return false;
			}
		}
	}

	// 如果最新价不在买卖一价之间，则为无效行情，由于此算法未确定，暂时注释
	/*if( stQuotation.m_uiLast > 0 )
	{
	if( stQuotation.m_Ask[0].m_uiPrice > 0 )
	{
	if( stQuotation.m_uiLast > stQuotation.m_Ask[0].m_uiPrice )
	{
	return false;
	}
	}

	if( stQuotation.m_Bid[0].m_uiPrice > 0 )
	{
	if( stQuotation.m_uiLast < stQuotation.m_Bid[0].m_uiPrice )
	{
	return false;
	}
	}
	}*/
	return true;
}


int CTraderCpMgr::TranslateUnzipPacket(CBroadcastPacket& oPktSrc, QUOTATION& stQuotation)
{
	//
	QString strTmp = "";
	if (0 == oPktSrc.GetParameterVal("name", strTmp.toStdString()))
	{
		stQuotation.name = strTmp.toStdString();
	}

	if (0 == oPktSrc.GetParameterVal("sequenceNo", strTmp.toStdString()))
	{
		stQuotation.m_uiSeqNo = strTmp.toUInt();
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("quoteDate", strTmp.toStdString()))
	{
		stQuotation.m_uiDate = strTmp.toUInt();
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("quoteTime", strTmp.toStdString()))
	{
		//QVector<QString> v = strutils::explode(":",strTmp);
		QStringList  v = strTmp.split(QRegExp(":"));
		if (v.size() == 3)
		{
			int nHour = v[0].toInt();
			int nMin = v[1].toInt();
			int nSec = v[2].toInt();
			//stQuotation.m_uiTime = nHour * 10000 + nMin * 100 + nSec;
			stQuotation.m_uiTime = nHour * 10000000 + nMin * 100000 + nSec * 1000 + 0;
		}
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("open", strTmp.toStdString()))
	{
		//stQuotation.m_uiOpenPrice = RoundToInt<double>(FromQString<double>(strTmp),2);
		stQuotation.m_uiOpenPrice = strTmp.toDouble();
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("high", strTmp.toStdString()))
	{
		//stQuotation.m_uiHigh = RoundToInt<double>(FromQString<double>(strTmp),2);
		stQuotation.m_uiHigh = strTmp.toDouble();
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("low", strTmp.toStdString()))
	{
		//stQuotation.m_uiLow = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("lastClose", strTmp.toStdString()))
	{
		//stQuotation.m_uilastClose = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("last", strTmp.toStdString()))
	{
		//stQuotation.m_uiLast = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("volume", strTmp.toStdString()))
	{
		//stQuotation.m_uiVolume = (unsigned int)(FromQString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("close", strTmp.toStdString()))
	{// 今收盘
	 //stQuotation.m_uiClose = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("ask1", strTmp.toStdString()))
	{
		//stQuotation.m_Ask[0].m_uiPrice = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("ask2", strTmp.toStdString()))
	{
		//stQuotation.m_Ask[1].m_uiPrice = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("ask3", strTmp.toStdString()))
	{
		//stQuotation.m_Ask[2].m_uiPrice = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("ask4", strTmp.toStdString()))
	{
		//stQuotation.m_Ask[3].m_uiPrice = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("ask5", strTmp.toStdString()))
	{
		//stQuotation.m_Ask[4].m_uiPrice = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("askLot1", strTmp.toStdString()))
	{
		//stQuotation.m_Ask[0].m_uiVol = (unsigned int)(FromQString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("askLot2", strTmp.toStdString()))
	{
		//stQuotation.m_Ask[1].m_uiVol = (unsigned int)(FromQString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("askLot3", strTmp.toStdString()))
	{
		//stQuotation.m_Ask[2].m_uiVol = (unsigned int)(FromQString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("askLot4", strTmp.toStdString()))
	{
		//stQuotation.m_Ask[3].m_uiVol = (unsigned int)(FromQString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("askLot5", strTmp.toStdString()))
	{
		//stQuotation.m_Ask[4].m_uiVol = (unsigned int)(FromQString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("bid1", strTmp.toStdString()))
	{
		//stQuotation.m_Bid[0].m_uiPrice = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("bid2", strTmp.toStdString()))
	{
		//stQuotation.m_Bid[1].m_uiPrice = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("bid3", strTmp.toStdString()))
	{
		//stQuotation.m_Bid[2].m_uiPrice = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("bid4", strTmp.toStdString()))
	{
		//stQuotation.m_Bid[3].m_uiPrice = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("bid5", strTmp.toStdString()))
	{
		//stQuotation.m_Bid[4].m_uiPrice = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("bidLot1", strTmp.toStdString()))
	{
		//stQuotation.m_Bid[0].m_uiVol = (unsigned int)(FromQString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("bidLot2", strTmp.toStdString()))
	{
		//stQuotation.m_Bid[1].m_uiVol = (unsigned int)(FromQString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("bidLot3", strTmp.toStdString()))
	{
		//stQuotation.m_Bid[2].m_uiVol = (unsigned int)(FromQString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("bidLot4", strTmp.toStdString()))
	{
		//stQuotation.m_Bid[3].m_uiVol = (unsigned int)(FromQString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("bidLot5", strTmp.toStdString()))
	{
		//stQuotation.m_Bid[4].m_uiVol = (unsigned int)(FromQString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("turnOver", strTmp.toStdString()))
	{//再 /１００００
	 //stQuotation.m_dbTurnOver = (FromQString<double>(strTmp)/10000);
	 //stQuotation.m_uiTurnOver = RoundToInt<double>(FromQString<double>(strTmp),-2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("highLimit", strTmp.toStdString()))
	{// 涨停板
	 //stQuotation.m_uiHighLimit = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("lowLimit", strTmp.toStdString()))
	{// 跌停板
	 //stQuotation.m_uiLowLimit = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("average", strTmp.toStdString()))
	{// 跌停板
	 //stQuotation.m_uiAverage = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("weight", strTmp.toStdString()))
	{// 跌停板
	 //stQuotation.m_uiWeight = RoundToInt<double>(FromQString<double>(strTmp),3);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("Posi", strTmp.toStdString()))
	{
		//stQuotation.m_uiChiCangLiang = (unsigned int)(FromQString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("settle", strTmp.toStdString()))
	{
		//stQuotation.m_uiSettle = RoundToInt<double>(FromQString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("lastSettle", strTmp.toStdString()))
	{
		//stQuotation.m_uiLastSettle = RoundToInt<double>(FromQString<double>(strTmp),2);
	}
	return 0;
}

int CTraderCpMgr::TranslateZipPacket(CBroadcastPacket& oPktSrc, QUOTATION& stQuotation)
{
	string sZipVal;
	string sInstID;
	oPktSrc.GetParameterVal("instID", sInstID);
	oPktSrc.GetParameterVal("sZipBuff", sZipVal);

	unsigned int uiTime = 0;

	//去掉\r\n
	sZipVal = strutils::stripNewLines(sZipVal);

	//base64解码
	sZipVal = strutils::base64Decode(sZipVal);

	//int nLen = CEncode::unbase64(const_cast<char*>(sZipVal),sZipVal.length(),aZipData);
	//aZipData 按照多个FLV串接而成 即第一字节的6bit表示FieldID,随后2bit的值+3=后续实际值占用字节数,实际值按照网络字节序

	int nIdx = 0;
	char cField;
	char cLen;
	char acValues[6];
	//unsigned int uiValue;
	INT64 uiValue;
	unsigned char* lpPointer = (unsigned char*)sZipVal.data();
	int nLen = sZipVal.length();
	char cByte;

	while (nIdx < nLen)
	{
		cByte = lpPointer[nIdx];
		// BIT:0 - 5
		cField = (cByte & 0xFC) >> 2;//11111100;
									 // BIT:6 - 7   x + 3
		cLen = (cByte & 0x03) + 3;
		if (nIdx + cLen >= nLen)
		{
			//CRLog(E_ERROR,"行情报文长度异常!");
			return false;
		}

		nIdx++;
		memset(acValues, 0, 6);
		memcpy(acValues, &lpPointer[nIdx], cLen);
		nIdx += cLen;
		double dbValue = 0.0;
		for (int i = 0; i < cLen; i++)
		{
			unsigned char chTmp = acValues[i];

			for (int j = 7; j >= 0; j--)
			{
				unsigned int nIndex = ((cLen - i - 1) * 8 + j);
				int nFlag = ((chTmp >> j) & 0x01);
				if (nFlag > 0)
				{
					dbValue += pow(2.0, (double)nIndex);
				}
			}
		}
		//uiValue = (unsigned int)dbValue;
		uiValue = (INT64)dbValue;
		switch (cField)
		{
		case FIELDKEY_LASTSETTLE:
			stQuotation.m_uiLastSettle = uiValue / 10;
			break;
		case FIELDKEY_LASTCLOSE:
			stQuotation.m_uilastClose = uiValue / 10;
			break;
		case FIELDKEY_OPEN:
			stQuotation.m_uiOpenPrice = uiValue / 10;
			break;
		case FIELDKEY_HIGH:
			stQuotation.m_uiHigh = uiValue / 10;
			break;
		case FIELDKEY_LOW:
			stQuotation.m_uiLow = uiValue / 10;
			break;
		case FIELDKEY_LAST:
			stQuotation.m_uiLast = uiValue / 10;
			break;
		case FIELDKEY_CLOSE:
			stQuotation.m_uiClose = uiValue / 10;
			break;
		case FIELDKEY_SETTLE:
			stQuotation.m_uiSettle = uiValue / 10;
			break;
		case FIELDKEY_BID1:
			stQuotation.m_Bid[0].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_BIDLOT1:
			stQuotation.m_Bid[0].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_BID2:
			stQuotation.m_Bid[1].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_BIDLOT2:
			stQuotation.m_Bid[1].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_BID3:
			stQuotation.m_Bid[2].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_BIDLOT3:
			stQuotation.m_Bid[2].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_BID4:
			stQuotation.m_Bid[3].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_BIDLOT4:
			stQuotation.m_Bid[3].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_BID5:
			stQuotation.m_Bid[4].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_BIDLOT5:
			stQuotation.m_Bid[4].m_uiVol = uiValue / 1000;
			break;


		case FIELDKEY_BID6:
			stQuotation.m_Bid[5].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_BIDLOT6:
			stQuotation.m_Bid[5].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_BID7:
			stQuotation.m_Bid[6].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_BIDLOT7:
			stQuotation.m_Bid[6].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_BID8:
			stQuotation.m_Bid[7].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_BIDLOT8:
			stQuotation.m_Bid[7].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_BID9:
			stQuotation.m_Bid[8].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_BIDLOT9:
			stQuotation.m_Bid[8].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_BID10:
			stQuotation.m_Bid[9].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_BIDLOT10:
			stQuotation.m_Bid[9].m_uiVol = uiValue / 1000;
			break;



		case FIELDKEY_ASK1:
			stQuotation.m_Ask[0].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_ASKLOT1:
			stQuotation.m_Ask[0].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_ASK2:
			stQuotation.m_Ask[1].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_ASKLOT2:
			stQuotation.m_Ask[1].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_ASK3:
			stQuotation.m_Ask[2].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_ASKLOT3:
			stQuotation.m_Ask[2].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_ASK4:
			stQuotation.m_Ask[3].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_ASKLOT4:
			stQuotation.m_Ask[3].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_ASK5:
			stQuotation.m_Ask[4].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_ASKLOT5:
			stQuotation.m_Ask[4].m_uiVol = uiValue / 1000;
			break;

		case FIELDKEY_ASK6:
			stQuotation.m_Ask[5].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_ASKLOT6:
			stQuotation.m_Ask[5].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_ASK7:
			stQuotation.m_Ask[6].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_ASKLOT7:
			stQuotation.m_Ask[6].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_ASK8:
			stQuotation.m_Ask[7].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_ASKLOT8:
			stQuotation.m_Ask[7].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_ASK9:
			stQuotation.m_Ask[8].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_ASKLOT9:
			stQuotation.m_Ask[8].m_uiVol = uiValue / 1000;
			break;
		case FIELDKEY_ASK10:
			stQuotation.m_Ask[9].m_uiPrice = uiValue / 10;
			break;
		case FIELDKEY_ASKLOT10:
			stQuotation.m_Ask[9].m_uiVol = uiValue / 1000;
			break;

		case FIELDKEY_VOLUME:
			stQuotation.m_uiVolume = uiValue / 1000;
			break;
		case FIELDKEY_WEIGHT:
			stQuotation.m_uiWeight = uiValue / 10;
			break;
		case FIELDKEY_HIGHLIMIT:
			stQuotation.m_uiHighLimit = uiValue / 10;
			break;
		case FIELDKEY_LOWLIMIT:
			stQuotation.m_uiLowLimit = uiValue / 10;
			break;
		case FIELDKEY_POSI:
			stQuotation.m_uiChiCangLiang = uiValue / 1000;
			break;
		case FIELDKEY_UPDOWN: // 之前不对该字段做处理
							  //stQuotation.m_uiUpDown = uiValue /10;
			break;
		case FIELDKEY_TURNOVER:
			stQuotation.m_uiTurnOver = uiValue / 10;
			break;
		case FIELDKEY_AVERAGE:
			stQuotation.m_uiAverage = uiValue / 10;
			break;
		case FIELDKEY_SEQUENCENO:
			stQuotation.m_uiSeqNo = uiValue / 1000;
			break;
		case FIELDKEY_QUOTETIME: //次字段处理需要再确认
			stQuotation.m_uiTime = uiValue;
			break;
		//case FIELDKEY_QUOTEDATE: //次字段处理需要再确认
		//	stQuotation.m_uiDate = uiValue;
		//	break;
		case FIELDKEY_UPDOWNRATE: // 之前不对该字段做处理
								  //stQuotation.m_uiUpDownRate = uiValue;
			break;
		default:
			CRLog(E_ERROR,"行情报文字段序号[%d]不存在!",cField);
			break;
		}
	}

	// 计算涨跌 （最新价-昨结（昨收））/最新价
	if (stQuotation.m_uiLast != 0)
	{
		if (stQuotation.m_uiLastSettle != 0)
		{
			stQuotation.m_uiUpDown = stQuotation.m_uiLast - stQuotation.m_uiLastSettle;
			stQuotation.m_dUpDownRate = stQuotation.m_uiUpDown / (double)stQuotation.m_uiLastSettle * 100;
		}
		else
		{
			stQuotation.m_uiUpDown = stQuotation.m_uiLast - stQuotation.m_uilastClose;
			stQuotation.m_dUpDownRate = stQuotation.m_uiUpDown / (double)stQuotation.m_uilastClose * 100;
		}
	}
	else
	{
		stQuotation.m_uiUpDown = 0;
		stQuotation.m_dUpDownRate = 0.00;
	}

	// 计算涨跌幅度
	return 0;
}

// 接受到行情广播报文的时候统一调用的函数
int CTraderCpMgr::DealRecvQuotation(CBroadcastPacket& pkt, const int& iBroadcastID, bool bPosi_PL /*= false*/)
{
	const QUOTATION* pQuotation = Translate(pkt);
	if (pQuotation == NULL)
	{
		// 解码失败
		return -1;
	}
	else
	{
		const string &sInstID = pQuotation->instID;
		if (bPosi_PL)
		{
			// 计算持仓盈亏
			//g_CusInfo.HandleRecvQuotation(*pQuotation);

			// 处理
			//m_cPreOrder.HandleRecQuotation( sInstID, pQuotation->m_uiLast );

#ifdef _WRITE_LOG
			/*
			static QString csLog;
			static double dFactor = g_Global.m_dQTFactor;;
			csLog.Format("%s 最新价%.2f 买一价%.2f 卖一价%.2f", sInstID, pQuotation->m_uiLast/dFactor, pQuotation->m_Bid[0].m_uiPrice/dFactor, pQuotation->m_Ask[0].m_uiPrice/dFactor);
			g_Global.WriteLog(csLog);
			*/
#endif
		}

		// 广播消息
		 m_mapBdr[iBroadcastID].Broadcast(MSG_RECV_QUOTATION, (void *)pQuotation, nullptr, false);//&sInstID
	}

	return 0;
}

CBroadcaster* CTraderCpMgr::GetBroadcaster(int nBdrType)
{
	QMapBDR::iterator it = m_mapBdr.find(nBdrType);
	if (it != m_mapBdr.end())
	{
		return &(it.value());// &(it.value);
	}
	else
	{
		return nullptr;
	}

	return nullptr;
}


void CTraderCpMgr::Subscribe(int nBdrMsgType, QWidget* val)
{
	m_mapBdr[nBdrMsgType].Subscribe(val);
}

void CTraderCpMgr::Unsubscribe(int nBdrMsgType, QWidget* val)
{
	m_mapBdr[nBdrMsgType].Unsubscribe(val);
}


/*考虑独立出一个全局公用类来实现，比如CHJGlobalFun********************************************************************************************************/
bool CTraderCpMgr::CheckRspCode(const QString &strCode, QString &strMsg, bool bShowMsg /*= true*/)
{
	if (strCode == "00000000")
	{
		return true;
	}
	else
	{
		if (bShowMsg)
		{
			if (strMsg.length() <= 0)
			{
				QMessageBox::warning(NULL, "错误提示", "网络通讯异常!", QMessageBox::Yes, QMessageBox::Yes);
			}
			else
			{

				QMessageBox::warning(NULL, "错误提示", strMsg, QMessageBox::Yes, QMessageBox::Yes);
			}
		}

		return false;
	}
}
/*获取CTraderCpMgr的内存变量********************************************************************************************************/
// 根据交割品种ID获取交割品种名称
QString CTraderCpMgr::GetVarietyName(const QString &sID)
{
	for (size_t i = 0; i < m_vVariety.size(); i++)
	{
		if (m_vVariety.at(i).variety_id == sID)
		{
			return m_vVariety.at(i).name;
		}
	}

	return sID;
}
// 根据交易类型ID获取交易类型名称
QString CTraderCpMgr::GetExchName(const QString &sID)
{
	if (sID == "4061")
	{
		return "撤单";// 由于为了combo的需要，去掉了撤单，所以在这里要补齐
	}

	for (size_t i = 0; i < m_vExchCode.size(); i++)
	{
		if (m_vExchCode.at(i).code_id == sID)
		{
			return m_vExchCode.at(i).code_desc;
		}
	}

	return sID;
}

// 根据交易类型ID获取交易类型名称
QString CTraderCpMgr::GetExchIDFromName(const QString &sName)
{
	if (sName == "撤单")
	{
		return "4061";// 由于为了combo的需要，去掉了撤单，所以在这里要补齐
	}

	for (size_t i = 0; i < m_vExchCode.size(); i++)
	{
		if (m_vExchCode.at(i).code_desc == sName)
		{
			return m_vExchCode.at(i).code_id;
		}
	}

	return sName;
}
// 根据委托状态ID获取委托状态名称
QString CTraderCpMgr::GetEntr_statName(const QString &sID)
{
	for (size_t i = 0; i < g_TraderCpMgr.m_vEntrState.size(); i++)
	{
		if (g_TraderCpMgr.m_vEntrState.at(i).code_id == sID)
			return g_TraderCpMgr.m_vEntrState.at(i).code_desc;
	}

	return sID;
}
// 根据复核状态ID获取复核状态名称
QString CTraderCpMgr::GetCheck_staName(const QString &sID)
{
	for (size_t i = 0; i < m_vCheckState.size(); i++)
	{
		if (m_vCheckState.at(i).code_id == sID)
		{
			return m_vCheckState.at(i).code_desc;
		}
	}

	return sID;
}

// 根据账户状态ID获取账户状态名称
QString CTraderCpMgr::GetAcct_statName(const QString &sID)
{
	for (size_t i = 0; i < m_vAcctState.size(); i++)
	{
		if (m_vAcctState.at(i).code_id == sID)
			return m_vAcctState.at(i).code_desc;
	}

	return sID;
}
// 根据证件ID获取证件名称
QString CTraderCpMgr::GetIdTypeName(const QString &sID)
{
	for (size_t i = 0; i < m_vCertType.size(); i++)
	{
		if (m_vCertType.at(i).code_id == sID)
			return m_vCertType.at(i).code_desc;
	}

	return sID;

}
// 根据证件名称获得证件id
QString CTraderCpMgr::GetIdTypeId(const QString &sName)
{
	for (size_t i = 0; i < g_TraderCpMgr.m_vCertType.size(); i++)
	{
		if (g_TraderCpMgr.m_vCertType.at(i).code_desc == sName)
			return g_TraderCpMgr.m_vCertType.at(i).code_id;
	}
	return sName;
}
// 根据提货状态ID获取提货状态名称
QString CTraderCpMgr::GetTake_statName(const QString &sID)
{
	for (size_t i = 0; i < g_TraderCpMgr.m_vTakeState.size(); i++)
	{
		if (g_TraderCpMgr.m_vTakeState.at(i).code_id == sID)
			return g_TraderCpMgr.m_vTakeState.at(i).code_desc;
	}
	return sID;
}
// 根据提货类型ID获取提货类型名称
QString CTraderCpMgr::GetDraw_typeName(const QString &sID)
{
	for (size_t i = 0; i < m_vDrawType.size(); i++)
	{
		if (m_vDrawType.at(i).code_id == sID)
			return m_vDrawType.at(i).code_desc;
	}
	return sID;
}
//
QString CTraderCpMgr::GetFCOrderStateName(const QString &sID)
{
	for (size_t i = 0; i < m_vFCOrderState.size(); i++)
	{
		if (m_vFCOrderState.at(i).code_id == sID)
		{
			return m_vFCOrderState.at(i).code_desc;
		}
	}
	return sID;
}

// 根据提货状态名称获取提货状态ID
QString CTraderCpMgr::GetTake_statID(const QString &sName)
{
	for (size_t i = 0; i < g_TraderCpMgr.m_vTakeState.size(); i++)
	{
		if (g_TraderCpMgr.m_vTakeState.at(i).code_desc == sName)
			return g_TraderCpMgr.m_vTakeState.at(i).code_id;
	}
	return sName;
}

QString CTraderCpMgr::GetMarketName(const QString &sID)
{
	for (size_t i = 0; i < g_TraderCpMgr.m_vMarketId.size(); i++)
	{
		if (g_TraderCpMgr.m_vMarketId.at(i).code_id == sID)
			return g_TraderCpMgr.m_vMarketId.at(i).code_desc;
	}
	return sID;
}

QString CTraderCpMgr::GetOffset_flagName(const QString &sID)
{
	for (size_t i = 0; i < g_TraderCpMgr.m_vOffsetFlag.size(); i++)
	{
		if (g_TraderCpMgr.m_vOffsetFlag.at(i).code_id == sID)
		{
			return g_TraderCpMgr.m_vOffsetFlag.at(i).code_desc;
		}
	}
	return sID;
}

QString CTraderCpMgr::GetDeli_flagName(const QString &sID)
{
	for (size_t i = 0; i < g_TraderCpMgr.m_vDeliFlag.size(); i++)
	{
		if (g_TraderCpMgr.m_vDeliFlag.at(i).code_id == sID)
		{
			return g_TraderCpMgr.m_vDeliFlag.at(i).code_desc;
		}
	}
	return sID;
}

QString CTraderCpMgr::GetBuySellName(const QString &sID)
{
	if (sID == "s")
	{
		return "卖";
	}
	else if (sID == "b")
	{
		return "买";
	}
	return sID;
}

QString CTraderCpMgr::GetCancel_flagName(const QString &sID)
{
	for (size_t i = 0;i < g_TraderCpMgr.m_vCancelFlag.size(); i++)
	{
		if (g_TraderCpMgr.m_vCancelFlag.at(i).code_id == sID)
		{
			return g_TraderCpMgr.m_vCancelFlag.at(i).code_desc;
		}
	}
	return sID;
}

QString CTraderCpMgr::GetAccessWayName(const QString &sID)
{
	for (size_t i = 0;i < g_TraderCpMgr.m_vAccessWay.size(); i++)
	{
		if (g_TraderCpMgr.m_vAccessWay.at(i).code_id == sID)
		{
			return g_TraderCpMgr.m_vAccessWay.at(i).code_desc;
		}
	}
	return sID;
}

QString CTraderCpMgr::GetIn_account_flagName(const QString &sID)
{
	if (sID == "1")
	{
		return "是";
	}
	else
	{
		return sID;
	}
}

QString CTraderCpMgr::GetLongShortName(const QString &sID)
{
	for (size_t i = 0; i < g_TraderCpMgr.m_vLongShort.size(); i++)
	{
		if (g_TraderCpMgr.m_vLongShort.at(i).code_id == sID)
		{
			return g_TraderCpMgr.m_vLongShort.at(i).code_desc;
		}
	}
	return sID;
}

QString CTraderCpMgr::GetTerm_typeName(const QString &sID)
{
	for (size_t i = 0; i < g_TraderCpMgr.m_vTermType.size(); i++)
	{
		if (g_TraderCpMgr.m_vTermType.at(i).code_id == sID)
		{
			return g_TraderCpMgr.m_vTermType.at(i).code_desc;
		}
	}
	return sID;
}


/*登录后的数据处理相关********************************************************************************************************/
// 比较本地版本号与服务器版本号，如果小于服务器的版本号，不需要更新参数
bool CTraderCpMgr::CompareVersion(QString strLocalVersion, QString strSerVersion, int iCompare /*= 2 */)
{
	// 如果本地版本号其中有一个为空，则不升级（这个逻辑，具体的业务可以修改）
	if (strLocalVersion == "" || strSerVersion == "")
	{
		return false;
	}
	else
	{
		// 分别分割段到容器
		QVector<QString> vecLocal, vecServer;
		//CHJGlobalFun::SplitStrToVector(strLocalVersion, ".", vecLocal);
		//CHJGlobalFun::SplitStrToVector(strSerVersion,   ".", vecServer);

		// 获取本地、服务端、和iCompare中最小的段，主要是防止循环的时候溢出出错
		int iCliSize = vecLocal.size();
		int iSerSize = vecServer.size();
		int iComStep = iCliSize < iSerSize ? iCliSize : iSerSize;
		iComStep = iComStep < iCompare ? iComStep : iCompare;

		for (int i = 0; i < iComStep; i++)
		{
			/*if(atoi(vecServer.at(i)) > atoi(vecLocal.at(i)))
			{
			return true;
			}*/
		}

		// 如果相同步长内的值相等，则比较步长的长度
		if (iSerSize > iCliSize)
		{
			return true;
		}
	}

	return false;
}

// 清空所有保存码表的vector
void CTraderCpMgr::ClearCodeTableList()
{
	m_vExchCode.clear();
	m_vOffsetFlag.clear();
	m_vFeeCode.clear();
	m_vSubFeeCode.clear();
	m_vMarketId.clear();
	m_vEntrState.clear();
	m_vCancelFlag.clear();
	m_vTermType.clear();
	m_vAccessWay.clear();
	m_vCheckState.clear();
	m_vLongShort.clear();
	m_vTakeState.clear();
	m_vDrawType.clear();
	m_vCertType.clear();
	m_vAcctState.clear();
}

QString CTraderCpMgr::GetProdCodeName(const QString &sID)
{
	for (size_t i = 0; i < m_vProdCode.size(); i++)
	{
		if (m_vProdCode.at(i).prod_code == sID)
		{
			return m_vProdCode.at(i).prod_name;
		}
	}
	return sID;
}

QString CTraderCpMgr::TranslateIDToValue(const QString &sTitleID, const QString &sValue)
{
	QString sFinal;
	if (sTitleID == "exch_date" || sTitleID == "bk_plat_date" || sTitleID == "stor_date")//交易时间
		sFinal = CHJGlobalFun::FormatDate(sValue);
	else if (sTitleID == "market_id")//交易市场
		sFinal = GetMarketName(sValue);
	else if (sTitleID == "exch_code")//交易类型
		sFinal = GetExchName(sValue);
	else if (sTitleID == "offset_flag")//开平标志
		sFinal = GetOffset_flagName(sValue);
	else if (sTitleID == "deli_flag")//交收标志，待处理
		sFinal = GetDeli_flagName(sValue);
	else if (sTitleID == "bs")//买卖方向
		sFinal = GetBuySellName(sValue);
	else if (sTitleID == "entr_stat")//委托状态
		sFinal = GetEntr_statName(sValue);
	else if (sTitleID == "cancel_flag")//撤单标志
		sFinal = GetCancel_flagName(sValue);
	else if (sTitleID == "access_way")//操作方向
		sFinal = GetAccessWayName(sValue);
	else if (sTitleID == "check_stat1" || sTitleID == "check_stat2")//复核状态
		sFinal = GetCheck_staName(sValue);
	else if (sTitleID == "in_account_flag")//是否成功
		sFinal = GetIn_account_flagName(sValue);
	else if (sTitleID == "long_short")//持仓方向
		sFinal = GetLongShortName(sValue);
	else if (sTitleID == "e_term_type" || sTitleID == "c_term_type" || sTitleID == "term_type" || sTitleID == "o_term_type")//委托渠道,撤销渠道,操作来源
		sFinal = GetTerm_typeName(sValue);
	else if (sTitleID == "market_id" || sTitleID == "accept_time" || sTitleID == "e_exch_time" || sTitleID == "c_exch_time" || sTitleID == "exch_time")//撤销时间
		sFinal = CHJGlobalFun::FormatTime(sValue);
	else if (IsFloatTitle(sTitleID))//委托价格
		sFinal = CHJGlobalFun::FormatFloat(sValue);
	else
		sFinal = sValue;

	return sFinal;
}

bool CTraderCpMgr::IsFloatTitle(const QString &sTitleID)
{
	const int iCount = 51;
	// 去掉last_long_froz,last_can_use,curr_can_use,curr_can_get
	QString title[iCount] = { "entr_price", "match_price", "exch_bal", "exch_fare", "margin", "long_open_avg_price", "short_open_avg_price", "long_posi_avg_price", "short_posi_avg_price", "last_settle_price",
		"day_settle_price", "storage_cost", "last_bal", "curr_bal", "last_margin", "last_reserve", "out_bal",
		"in_bal","real_buy","real_sell","real_reserve","real_margin","base_margin","last_base_margin","deli_prepare","last_deli_prepare","deli_margin",
		"last_deli_margin","real_exch_fare","other_fare","pay_breach","take_breach","cov_surplus","mark_surplus","float_surplus","day_long_froz ",
		"last_forward_froz","day_forward_froz","inte_integral","puni_integral","wait_incr_inte","wait_incr_inte_tax","day_incr_inte","day_incr_inte_tax","last_take_margin","day_take_margin",
		"last_stor_fare_froz", "day_stor_fare_froz", "stor_price", "price", "fee_value" };//

	for (int i = 0; i < iCount; i++)
	{
		if (title[i] == sTitleID)
		{
			return true;
		}
	}
	return false;
}

// 获取客户费率模板到内存
void CTraderCpMgr::GetRsp2040()
{
	CTranMessage::Handle2040(m_rsp2040);
}

FareInfo CTraderCpMgr::GetBFareValue(const QString &sProdCode, const QString &sExchCode)
{
	FareInfo fi;
	for (size_t i = 0; i < m_rsp2040.alm_b_fare_list.size(); i++) //遍历返回结果
	{
		ArrayListMsg &aMsg = m_rsp2040.alm_b_fare_list.GetValue(i); //获取结果的第一个组数据

		if (aMsg.GetStringEx(0) == sProdCode.toStdString() && aMsg.GetStringEx(2) == sExchCode.toStdString())
		{
			fi.fare_mode_id = aMsg.GetStringEx(4).c_str();
			fi.fare_mode_name = aMsg.GetStringEx(5).c_str();
			fi.fare_value = aMsg.GetStringEx(6).c_str();
			return fi;
		}
	}

	return fi;
}
// 根据品种获得会员手续费信息
FareInfo CTraderCpMgr::GetMFareValue(const QString &sProdCode, const QString &sExchCode)
{
	FareInfo fi;
	for (size_t i = 0; i < m_rsp2040.alm_m_fare_list.size(); i++) //遍历返回结果
	{
		ArrayListMsg &aMsg = m_rsp2040.alm_m_fare_list.GetValue(i); //获取结果的第一个组数据

		if (aMsg.GetStringEx(0) == sProdCode.toStdString() && aMsg.GetStringEx(2) == sExchCode.toStdString())
		{
			fi.fare_mode_id = aMsg.GetStringEx(4).c_str();
			fi.fare_mode_name = aMsg.GetStringEx(5).c_str();
			fi.fare_value = aMsg.GetStringEx(6).c_str();
			return fi;
		}
	}

	return fi;
}

ProdCodeInfo CTraderCpMgr::GetProdeCode(const QString &sProdCode)
{
	ProdCodeInfo info;

	for (size_t i = 0; i < m_vProdCode.size(); i++)
	{
		if (m_vProdCode.at(i).prod_code == sProdCode)
		{
			info = m_vProdCode.at(i);
			break;
		}
	}

	return info;
}

bool CTraderCpMgr::GetProdeCode(ProdCodeInfo& stProdCode, const QString &sProdCode)
{
	for (size_t i = 0; i < m_vProdCode.size(); i++)
	{
		if (m_vProdCode.at(i).prod_code == sProdCode)
		{
			stProdCode = m_vProdCode.at(i);
			return true;
		}
	}

	return false;
}



// 从今仓中依照顺序平掉iHand手  20121107 因为要保存延期成交流水信息，所以不删除流水
void CTraderCpMgr::RemoveMatchFlow(LISTDeferMatch &listMatch, int iHand)
{
	for (LISTDeferMatch::iterator it = listMatch.begin(); it != listMatch.end(); it++)
	{
		if (it->iHand >= iHand) // 如果当前流水的手数不小于剩余手数，则记录平掉的手数，退出循环
		{
			it->iHand -= iHand;
			break;
		}
		else // 如果当前流水的手数小于剩余手数，则记录未被平掉的手数，并删除当前流水
		{
			//  解决之前没有将it->iHand置为0导致的计算错误
			if (it->iHand != 0)
			{
				iHand -= it->iHand;
				it->iHand = 0;
			}
		}
		/*
		if(it->iHand > iHand ) // 如果当前流水的手数大于剩余手数，则记录平掉的手数，退出循环
		{
		it->iHand -= iHand;
		break;
		}
		else if(it->iHand == iHand) // 如果刚好相等，则该流水被平掉，删除该流水，退出循环
		{
		listMatch.erase(it);
		break;
		}
		else // 如果当前流水的手数小于剩余手数，则记录未被平掉的手数，并删除当前流水
		{
		iHand -= it->iHand;
		it = listMatch.erase(it);
		}
		*/
	}
}

#if 0
// 重新计算某一个品种在某一个仓位上的统计信息（持仓均价、持仓盈亏，持仓保证金）
void CTraderCpMgr::CalculateOtherValue(const QString &sProdCode, DeferPosiInfo &stDeferPosiInfo, bool bLong)
{
	// 计算持仓均价以及开仓均价-----------------------------------------------------------------------------------------
	// 获取成交流水
	const LISTDeferMatch &listMatch = stDeferPosiInfo.listMatch;

	// 获取总持仓量
	int iTotalHand = stDeferPosiInfo.iYesAmt; // 用昨仓初始化
											  // 最后的总保证金
	double dTotalMoney = stDeferPosiInfo.fYesAvgPosPrice * stDeferPosiInfo.iYesAmt; // 用昨仓的持仓保证金初始化

																					// modify by  20130222 根据金联通的要求，开仓均价与服务器保持一致
																					//// 总的开仓手数，用昨日的原始仓位初始化
																					//int iOpenTotalHand = stDeferPosiInfo.iYesOrgAmt;
																					//// 总的开仓金额，用昨日的原始开仓金额初始化
																					//double dOpenTotalMoney = stDeferPosiInfo.dYesAvgOpenPosPrice * stDeferPosiInfo.iYesOrgAmt;

																					// 总的开仓手数，用昨日的原始仓位初始化
	int iOpenTotalHand = stDeferPosiInfo.iYesAmt;
	// 总的开仓金额，用昨日的原始开仓金额初始化
	double dOpenTotalMoney = stDeferPosiInfo.dYesAvgOpenPosPrice * stDeferPosiInfo.iYesAmt;

	// 加上今仓的保证金
	for (LISTDeferMatch::const_iterator it = listMatch.begin(); it != listMatch.end(); ++it)
	{
		dTotalMoney += it->dPrice * it->iHand;
		iTotalHand += it->iHand;

#ifdef _WRITE_LOG
		static QString csLog;
		csLog.Format("品种：%s 增加价格：%.2f 手数：%d 总金额：%.2f 总手数：%d",
			sProdCode, it->dPrice, it->iHand, dTotalMoney, iTotalHand);
		g_Global.WriteLog(csLog);
#endif

		// modify by  20130222 根据金联通的要求，开仓均价与服务器保持一致
		/*dOpenTotalMoney += it->dPrice * it->iTotalHand;
		iOpenTotalHand += it->iTotalHand;*/
		dOpenTotalMoney += it->dPrice * it->iHand;
		iOpenTotalHand += it->iHand;
	}
	stDeferPosiInfo.dAvgPosPrice = dTotalMoney / iTotalHand;
	stDeferPosiInfo.dAvgOpenPosPrice = dOpenTotalMoney / iOpenTotalHand;

	// 计算持仓盈亏-----------------------------------------------------------------------------------------
	stDeferPosiInfo.dPosi_PL = GetPosi_PL(sProdCode, stDeferPosiInfo.dAvgPosPrice, iTotalHand, bLong);

	// 计算持仓保证金--------------------------------------------------------------------------------------
	//stDeferPosiInfo.dTotalFare = CalculateMagin(sProdCode, stDeferPosiInfo.dAvgPosPrice, iTotalHand);
	if (stDeferPosiInfo.iYesOrgAmt > 0)
	{
		stDeferPosiInfo.dTotalFare = stDeferPosiInfo.dYesPosiMoney * stDeferPosiInfo.iYesAmt / stDeferPosiInfo.iYesOrgAmt;
	}
	else
	{
		stDeferPosiInfo.dTotalFare = 0.00;
	}

	for (LISTDeferMatch::const_iterator it = listMatch.begin(); it != listMatch.end(); ++it)
	{
		stDeferPosiInfo.dTotalFare += it->dPosiMoney*it->iHand / it->iTotalHand;
	}

	// 计算持仓盈亏
}
#endif

// 重新计算某一个品种在某一个仓位上的统计信息（持仓均价、持仓盈亏，持仓保证金）
void CTraderCpMgr::CalculateOtherValue(const QString &sProdCode, DeferPosiInfo &stDeferPosiInfo, bool bLong, bool bRefresh /*= false*/)
{
	// 计算持仓均价以及开仓均价-----------------------------------------------------------------------------------------
	// 获取成交流水
	const LISTDeferMatch &listMatch = stDeferPosiInfo.listMatch;

	// 获取总持仓量
	int iTotalHand = stDeferPosiInfo.iYesAmt; // 用昨仓初始化
											  // 最后的总保证金
	double dTotalMoney = stDeferPosiInfo.fYesAvgPosPrice * stDeferPosiInfo.iYesAmt; // 用昨仓的持仓保证金初始化

																					// modify by  20130222 根据金联通的要求，开仓均价与服务器保持一致
																					//// 总的开仓手数，用昨日的原始仓位初始化
																					//int iOpenTotalHand = stDeferPosiInfo.iYesOrgAmt;
																					//// 总的开仓金额，用昨日的原始开仓金额初始化
																					//double dOpenTotalMoney = stDeferPosiInfo.dYesAvgOpenPosPrice * stDeferPosiInfo.iYesOrgAmt;

																					// 总的开仓手数，用昨日的原始仓位初始化
	int iOpenTotalHand = stDeferPosiInfo.iYesAmt;
	// 总的开仓金额，用昨日的原始开仓金额初始化
	double dOpenTotalMoney = stDeferPosiInfo.dYesAvgOpenPosPrice * stDeferPosiInfo.iYesAmt;

	// 加上今仓的保证金
	for (LISTDeferMatch::const_iterator it = listMatch.begin(); it != listMatch.end(); ++it)
	{
		double d = it->dPrice;
		int h = it->iHand;
		dTotalMoney += it->dPrice * it->iHand;
		iTotalHand += it->iHand;

#ifdef _WRITE_LOG
		static QString csLog;
		csLog.Format("品种：%s 增加价格：%.2f 手数：%d 总金额：%.2f 总手数：%d",
			sProdCode, it->dPrice, it->iHand, dTotalMoney, iTotalHand);
		g_Global.WriteLog(csLog);
#endif

		// modify by  20130222 根据金联通的要求，开仓均价与服务器保持一致
		/*dOpenTotalMoney += it->dPrice * it->iTotalHand;
		iOpenTotalHand += it->iTotalHand;*/
		dOpenTotalMoney += it->dPrice * it->iHand;
		iOpenTotalHand += it->iHand;
	}
	stDeferPosiInfo.dAvgPosPrice = dTotalMoney / iTotalHand;
	stDeferPosiInfo.dAvgOpenPosPrice = dOpenTotalMoney / iOpenTotalHand;

	if (bRefresh)
	{
		// mod by Jerry Lee, 2013-3-23, 通过查询获得持仓均价和开仓均价，避免与服务端不一致的情况发生
		Rsp1020 rsp1020;
		//if(CTranMessage::Handle1020(rsp1020, '0', '0', '0', '0', '1', '0', true) == 0)
		//{
		//	for (int j = 0; j < rsp1020.htm_td_info.size(); j++)
		//	{
		//		QString s = rsp1020.htm_td_info.GetQString(j, "td_prod_code");
		//		if(s == sProdCode)
		//		{
		//			double dAvgPosPrice = atof(rsp1020.htm_td_info.GetQString(j, 
		//				bLong?"td_long_posi_avg_price":"td_short_posi_avg_price"));
		//			double dAvgOpenPosPrice = atof(rsp1020.htm_td_info.GetQString(j, 
		//				bLong?"td_long_open_avg_price":"td_short_open_avg_price"));
		//			//stDeferPosiInfo.dAvgPosPrice = dAvgPosPrice;
		//			stDeferPosiInfo.dAvgOpenPosPrice = dAvgOpenPosPrice;
		//			stDeferPosiInfo.dSvrAvgPosPrice = dAvgPosPrice;

		//			break;
		//		}
		//	}
		//}
	}
	else
	{
		for (int j = 0; j < m_rsp1020.htm_td_info.size(); j++)
		{
			string s = m_rsp1020.htm_td_info.GetString(j, "td_prod_code");
			if (s == sProdCode.toStdString())
			{
				double dAvgPosPrice =CHJGlobalFun::str2qstr( m_rsp1020.htm_td_info.GetString(j,
					bLong ? "td_long_posi_avg_price" : "td_short_posi_avg_price")).toFloat();
				double dAvgOpenPosPrice = CHJGlobalFun::str2qstr(m_rsp1020.htm_td_info.GetString(j,
					bLong ? "td_long_open_avg_price" : "td_short_open_avg_price")).toFloat();
				//stDeferPosiInfo.dAvgPosPrice = dAvgPosPrice;
				stDeferPosiInfo.dAvgOpenPosPrice = dAvgOpenPosPrice;
				stDeferPosiInfo.dSvrAvgPosPrice = dAvgPosPrice;

				break;
			}
		}
	}

	//

	// 计算持仓盈亏-----------------------------------------------------------------------------------------
	stDeferPosiInfo.dPosi_PL = GetPosi_PL(sProdCode, stDeferPosiInfo.dAvgPosPrice, iTotalHand, bLong);

	// 计算持仓保证金--------------------------------------------------------------------------------------
	//stDeferPosiInfo.dTotalFare = CalculateMagin(sProdCode, stDeferPosiInfo.dAvgPosPrice, iTotalHand);
	if (stDeferPosiInfo.iYesOrgAmt > 0)
	{
		stDeferPosiInfo.dTotalFare = stDeferPosiInfo.dYesPosiMoney * stDeferPosiInfo.iYesAmt / stDeferPosiInfo.iYesOrgAmt;
	}
	else
	{
		stDeferPosiInfo.dTotalFare = 0.00;
	}

	for (LISTDeferMatch::const_iterator it = listMatch.begin(); it != listMatch.end(); ++it)
	{
		stDeferPosiInfo.dTotalFare += it->dPosiMoney*it->iHand / it->iTotalHand;
	}

	// 计算持仓盈亏
}

// 计算一个品种的持仓盈亏
double CTraderCpMgr::GetPosi_PL(const QString &sProdCode, double dAvgPosiPrice, int iTotalHand, bool bLong)
{
	// 获取该品种的计量单位（则每手的克数）
	ProdCodeInfo& info = GetProdeCode(sProdCode);
	double dUnite = 0.00;
	if (!info.prod_code.isEmpty())
	{
		dUnite = info.measure_unit;
	}

	// 根据品种获得行情
	const QUOTATION &qt = m_QMapQuotation[sProdCode.toStdString()];

	// 获取最新价
	unsigned int uiOrg = qt.m_uiLast > 0 ? qt.m_uiLast : qt.m_uiLastSettle; // 如果最新价没有则采用昨结
	double dNewest = (double)uiOrg / g_Global.m_dQTFactor; // 获取最新价

														   // 获取盈亏价，注意如果是空仓的话，盈亏相反
	double dPriceDif = bLong ? (dNewest - dAvgPosiPrice) : (dAvgPosiPrice - dNewest);

	// 用计算持仓盈亏 = （最新价-持仓均价）*总持仓*计量单位
	double dMoney = dPriceDif * iTotalHand * CHJCommon::HandToGram(sProdCode, dUnite);

	// 解决空仓的时候持仓盈亏为0的时候显示-0.00的问题
	if (dMoney < 0.001 && dMoney > -0.001)
	{
		dMoney = 0.00;
	}

	/*
	// 获取该品种的计量单位（则每手的克数）
	double dUnite = GetProdeCode(sProdCode).measure_unit;

	// 根据品种获得行情
	const QUOTATION &qt = m_QMapQuotation[sProdCode];

	// 获取最新价
	unsigned int uiOrg = qt.m_uiLast > 0 ? qt.m_uiLast : qt.m_uiLastSettle; // 如果最新价没有则采用昨结
	unsigned int u = uiOrg - dAvgPosiPrice*g_Global.m_dQTFactor;

	// 获取盈亏价，注意如果是空仓的话，盈亏相反
	int nProfit = bLong ? u : -1*u;

	// 用计算持仓盈亏 = （最新价-持仓均价）*总持仓*计量单位
	double dMoney = (nProfit * iTotalHand * CHJCommon::HandToGram(sProdCode, dUnite))/g_Global.m_dQTFactor;

	// 解决空仓的时候持仓盈亏为0的时候显示-0.00的问题
	if(dMoney < 0.001 && dMoney > -0.001)
	{
	dMoney = 0.00;
	}
	*/


	return dMoney;
}
/*
处理延期平仓撤单的释放冻结仓位
instID：撤单的品种
bLong：true为多仓，false为空仓
bExist：该笔撤单对应的报单流水是否在内存中存在，一般情况下都为true
false的情况如下：一笔部分成交的报单被撤销后，如果软件重新登录，服务器会推送
撤单流水以及成交流水，但是不会推送报单流水过来，由于成交流水
不会处理可用仓位，这样的话已成交的仓位没有减掉，所以要在这里处理
iTotalAmount：总的报单手数
iRemainAmount：剩余手数
*/
bool CTraderCpMgr::DealDeferCancelFreezePosi(const QString &instID, bool bLong, bool bExist, int iTotalAmount, int iRemainAmount)
{
	// 根据合约代码获取该品种的在该仓位上的持仓信息
	QMap<QString, DeferPosi>::iterator itPosi = m_QMapDeferPosi.find(instID);
	DeferPosiInfo &info = bLong ? itPosi.value().infoLong : itPosi.value().infoShort;

	// 根据该撤单的本地报单号查找是否在报单流水中存在
	if (bExist)
	{
		// 取消报单的时候冻结的可用仓位
		info.iUsefulAmt += iRemainAmount;

		return true;
	}
	else
	{
		// 由于处理一笔报单部分成交的撤销单没有报单信息的情况（软件刚开始登录的时候出现）
		// 如果报单量大于未成交量，证明有成交
		// 模拟一次报单流水（即将可用仓减少1）
		int iMatchHand = iTotalAmount - iRemainAmount;
		if (iMatchHand > 0)
		{
			info.iUsefulAmt -= iMatchHand;

			return true;
		}
	}

	return false;
}

int CTraderCpMgr::GetInstStatePos(const QString &sProdCode)
{
	for (size_t i = 0; i < m_vInstState.size(); i++)
	{
		if (m_vInstState.at(i).instID == sProdCode.toStdString())
			return i;
	}

	return -1;
}

QString CTraderCpMgr::GetInsStateID(const QString &sInsID)
{
	for (size_t i = 0; i < m_vInstState.size(); i++)
	{
		if (m_vInstState.at(i).instID == sInsID.toStdString())
		{
			return m_vInstState.at(i).tradeState.c_str();
		}
	}

	return "";
}
// 获取无符号整形的广义最新价
unsigned int CTraderCpMgr::GetLastPrice(const QUOTATION &qt)
{
	unsigned int uiOrg = qt.m_uiLast > 0 ? qt.m_uiLast : qt.m_uiLastSettle;

	if (uiOrg > 0)
		return uiOrg;
	else
		return qt.m_uilastClose;
}
// 获取double的广义最新价
double CTraderCpMgr::GetLastPriceEx(const QUOTATION &qt)
{
	return GetLastPrice(qt) / g_Global.m_dQTFactor;
}
// 初始化客户数据，csTitle 配置文件中的title，pFunAsk对应的请求数据的函数地址，pFunLoad对应的加载数据的函数地址
bool CTraderCpMgr::IniBasicData(const QString &csTitle, PFunTypeAsk pFunAsk, PFunTypLoad pFunLoad)
{
	bool bLoad = false;

	QString strDataPath = g_Global.GetDataIniPath();
	char buf[500] = { 0 };
	QString sValue;

	// 读取文件中保存的客户号
	//::GetPrivateProfileQString("info", csTitle, "", buf, sizeof(buf), strDataPath);
	sValue = buf;
	int iPos = sValue.indexOf("∧"); // 获取客户号的分隔符
	if (iPos != -1)
	{
		// 如果客户号相同
		if (sValue.mid(0, iPos) == g_Global.m_strUserID)
		{
			// 获取上一交易日+值的字符串
			sValue = sValue.mid(iPos + 2);

			iPos = sValue.indexOf("∧"); // 获取客户号的分隔符
			if (iPos != -1)
			{
				// 获取数据对应的上一交易日
				QString sLastExchDate = sValue.mid(0, iPos);
				// 与当前实际的上一交易日对比
				if (sLastExchDate == g_Global.m_strLast_Exch_Date)
				{
					// 如果相同，则直接用本地的数据加载
					ArrayListMsg alm_result;
					alm_result.Parse(sValue.mid(iPos + 2).toStdString());

					(this->*pFunLoad)(alm_result);
				}
				else
				{
					// 如果交易日不相符，则标记为向服务器请求数据
					bLoad = true;
				}
			}
			else
			{
				bLoad = true;
			}
		}
		else
		{
			bLoad = true;
		}
	}
	else
	{
		// 如果没找到分割符则标记为向服务器请求数据
		bLoad = true;
	}

	// 如果本地保存的数据不是最新的或者没有，则向服务器请求数据
	if (bLoad)
	{
		ArrayListMsg alm_result;
		if ((this->*pFunAsk)(alm_result))
		{
			QString sWriteData = g_Global.m_strUserID + "∧" + g_Global.m_strLast_Exch_Date + "∧" + CHJGlobalFun::str2qstr(alm_result.ToString());
			//::WritePrivateProfileQString("Info", csTitle, sWriteData,strDataPath); 
			(this->*pFunLoad)(alm_result);
		}
		else
		{
			return false;
		}
	}

	return true;
}

// 请求服务器基础数据统一调用的函数
bool CTraderCpMgr::AskForData(const QString &sReqTitle, QVector< pair<QString, QString> > &vecPara, const ArrayListMsg &almViewField, ArrayListMsg &alm_result)
{
	Rsp6002 rsp6002;

	// 添加查询条件
	//CHJGlobalFun::PairAdd(vecPara, "start_date", g_Global.m_strLast_Exch_Date); 
	//CHJGlobalFun::PairAdd(vecPara, "end_date", g_Global.m_strLast_Exch_Date);

	// 如果服务器正常返回数据
	//if( CTranMessage::Handle6002(rsp6002, sReqTitle, almViewField, vecPara, 500 ) == 0 )
	//{
	//	alm_result = rsp6002.alm_result;
	//	return true;
	//}
	//else
	{
		return false;
	}
}
// 向服务器请求客户昨日资金数据
bool CTraderCpMgr::AskForFundBaseData(ArrayListMsg &alm_result)
{
	QVector< pair<QString, QString> > vecPara;

	// 获取请求的列ID 当日余额（curr_bal） 当日可用（curr_can_use） 当日提货保证金冻结（day_take_margin） 当日仓储费冻结（day_stor_fare_froz）
	ArrayListMsg almViewField;
	almViewField.Parse("curr_bal∧curr_can_use∧day_take_margin∧day_stor_fare_froz∧");

	bool bReturn = AskForData("AcctFundHis", vecPara, almViewField, alm_result);
	if (bReturn && alm_result.size() == 0)
	{
		alm_result.Parse("0｜0｜0｜0｜∧");
	}

	return bReturn;
}
// 加载昨日资金数据到内存
void CTraderCpMgr::LoadFundBaseData(const ArrayListMsg &alm_result)
{
	// 保存数据到内存，注意到下面获取某一个字段的值都是采用序号
	const ArrayListMsg &aMsg = alm_result.GetValue(0); // 获取结果的一组数据

	m_stFundInfo.dAllBalance += aMsg.GetValue<double>(0);

	//m_stFundInfo.dUseFulBal += aMsg.GetValue<double>(1);
	m_stFundInfo.dUseFulBal = m_stFundInfo.dUseFulBal + aMsg.GetValue<double>(1) - aMsg.GetValue<double>(2) - aMsg.GetValue<double>(3);
}

// 向服务器请求客户昨日数据
bool CTraderCpMgr::AskForDeferPosiBaseData(ArrayListMsg &alm_result)
{
	QVector< pair<QString, QString> > vecPara;
	// 添加查询条件
	//CHJGlobalFun::PairAdd(vecPara, "prod_code", "");

	ArrayListMsg almViewField; // 获取请求的列ID
							   //almViewField.Parse("exch_date∧prod_code∧last_long_amt∧last_short_amt∧curr_long_amt∧curr_short_amt∧curr_long_can_use∧curr_short_can_use∧open_long_amt∧open_short_amt∧cov_long_amt∧cov_short_amt∧deli_long_amt∧deli_short_amt∧midd_open_long_amt∧midd_open_short_amt∧long_open_avg_price∧short_open_avg_price∧long_posi_avg_price∧short_posi_avg_price∧last_settle_price∧day_settle_price∧");
	almViewField.Parse("prod_code∧curr_long_amt∧curr_short_amt∧curr_long_can_use∧curr_short_can_use∧day_settle_price∧long_open_avg_price∧short_open_avg_price∧");

	return AskForData("AcctDeferPosiHis", vecPara, almViewField, alm_result);
}
// 加载昨日数据到内存
void CTraderCpMgr::LoadDeferPosiBaseData(const ArrayListMsg &alm_result)
{
	size_t iTotalCount = alm_result.size();

	// 保存数据到内存，注意到下面获取某一个字段的值都是采用序号，如果序号变了的话则有误，是否考虑根据字段ID来
	for (size_t i = 0; i < iTotalCount; i++)
	{
		const ArrayListMsg &aMsg = alm_result.GetValue(i); // 获取结果的一组数据

														   // 去掉最后一行的统计
		if (i == iTotalCount - 1 && aMsg.GetStringEx(0).empty())
			break;

		// 获取合约品种
		QString sProdCode = aMsg.GetStringEx(0).c_str();

		// 获取该合约代码对应的持仓信息
		QMap<QString, DeferPosi>::iterator it = m_QMapDeferPosi.find(sProdCode);
		if (it == m_QMapDeferPosi.end())
		{
			DeferPosi stDeferPosi;
			stDeferPosi.sProdCode = sProdCode;
			it = m_QMapDeferPosi.insert(sProdCode, stDeferPosi);
		}
		DeferPosi &stDeferPosi = it.value();

		//添加 昨日多仓信息（总持仓、可用仓、昨仓）
		stDeferPosi.infoLong.iCurrAllAmt += CHJGlobalFun::str2qstr( aMsg.GetStringEx(1)).toInt(); // 获取昨天的总持仓作为今天的总持仓
		stDeferPosi.infoLong.iYesOrgAmt = stDeferPosi.infoLong.iYesAmt = CHJGlobalFun::str2qstr(aMsg.GetStringEx(1)).toUInt(); // 由于仓位数据记录的是今天的，所以内存中的昨仓应该是报文中的总持仓
		stDeferPosi.infoLong.iUsefulAmt += CHJGlobalFun::str2qstr(aMsg.GetStringEx(3)).toUInt(); //  获取昨天的可用仓作为今天的可用仓

																		  // 添加昨日空仓信息
		stDeferPosi.infoShort.iCurrAllAmt += CHJGlobalFun::str2qstr(aMsg.GetStringEx(2)).toUInt();
		stDeferPosi.infoShort.iYesOrgAmt = stDeferPosi.infoShort.iYesAmt = CHJGlobalFun::str2qstr(aMsg.GetStringEx(2)).toUInt(); //
		stDeferPosi.infoShort.iUsefulAmt += CHJGlobalFun::str2qstr(aMsg.GetStringEx(4)).toUInt();

		// 记录昨日持仓均价（昨结）
		stDeferPosi.infoLong.fYesAvgPosPrice = CHJGlobalFun::str2qstr(aMsg.GetStringEx(5)).toUInt();
		stDeferPosi.infoShort.fYesAvgPosPrice = CHJGlobalFun::str2qstr(aMsg.GetStringEx(5)).toUInt();

		// 记录上日的开仓均价
		stDeferPosi.infoLong.dYesAvgOpenPosPrice = CHJGlobalFun::str2qstr(aMsg.GetStringEx(6)).toUInt();
		stDeferPosi.infoShort.dYesAvgOpenPosPrice = CHJGlobalFun::str2qstr(aMsg.GetStringEx(7)).toUInt();

		// 记录昨日保证金信息
		if (stDeferPosi.infoLong.iYesOrgAmt > 0)
		{
			stDeferPosi.infoLong.dYesPosiMoney = CalculateMagin(sProdCode, stDeferPosi.infoLong.fYesAvgPosPrice, stDeferPosi.infoLong.iYesOrgAmt, false);
		}
		if (stDeferPosi.infoShort.iYesOrgAmt > 0)
		{
			stDeferPosi.infoShort.dYesPosiMoney = CalculateMagin(sProdCode, stDeferPosi.infoShort.fYesAvgPosPrice, stDeferPosi.infoShort.iYesOrgAmt, false);
		}

		// 计算统计信息
		CalculateOtherValue(stDeferPosi.sProdCode, stDeferPosi.infoLong, true);
		CalculateOtherValue(stDeferPosi.sProdCode, stDeferPosi.infoShort, false);

		m_QMapDeferPosi[stDeferPosi.sProdCode] = stDeferPosi;
	}
}

// 向服务器请求客户昨日远期数据
bool CTraderCpMgr::AskForForwardPosiBaseData(ArrayListMsg &alm_result)
{
	QVector< pair<QString, QString> > vecPara;
	// 添加查询条件
	/*CHJGlobalFun::PairAdd(vecPara, "prod_code", "");*/

	ArrayListMsg almViewField; // 获取请求的列ID
							   // 合约代码∧当前多仓∧当前空仓∧可用多仓∧可用空仓∧上日结算价
	almViewField.Parse("prod_code∧curr_long_amt∧curr_short_amt∧curr_long_can_use∧curr_short_can_use∧settle_price∧");

	return AskForData("AcctForwardPosiHis", vecPara, almViewField, alm_result);
}
// 加载昨日远期数据到内存
void CTraderCpMgr::LoadForwardPosiBaseData(const ArrayListMsg &alm_result)
{
	size_t iTotalCount = alm_result.size();

	// 保存数据到内存，注意到下面获取某一个字段的值都是采用序号，如果序号变了的话则有误，是否考虑根据字段ID来
	for (size_t i = 0; i < iTotalCount; i++)
	{
		const ArrayListMsg &aMsg = alm_result.GetValue(i); // 获取结果的一组数据

														   // 去掉最后一行的统计
		if (i == iTotalCount - 1 && aMsg.GetStringEx(0).empty())
			break;

		// 获取合约品种
		QString sProdCode = aMsg.GetStringEx(0).c_str();

		// 获取该合约代码在内存中对应的持仓信息
		QMap<QString, DeferPosi>::iterator it = m_QMapDeferPosi.find(sProdCode);
		if (it == m_QMapDeferPosi.end())
		{
			// 如果不存在则插入
			DeferPosi stDeferPosi;
			stDeferPosi.sProdCode = sProdCode;
			it = m_QMapDeferPosi.insert(sProdCode, stDeferPosi);
			//it = Insert_Pair.key;
		}
		DeferPosi &stDeferPosi = it.value();

		// 记录昨日多仓信息（总持仓、昨仓、可用仓）
		stDeferPosi.infoLong.iCurrAllAmt += CHJGlobalFun::str2qstr(aMsg.GetStringEx(1)).toUInt(); // 获取昨天的总持仓作为今天的总持仓
		stDeferPosi.infoLong.iYesOrgAmt = stDeferPosi.infoLong.iYesAmt = CHJGlobalFun::str2qstr(aMsg.GetStringEx(1)).toUInt(); // 由于仓位数据记录的是今天的，所以内存中的昨仓应该是报文中的总持仓
		stDeferPosi.infoLong.iUsefulAmt += CHJGlobalFun::str2qstr(aMsg.GetStringEx(3)).toUInt(); //  获取昨天的可用仓作为今天的可用仓

																		  // 记录昨日空仓信息
		stDeferPosi.infoShort.iCurrAllAmt += CHJGlobalFun::str2qstr(aMsg.GetStringEx(2)).toUInt();
		stDeferPosi.infoShort.iYesOrgAmt = stDeferPosi.infoShort.iYesAmt = CHJGlobalFun::str2qstr(aMsg.GetStringEx(2)).toUInt(); //
		stDeferPosi.infoShort.iUsefulAmt += CHJGlobalFun::str2qstr(aMsg.GetStringEx(4)).toUInt();

		// 记录昨日持仓均价（昨结）
		stDeferPosi.infoLong.fYesAvgPosPrice = CHJGlobalFun::str2qstr(aMsg.GetStringEx(5)).toUInt();
		stDeferPosi.infoShort.fYesAvgPosPrice = CHJGlobalFun::str2qstr(aMsg.GetStringEx(5)).toUInt();

		// 记录昨日保证金信息
		if (stDeferPosi.infoLong.iYesOrgAmt > 0)
		{
			stDeferPosi.infoLong.dYesPosiMoney = CalculateMagin(sProdCode, stDeferPosi.infoLong.fYesAvgPosPrice, stDeferPosi.infoLong.iYesOrgAmt, false);
		}
		if (stDeferPosi.infoShort.iYesOrgAmt > 0)
		{
			stDeferPosi.infoShort.dYesPosiMoney = CalculateMagin(sProdCode, stDeferPosi.infoShort.fYesAvgPosPrice, stDeferPosi.infoShort.iYesOrgAmt, false);
		}

		// 计算统计信息
		//CalculateOtherValue(stDeferPosi.sProdCode, stDeferPosi.infoLong, true);
		//CalculateOtherValue(stDeferPosi.sProdCode, stDeferPosi.infoShort, false);

		m_QMapDeferPosi[stDeferPosi.sProdCode] = stDeferPosi;
	}
}

// 向服务器请求客户昨日库存数据
bool CTraderCpMgr::AskForStoreBaseData(ArrayListMsg &alm_result)
{
	QVector< pair<QString, QString> > vecPara;
	// 添加查询条件
	//CHJGlobalFun::PairAdd(vecPara, "variety_id", "");

	// 获取请求的列ID：合约代码，当前库存（curr_amt），当前可用（curr_can_use），当前可提（curr_can_get）
	ArrayListMsg almViewField;
	almViewField.Parse("variety_id∧curr_amt∧curr_can_use∧");

	return AskForData("AcctStorageHis", vecPara, almViewField, alm_result);
}
// 加载昨日库存数据到内存
void CTraderCpMgr::LoadStoreBaseData(const ArrayListMsg &alm_result)
{
	size_t iTotalCount = alm_result.size();

	// 保存数据到内存，注意到下面获取某一个字段的值都是采用序号，如果序号变了的话则有误，是否考虑根据字段ID来
	for (size_t i = 0; i < iTotalCount; i++)
	{
		const ArrayListMsg &aMsg = alm_result.GetValue(i); // 获取结果的一组数据

														   // 去掉最后一行的统计
		if (i == iTotalCount - 1 && CHJGlobalFun::str2qstr(aMsg.GetStringEx(0)).isEmpty())
			break;

		// 获取合约品种
		const QString &sProdCode = CHJGlobalFun::str2qstr(aMsg.GetStringEx(0));

		// 获取该合约代码对应的库存信息
		QMap<QString, StoreInfo>::iterator it = m_QMapStoreInfo.find(sProdCode);
		if (it == m_QMapStoreInfo.end())
		{
			StoreInfo stStore;
			it = m_QMapStoreInfo.insert(sProdCode, stStore);

		}
		StoreInfo &stStoreInfo = it.value();

		// 记录库存信息
		stStoreInfo.iAllAmt += aMsg.GetValue<int>(1);   // 获取昨天的总库存作为今天的总库存
		stStoreInfo.iUseFulAmt += aMsg.GetValue<int>(2); // 获取昨天的可用库存作为今天的可用库存

#ifdef _WRITE_SPOT_LOG
		QString csLog;
		csLog.Format("初始化%s的库存，总库存为%d，可用库存为：%d",
			sProdCode,
			stStoreInfo.iAllAmt,
			stStoreInfo.iUseFulAmt);
		g_Global.WriteLog(csLog);
#endif
	}
}



// 该部分代码
void CTraderCpMgr::CalSpotSellMatchFee(const ProdCodeInfo &info, double dPrice, int iHand, double &dMatchBal, double &dExchFare)
{
	// 合约代码
	const QString &sProd_Code = info.prod_code;

	// 获取每手的计量单位 （克）
	double dUnite = CHJCommon::HandToGram(sProd_Code, info.measure_unit);

	// 本金（发生货款）   计量单位*委托价格*手数，（注：参照服务器代码，对结果做了处理）
	double dOrgCost = CHJGlobalFun::DoubleFormat(dUnite * iHand * dPrice);

	// 获取手续费信息（会员和交易所）
	ExchFare ef_exch_fare = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, CONSTANT_SPOT_SELL_FARE_ID), GetMFareValue(sProd_Code, CONSTANT_SPOT_SELL_FARE_ID));
	dExchFare = ef_exch_fare.sum();

	dMatchBal = dOrgCost;
}

/*
info 品种信息结构
sExchID：交易代码ID
dPrice：价格
iHand：手数
bMatch：是否是处理成交（成交时计算的资金与报单不一定一致，主要体现在计算精度的问题）
*/
//最大手数 = 可用资金/ (每手冻结保证金  + 每手手续费)
//kenny:20180507  根据交易所规则，修改冻结保证金的计算公式
/*
1020 的响应报文增加两个字段


// 大边保证金信息	字段：group_id,long_margin,short_margin

public HashtableMsg htm_acct_large_side_sum = new HashtableMsg();

//大边组合信息	  字段：prod_code,group_id
public HashtableMsg htm_prod_group_info = new HashtableMsg();


1：报单的时候先判断合约 是否在 htm_prod_group_info 中，如果不在，则按照原逻辑计算。
如果在，则：先根据合约取htm_prod_group_info对应的group_id，根据group_id取 htm_acct_large_side_sum 的对应组的多仓和空仓保证金，
2:先根据多仓和空仓哪边大则那边为大边。如果两边相等，则按照原逻辑计算。
3 :
	·如果报单方向为大边，则待冻结金额为报单金额。
	·如果报单方向为小边，且小边金额 + 报单金额 <= 大边金额，则待冻结金额为0。
	否则，待冻结金额为 小边金额 + 报单金额 – 大边金额。
*/
double CTraderCpMgr::CalculateTradeFee(const ProdCodeInfo &info, const QString &sExchID, double dPrice, int iHand, bool bMatch /*= false*/, const QString &localOrderNo /*= ""*/) //, bool bTry /*=false*/
{
	// 合约代码
	const QString &sProd_Code = info.prod_code;

	// 获取每手的计量单位 （克）
	double dUnite = CHJCommon::HandToGram(sProd_Code, info.measure_unit);

	// 交易所需要的总金额
	double fTotalMoney = 0.00;

	// 本金（发生货款）   计量单位*委托价格*手数，（注：参照服务器代码，对结果做了处理）
	double dOrgCost = CHJGlobalFun::DoubleFormat(dUnite * iHand * dPrice);

	

	FareInfo stFareInfo;
	if (info.market_id == CONSTANT_B_MARKET_ID_SPOT) // 现货市场
	{
		// 获取手续费信息（会员和交易所）
		ExchFare ef_exch_fare;

		// 现货买入
		if (sExchID == CONSTANT_EXCH_CODE_SPOT_BUY)
		{
			ef_exch_fare = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, CONSTANT_SPOT_BUY_FARE_ID), GetMFareValue(sProd_Code, CONSTANT_SPOT_BUY_FARE_ID));

			// 溢短备付金
			ExchFare ef_diff_margin;

			if (sProd_Code == CONSTANT_PT9995)  // 铂金 Pt99.95 还需要算溢短备付金
			{
				// 计算溢短备付金
				ef_diff_margin = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, CONSTANT_OVER_SPARE_BAIL_ID), GetMFareValue(sProd_Code, CONSTANT_OVER_SPARE_BAIL_ID));
			}

			if (bMatch)
				fTotalMoney = dOrgCost + ef_exch_fare.sum() + ef_diff_margin.sum();
			else
				fTotalMoney = dOrgCost + ef_exch_fare.sumSingle() + ef_diff_margin.sumSingle();

			// 如果是报单信息则记录报单的冻结信息
			if (!localOrderNo.isEmpty())
			{
				OrderFrozeInfo stFrozeInfo;
				stFrozeInfo.Init(dOrgCost, ef_diff_margin.sumSingle(), ef_exch_fare.sumSingle(), iHand);

				m_QMapOrderFroze[localOrderNo] = stFrozeInfo;
			}
		}
		else // 现货卖出
		{
			// 获取手续费信息（会员和交易所）
			ef_exch_fare = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, CONSTANT_SPOT_SELL_FARE_ID), GetMFareValue(sProd_Code, CONSTANT_SPOT_SELL_FARE_ID));

			if (bMatch)
				fTotalMoney = dOrgCost - ef_exch_fare.sum();
			else
				fTotalMoney = dOrgCost - ef_exch_fare.sumSingle();
		}
	}
	else if (info.market_id == CONSTANT_B_MARKET_ID_DEFER) // 延期
	{
		//kenny  20180507   每手冻结保证金  有前置条件的变化逻辑
		if (sExchID == CONSTANT_EXCH_CODE_DEFER_OPEN_LONG || sExchID == CONSTANT_EXCH_CODE_DEFER_OPEN_SHORT) // 延期开仓
		{
			// 获取手续费信息（会员和交易所）
			ExchFare ef_exch_fare = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, 
																		GetBFareValue(sProd_Code, CONSTANT_OPEN_FARE_ID), GetMFareValue(sProd_Code, CONSTANT_OPEN_FARE_ID));

			if (bMatch)
			{
				// 计算成交的话只需扣除手续费
				fTotalMoney = ef_exch_fare.sumSingle();
			}
			else
			{
				// 保证金计算（会员和交易所）
				QString sFareCode;

				sFareCode = CONSTANT_EXCH_BAIL_FARE_ID;

				ExchFare ef_margin = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand,	
														GetBFareValue(sProd_Code, sFareCode), GetMFareValue(sProd_Code, sFareCode));
				//1.判断大边是否同方向
				//kenny  冻结资金  
				double frozen = g_CusInfo.CalculateFrozenCapital(sProd_Code, sExchID, dOrgCost);
				
				// 开仓手续费+开仓保证金
				if(frozen <= -1)
					fTotalMoney = ef_exch_fare.sumSingle() + ef_margin.sumSingle();
				else
					fTotalMoney = ef_exch_fare.sumSingle() + frozen;

				// 如果是报单信息则记录报单的冻结信息
				if (!localOrderNo.isEmpty())
				{
					OrderFrozeInfo stFrozeInfo;
					stFrozeInfo.Init(ef_margin.sumSingle(), 0.00, ef_exch_fare.sumSingle(), iHand);

					m_QMapOrderFroze[localOrderNo] = stFrozeInfo;
				}
			}
		}
		else if (sExchID == CONSTANT_EXCH_CODE_DEFER_COV_LONG || sExchID == CONSTANT_EXCH_CODE_DEFER_COV_SHORT) // 延期平仓
		{
			// 获取手续费信息（会员和交易所）
			ExchFare ef_exch_fare = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, CONSTANT_COV_FARE_ID), GetMFareValue(sProd_Code, CONSTANT_COV_FARE_ID));

			if (bMatch)
			{
				// 计算成交的话只需扣除手续费
				fTotalMoney = ef_exch_fare.sumSingle();
			}
			else
			{
				// 保证金计算（会员和交易所）
				ExchFare ef_margin = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, CONSTANT_EXCH_BAIL_FARE_ID), GetMFareValue(sProd_Code, CONSTANT_EXCH_BAIL_FARE_ID));

				// 开仓手续费-开仓保证金
				fTotalMoney = ef_margin.sumSingle() - ef_exch_fare.sumSingle();
			}
		}
		else if (sExchID == CONSTANT_EXCH_CODE_DELIVERY_LONG) // 收货
		{
			// 交割保证金（会员和交易所）
			ExchFare ef_DDA_margin = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, CONSTANT_FTD_T_DELI_MARGIN), GetMFareValue(sProd_Code, CONSTANT_FTD_T_DELI_MARGIN));

			ExchFare efPosiMoney;
			double d_cov_surplus;

			// 获取平仓释放的保证金
			HandleDeferCovReleaseFee(sProd_Code, dPrice, iHand, info.measure_unit, true, efPosiMoney, d_cov_surplus);

			//kenny 20180507
			//4045  需要改成冻结100%交割保证金
			//fTotalMoney = ef_DDA_margin.sumSingle() - efPosiMoney.sumSingle();
			fTotalMoney = ef_DDA_margin.sumSingle();

			// 如果是报单信息则记录报单的冻结信息
			if (!localOrderNo.isEmpty())
			{
				OrderFrozeInfo stFrozeInfo;
				stFrozeInfo.Init(fTotalMoney, 0.00, 0.00, iHand);

				m_QMapOrderFroze[localOrderNo] = stFrozeInfo;
			}
		}
		//kenny  20180507  
		else if (sExchID == CONSTANT_EXCH_CODE_MIDD_DELIVERY_LONG || sExchID == CONSTANT_EXCH_CODE_MIDD_DELIVERY_SHORT) // 中立仓  || sExchID == CONSTANT_EXCH_CODE_MIDD_DELIVERY_SHORT
		{
			ExchFare ef_entr_margin = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, CONSTANT_EXCH_BAIL_FARE_ID), GetMFareValue(sProd_Code, CONSTANT_EXCH_BAIL_FARE_ID));

			if (sExchID == CONSTANT_EXCH_CODE_MIDD_DELIVERY_LONG) // 收金
			{
				// 交割保证金（会员和交易所）
				ExchFare ef_deli_prepare = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, CONSTANT_FTD_T_DELI_MARGIN), GetMFareValue(sProd_Code, CONSTANT_FTD_T_DELI_MARGIN));


				double frozen = g_CusInfo.CalculateFrozenCapital(sProd_Code, sExchID, dOrgCost);

				// 开仓手续费+开仓保证金
				if (frozen <= -1)
					fTotalMoney = ef_deli_prepare.sumSingle() + ef_entr_margin.sumSingle();
				else
					fTotalMoney = ef_deli_prepare.sumSingle() + frozen;

				//fTotalMoney = ef_entr_margin.sumSingle() + ef_deli_prepare.sumSingle();
			}
			else // 交金
			{
				fTotalMoney = ef_entr_margin.sumSingle();
			}

			// 如果是报单信息则记录报单的冻结信息
			if (!localOrderNo.isEmpty())
			{
				OrderFrozeInfo stFrozeInfo;
				stFrozeInfo.Init(fTotalMoney, 0.00, 0.00, iHand);

				m_QMapOrderFroze[localOrderNo] = stFrozeInfo;
			}
		}
	}
	else // 远期
	{
		// 获取手续费信息（交易所和会员）
		ExchFare ef_exch_fare = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, CONSTANT_OPEN_FARE_ID), GetMFareValue(sProd_Code, CONSTANT_OPEN_FARE_ID));

		if (bMatch) // 如果是远期成交
		{
			// 计算成交的话只需扣除手续费
			fTotalMoney = ef_exch_fare.sum();
		}
		else // 如果是报单
		{
			// 保证金计算（会员和交易所）
			ExchFare ef_margin = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, CONSTANT_EXCH_BAIL_FARE_ID), GetMFareValue(sProd_Code, CONSTANT_EXCH_BAIL_FARE_ID));

			// 开仓手续费+开仓保证金
			fTotalMoney = ef_exch_fare.sumSingle() + ef_margin.sumSingle();

			// 如果是报单信息则记录报单的冻结信息
			if (!localOrderNo.isEmpty())
			{
				OrderFrozeInfo stFrozeInfo;
				stFrozeInfo.Init(ef_margin.sumSingle(), 0.00, ef_exch_fare.sumSingle(), iHand);

				m_QMapOrderFroze[localOrderNo] = stFrozeInfo;
			}
		}
	}

	//fTotalMoney = CHJGlobalFun::DoubleFormat(fTotalMoney);

	return fTotalMoney;
}

bool CTraderCpMgr::GetProdCodeInfo(const QString &sProdCode, ProdCodeInfo &info)
{
	for (size_t i = 0; i < m_vProdCode.size(); i++)
	{
		if (m_vProdCode.at(i).prod_code == sProdCode)
		{
			info = m_vProdCode.at(i);

			return true;
		}
	}

	return false;
}

/*
sProdCode  需要处理的交易品种
sExchID 交易Id
dPrice 报单价格
iHand 报单手数
bFreeze true为冻结，false为解冻
localOrderNo
*/
bool CTraderCpMgr::HandleTradeFee(const QString &sProdCode, const QString &sExchID, double dPrice, int iHand, bool bFreeze, const QString &localOrderNo /*= ""*/)
{
	ProdCodeInfo info;
	if (GetProdCodeInfo(sProdCode, info)) // 如果获取该合约的信息成功
	{
#ifdef _Write_Fund_Log
		QString csLog;
		csLog.Format("调用HandleTradeFee，本地报单号：%s，是否冻结：%d，处理前可用资金：%.2f",
			localOrderNo,
			bFreeze,
			m_stFundInfo.dUseFulBal
		);
		g_Global.WriteLog(csLog);
#endif
		// 计算交易部分费用
		double dTradeFee = CalculateTradeFee(info, sExchID, dPrice, iHand, false, localOrderNo);

		// 计算成交时未处理的解冻费用（因为有时候先收到成交流水再收到报单流水）
		double dReleaseMoney = GetMatchUnReleaseMoney(localOrderNo);
		dTradeFee -= dReleaseMoney;

		// 处理可用资金
		bFreeze ? m_stFundInfo.dUseFulBal -= dTradeFee : m_stFundInfo.dUseFulBal += dTradeFee;

		// 处理交易冻结资金
		bFreeze ? m_stFundInfo.dExchFrozeBal += dTradeFee : m_stFundInfo.dExchFrozeBal -= dTradeFee;

#ifdef _Write_Fund_Log
		QString csqDebug;
		csqDebug.Format("调用HandleTradeFee，处理后可用资金：%.2f，实际交易费用：%.2f，解冻金额：%.2f",
			m_stFundInfo.dUseFulBal,
			dTradeFee,
			dReleaseMoney
		);
		g_Global.WriteLog(csqDebug);
#endif

		return true;
	}

	return false;
}

bool CTraderCpMgr::HandleCancelTradeFee(const QString &sProdCode, int iHand, const QString &localOrderNo)
{
	ProdCodeInfo info;
	if (GetProdCodeInfo(sProdCode, info)) // 如果获取该合约的信息成功
	{
#ifdef _Write_Fund_Log
		QString cslog;
		cslog.Format("调用HandleCancelTradeFee处理解除冻结，本地报单号：%s，品种：%s，处理前可用资金：%.2f",
			localOrderNo,
			sProdCode,
			m_stFundInfo.dUseFulBal);
		g_Global.WriteLog(cslog);
#endif	
		// ---------------------计算解冻费用
		double dFrozenTradeFee = GetRealeaseMoney(localOrderNo, iHand);

		// 处理可用资金
		m_stFundInfo.dUseFulBal += dFrozenTradeFee;

		// 处理交易冻结资金
		m_stFundInfo.dExchFrozeBal -= dFrozenTradeFee;

#ifdef _Write_Fund_Log
		QString csqDebug;
		csqDebug.Format("调用HandleCancelTradeFee处理解除冻结，本地报单号：%s，品种：%s，解冻后可用资金：%.2f，解冻费用：%.2f",
			localOrderNo,
			sProdCode,
			m_stFundInfo.dUseFulBal,
			dFrozenTradeFee);
		g_Global.WriteLog(csqDebug);
#endif	

		return true;
	}

	return false;
}
// 处理一笔现货成交单后资金的变化，处理成功则返回true，失败则返回false
// sProdCode（合约代码）  sExchID（交易ID） dPrice（成交价格） iHand（成交手数） bSell（是否卖出）
bool CTraderCpMgr::HandleSpotMatchFee(const QString &sProdCode, const QString &sExchID, double dPrice, int iHand, bool bSell, const QString &localOrderNo /*= ""*/)
{
	ProdCodeInfo info;
	if (GetProdCodeInfo(sProdCode, info)) // 如果获取该合约的信息成功
	{
		double dSellMatchBal, dSellExchFare;

		// 计算交易成交后所产生的全部金额（买入则为需要扣取的全部金额，卖出则为需要增加的全部金额）
		double dTradeFee;
		if (!bSell)
		{
			dTradeFee = CalculateTradeFee(info, sExchID, dPrice, iHand, true);
		}
		else
		{
			CalSpotSellMatchFee(info, dPrice, iHand, dSellMatchBal, dSellExchFare);

			dTradeFee = dSellMatchBal - dSellExchFare;
		}

		// ---------------------计算报单时候的冻结费用
		double dFrozenTradeFee = 0.00; //
		if (!bSell) // 
		{
			dFrozenTradeFee = HandleMatchReleaseMoney(localOrderNo, iHand);
		}

		// 卖出则增加金额，买入则减少金额
		bSell ? m_stFundInfo.dAllBalance += dTradeFee : m_stFundInfo.dAllBalance -= dTradeFee;

		// 可用资金处理，卖出成交则增加交易金额，同时冻结10%的本金；
		// 如果是买入，则解冻（增加）交易冻结的部分，然后减去全额的交易费用
		if (bSell) // 卖出
		{
			m_stFundInfo.dUseFulBal += dFrozenTradeFee + CHJGlobalFun::DoubleFormat(dSellMatchBal*0.9) - dSellExchFare;
		}
		else // 买入
		{
			m_stFundInfo.dUseFulBal += dFrozenTradeFee - dTradeFee;
		}

		// 处理交易冻结，如果是卖出则冻结10%的本金；如果是买入，则解冻报入时的冻结金额
		if (bSell)
		{
			m_stFundInfo.dExchFrozeBal += CHJGlobalFun::DoubleFormat(dSellMatchBal*0.1);
		}
		else
		{
			m_stFundInfo.dExchFrozeBal -= dFrozenTradeFee;
		}

		return true;
	}

	return false;
}

// 处理一笔远期成交后的资金变化
bool CTraderCpMgr::HandleForwardMatchFee(const QString &sProdCode, const QString &sExchID, double dPrice, int iHand, double &dPosiMoney, const QString &localOrderNo)
{
	ProdCodeInfo info;
	if (GetProdCodeInfo(sProdCode, info)) // 如果获取该合约的信息成功
	{
		// 计算交易产生的费用
		double dExchFare;
		CalForwardMatchFee(info, dPrice, iHand, dPosiMoney, dExchFare);

		// 计算报单时候的冻结费用
		double dFrozenTradeFee = HandleMatchReleaseMoney(localOrderNo, iHand);

		// 处理总资金：总额减少手续费的部分
		m_stFundInfo.dAllBalance -= dExchFare;

		// 处理可用资金：
		m_stFundInfo.dUseFulBal -= dExchFare + dPosiMoney - dFrozenTradeFee;

		// 处理冻结资金
		m_stFundInfo.dExchFrozeBal -= dFrozenTradeFee;

		return true;
	}

	return false;
}

// 处理一笔延期成交后的资金变化；bSell（是否开仓） 
bool CTraderCpMgr::HandleDeferMatchFee(const QString &sProdCode, const QString &sExchID, double dPrice, int iHand, bool bOpen, double &dPosiMoney, const QString &localOrderNo /*= ""*/)
{
	dPosiMoney = 0.00;
	ProdCodeInfo info;
	if (GetProdCodeInfo(sProdCode, info)) // 如果获取该合约的信息成功
	{
#ifdef _Write_Fund_Log
		QString csLog;
		csLog.Format("调用HandleDeferMatchFee处理延期成交，本地报单号为：%s，品种：%s，处理前可用金额为：%.2f，总额为：%.2f",
			localOrderNo,
			sProdCode,
			m_stFundInfo.dUseFulBal,
			m_stFundInfo.dAllBalance);
		g_Global.WriteLog(csLog);
#endif

		// 计算交易产生的费用
		double dExchFare; // 交易手续费，持仓保证金
		CalDeferMatchFee(info, dPrice, iHand, dPosiMoney, dExchFare, bOpen);

		// 计算报单时候的冻结费用（平仓的时候是解冻的费用）
		double dFrozenTradeFee = HandleMatchReleaseMoney(localOrderNo, iHand);

		// 处理总资金：总额减少手续费的部分
		m_stFundInfo.dAllBalance -= dExchFare;

		// 处理可用资金
		m_stFundInfo.dUseFulBal += dFrozenTradeFee - dExchFare - dPosiMoney;

		// 处理冻结资金
		m_stFundInfo.dExchFrozeBal -= dFrozenTradeFee;

#ifdef _Write_Fund_Log
		QString csqDebug;
		csqDebug.Format("调用HandleDeferMatchFee处理延期成交，本地报单号为：%s，品种：%s，处理后可用金额为：%.2f，总额为：%.2f，冻结（解冻）金额为：%.2f，交易费用：%.2f，持仓保证金：%.2f，总费用：%.2f",
			localOrderNo,
			sProdCode,
			m_stFundInfo.dUseFulBal,
			m_stFundInfo.dAllBalance,
			dFrozenTradeFee,
			dExchFare,
			dPosiMoney,
			dFrozenTradeFee - dExchFare - dPosiMoney);
		g_Global.WriteLog(csqDebug);
#endif

		return true;
	}

	return false;
}

// 处理延期平仓成交的资金信息 bLong
bool CTraderCpMgr::HandleDeferCovMatchFee(const QString &sProdCode, double dPrice, int iHand, bool bLong)
{
	ProdCodeInfo info;
	if (GetProdCodeInfo(sProdCode, info)) // 如果获取该合约的信息成功
	{
#ifdef _Write_Fund_Log
		QString csLog;
		csLog.Format("调用HandleDeferCovMatchFee处理延期平仓成交，品种：%s，处理前可用金额为：%.2f，总金额为：%.2f",
			sProdCode,
			m_stFundInfo.dUseFulBal,
			m_stFundInfo.dAllBalance);
		g_Global.WriteLog(csLog);
#endif

		// 计算交易产生的费用
		ExchFare efExchFare, efPosiMoney;
		double d_cov_surplus;

		CalDeferCovMatchFee(info, dPrice, iHand, bLong, efPosiMoney, efExchFare, d_cov_surplus);

		m_stFundInfo.dAllBalance += d_cov_surplus - efExchFare.sum();

		m_stFundInfo.dUseFulBal += efPosiMoney.sum() - efExchFare.sum() + d_cov_surplus;

#ifdef _Write_Fund_Log
		QString csqDebug;
		csqDebug.Format("调用HandleDeferCovMatchFee处理延期平仓成交，品种：%s，处理后可用金额为：%.2f，总金额为：%.2f，efPosiMoney为：%.2f，efExchFare为：%.2f，d_cov_surplus为：%.2f",
			sProdCode,
			m_stFundInfo.dUseFulBal,
			m_stFundInfo.dAllBalance,
			efPosiMoney.sum(),
			efExchFare.sum(),
			d_cov_surplus);
		g_Global.WriteLog(csqDebug);
#endif

		return true;
	}

	return false;
}


bool CTraderCpMgr::HandleDDAMatchFee(const QString &sProdCode, int iHand, bool bBuy, const QString &localOrderNo /*= ""*/)
{
	ProdCodeInfo info;
	if (GetProdCodeInfo(sProdCode, info)) // 如果获取该合约的信息成功
	{
		ExchFare ef_m_deli_prepare;
		ExchFare ef_m_release_margin;
		GetDDATradeFee(info, bBuy, iHand, ef_m_deli_prepare, ef_m_release_margin);

		if (bBuy) // 如果是收金
		{
			// 计算报单时候的冻结费用（平仓的时候是解冻的费用）
			double dFrozenTradeFee = HandleMatchReleaseMoney(localOrderNo, iHand);

			// 释放可用资金的冻结
			m_stFundInfo.dUseFulBal += dFrozenTradeFee - (ef_m_deli_prepare.sum() - ef_m_release_margin.sum());

			// 释放交易冻结
			m_stFundInfo.dExchFrozeBal -= dFrozenTradeFee;
		}
		else
		{
			m_stFundInfo.dUseFulBal += ef_m_release_margin.sum();
		}
	}

	return false;
}

// 处理中立仓成交的资金信息，交金成交后不做资金的处理
bool CTraderCpMgr::HandleMAMatchFee(const QString &sProdCode, int iHand, bool bBuy, const QString &localOrderNo /*= ""*/)
{
	//return true;

	ProdCodeInfo info;
	if (GetProdCodeInfo(sProdCode, info)) // 如果获取该合约的信息成功
	{
		// 不处理成交的扣除，仅仅处理释放交金的时候冻结的资金
		// 		if(bBuy)
		// 		{
		// 			// 获取成交的扣除金额
		// 			ExchFare ef_m_deli_prepare;
		// 			ExchFare ef_m_margin;
		// 			GetMAMatchFee(info, bBuy, iHand, ef_m_deli_prepare, ef_m_margin);
		// 			
		// 			// 获取成交后要释放的冻结资金
		// 			double dFrozenTradeFee  = HandleMatchReleaseMoney(localOrderNo, iHand);
		// 
		// 			m_stFundInfo.dUseFulBal += dFrozenTradeFee - (ef_m_deli_prepare.sum() + ef_m_margin.sum());
		// 
		// 			// 释放交易冻结
		// 			m_stFundInfo.dExchFrozeBal -= dFrozenTradeFee;
		// 		}

		// 获取成交的扣除金额
		ExchFare ef_m_deli_prepare;
		ExchFare ef_m_margin;
		GetMAMatchFee(info, bBuy, iHand, ef_m_deli_prepare, ef_m_margin);

		// 获取成交后要释放的冻结资金
		double dFrozenTradeFee = HandleMatchReleaseMoney(localOrderNo, iHand);

		//if(bBuy)
		// 
		m_stFundInfo.dUseFulBal += dFrozenTradeFee - (ef_m_deli_prepare.sum() + ef_m_margin.sum());

		// 释放交易冻结
		m_stFundInfo.dExchFrozeBal -= dFrozenTradeFee;

	}

	return false;
}

void CTraderCpMgr::CalDeferCovMatchFee(const ProdCodeInfo &info, double dPrice, int iHand, bool bLong, ExchFare &efPosiMoney, ExchFare &efExchFare, double &d_cov_surplus)
{
	// 合约代码
	const QString &sProd_Code = info.prod_code;

	// 获取每手的计量单位 （克）
	double dUnite = CHJCommon::HandToGram(sProd_Code, info.measure_unit);

	// 本金（发生货款）
	double dOrgCost = dUnite * iHand * dPrice;

	// 获取手续费信息（交易所和会员）
	efExchFare = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, CONSTANT_COV_FARE_ID), GetMFareValue(sProd_Code, CONSTANT_COV_FARE_ID), true);

	// 获取平仓释放的保证金
	HandleDeferCovReleaseFee(sProd_Code, dPrice, iHand, info.measure_unit, bLong, efPosiMoney, d_cov_surplus);
}

// 计算平仓的时候释放的保证金和持仓盈亏
void CTraderCpMgr::HandleDeferCovReleaseFee(const QString &sProdCode, double dPrice, int iHand, double dMeasure_unit, bool bLong, ExchFare &ef_m_release_margin, double &d_cov_surplus)
{
	d_cov_surplus = 0.00;

	// 根据持仓方向获取对应的持仓信息
	QMap<QString, DeferPosi>::iterator it = m_QMapDeferPosi.find(sProdCode);
	if (it == m_QMapDeferPosi.end())
	{
		return;
	}
	DeferPosiInfo &info = bLong ? it->infoLong : it->infoShort;
	LISTDeferMatch &listMatch = info.listMatch;

	int iCovYes;
	int iCovToday;
	if (info.iYesAmt > iHand || info.iYesAmt == iHand) // 如果昨仓能满足平仓
	{
		iCovYes = iHand;
		iCovToday = 0;
	}
	else
	{
		iCovYes = info.iYesAmt;
		iCovToday = iHand - info.iYesAmt;
	}

	double dDifPrice;
	dMeasure_unit = CHJCommon::HandToGram(sProdCode, dMeasure_unit);

	// 先平掉昨天的仓位
	if (iCovYes > 0)
	{
		// 释放的持仓保证金
		ef_m_release_margin.dMemberFare += info.dYesPosiMoney * iCovYes / info.iYesOrgAmt;

		// 平多仓(bLong=true)   = (平仓价-开仓价）×当前平仓数量×每手重量
		dDifPrice = bLong ? (dPrice - info.fYesAvgPosPrice) : (info.fYesAvgPosPrice - dPrice);
		d_cov_surplus += dDifPrice*dMeasure_unit*iCovYes; // 浮动盈亏
	}

	if (iCovToday > 0)
	{
		const LISTDeferMatch &listMatch = info.listMatch;
		for (LISTDeferMatch::const_iterator it = listMatch.begin(); it != listMatch.end(); )
		{
			dDifPrice = bLong ? (dPrice - it->dPrice) : (it->dPrice - dPrice);

			if (it->iHand > iCovToday || it->iHand == iCovToday) // 如果当前流水的手数不小于剩余手数
			{
				ef_m_release_margin.dMemberFare += it->dPosiMoney * iCovToday / it->iTotalHand;
				d_cov_surplus += dDifPrice*dMeasure_unit*iCovToday; // 浮动盈亏
				break;
			}
			else // 如果当前流水的手数小于剩余手数，则记录未被平掉的手数
			{
				if (it->iHand > 0)
				{
					iCovToday -= it->iHand;
					ef_m_release_margin.dMemberFare += it->dPosiMoney*it->iHand / it->iTotalHand;
					d_cov_surplus += dDifPrice*dMeasure_unit*it->iHand; // 浮动盈亏
				}

				it++;
			}
		}
	}
}

void CTraderCpMgr::GetDDATradeFee(const ProdCodeInfo &info, bool bLong, int iHand, ExchFare &ef_m_deli_prepare, ExchFare &ef_m_release_margin)
{
	const QString &sProdCode = info.prod_code;

	double dPrice = GetDeferDeliveryPrice(sProdCode);

	// 获取每手的计量单位 （克）
	double dUnite = CHJCommon::HandToGram(sProdCode, info.measure_unit);

	if (bLong) // 只有收金才要计算交割准备金，收金不用
	{
		// 本金（发生货款）
		double dOrgCost = dUnite * iHand * dPrice;

		// 交割保证金（会员和交易所）
		ef_m_deli_prepare = CHJCommon::GetExchFareValueByExchBal(sProdCode, dUnite, dOrgCost, iHand, GetBFareValue(sProdCode, CONSTANT_FTD_T_DELI_MARGIN), GetMFareValue(sProdCode, CONSTANT_FTD_T_DELI_MARGIN));
	}

	double d_cov_surplus;
	HandleDeferCovReleaseFee(sProdCode, dPrice, iHand, dUnite, bLong, ef_m_release_margin, d_cov_surplus);

	// 开仓的全部交易金额
	//double dOpenFrozFare = GetOpenFareOrg(sProdCode, true, dUnite, iHand);

	// 开仓保证金计算（会员和交易所）
	//ef_m_release_margin = CHJCommon::GetExchFareValueByExchBal(sProdCode, dUnite, dOpenFrozFare, iHand,  GetBFareValue(sProdCode, CONSTANT_EXCH_BAIL_FARE_ID), GetMFareValue(sProdCode, CONSTANT_EXCH_BAIL_FARE_ID));
}

void CTraderCpMgr::GetMAMatchFee(const ProdCodeInfo &info, bool bLong, int iHand, ExchFare &ef_m_deli_prepare, ExchFare &ef_m_release_margin)
{
	const QString &sProdCode = info.prod_code;

	double dPrice = GetMAOrderPrice(sProdCode);

	// 获取每手的计量单位 （克）
	double dUnite = CHJCommon::HandToGram(sProdCode, info.measure_unit);

	double dOrgCost = dUnite * iHand * dPrice;

	ef_m_release_margin = CHJCommon::GetExchFareValueByExchBal(sProdCode, dUnite, dOrgCost, iHand, GetBFareValue(sProdCode, CONSTANT_OPEN_FARE_ID), GetMFareValue(sProdCode, CONSTANT_OPEN_FARE_ID));

	if (bLong) // 收金
	{
		// 交割保证金（会员和交易所）
		ef_m_deli_prepare = CHJCommon::GetExchFareValueByExchBal(sProdCode, dUnite, dOrgCost, iHand, GetBFareValue(sProdCode, CONSTANT_FTD_T_DELI_MARGIN), GetMFareValue(sProdCode, CONSTANT_FTD_T_DELI_MARGIN));
	}
}

// 该部分代码
void CTraderCpMgr::CalForwardMatchFee(const ProdCodeInfo &info, double dPrice, int iHand, double &dPosiMoney, double &dExchFare)
{
	// 合约代码
	const QString &sProd_Code = info.prod_code;

	// 获取每手的计量单位 （克）
	double dUnite = CHJCommon::HandToGram(sProd_Code, info.measure_unit);

	// 本金（发生货款）   计量单位*委托价格*手数，（注：参照服务器代码，对结果做了处理）
	//double dOrgCost = CHJGlobalFun::DoubleFormat(dUnite * iHand * dPrice );
	double dOrgCost = dUnite * iHand * dPrice;

	// 获取手续费信息（交易所和会员）
	ExchFare ef_exch_fare = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, CONSTANT_OPEN_FARE_ID), GetMFareValue(sProd_Code, CONSTANT_OPEN_FARE_ID), true);

	// 保证金计算（会员和交易所）
	ExchFare ef_margin = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, CONSTANT_EXCH_BAIL_FARE_ID), GetMFareValue(sProd_Code, CONSTANT_EXCH_BAIL_FARE_ID), true);

	// 成交持仓总额
	dPosiMoney = ef_margin.sum();
	// 成交手续费总额
	dExchFare = ef_exch_fare.sum();
}

// 计算延期成交所涉及的资金计算：所要扣除的成交手续费以及保证金（开仓则为冻结，平仓则为解冻）
void CTraderCpMgr::CalDeferMatchFee(const ProdCodeInfo &info, double dPrice, int iHand, double &dPosiMoney, double &dExchFare, bool bOpen)
{
	// 合约代码
	const QString &sProd_Code = info.prod_code;

	// 获取每手的计量单位 （克）
	double dUnite = CHJCommon::HandToGram(sProd_Code, info.measure_unit);

	// 本金（发生货款）
	double dOrgCost = dUnite * iHand * dPrice;

	// 获取手续费信息（交易所和会员）
	QString sExchCode = (bOpen) ? CONSTANT_OPEN_FARE_ID : CONSTANT_COV_FARE_ID;
	ExchFare ef_exch_fare = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, sExchCode), GetMFareValue(sProd_Code, sExchCode), true);

	// 保证金计算（会员和交易所）
	ExchFare ef_margin = CHJCommon::GetExchFareValueByExchBal(sProd_Code, dUnite, dOrgCost, iHand, GetBFareValue(sProd_Code, CONSTANT_EXCH_BAIL_FARE_ID), GetMFareValue(sProd_Code, CONSTANT_EXCH_BAIL_FARE_ID), true);

	// 成交持仓总额
	dPosiMoney = ef_margin.sum();
	// 成交手续费总额
	dExchFare = ef_exch_fare.sum();
}

// 处理现货成交的库存变动：如果是现货买入成交，则bAdd为true，则总库存和可用库存增加；如果是现货卖出成交，bAdd为false，则冻结减少，总库存减少
bool CTraderCpMgr::HandleMatchStore(const QString &sProdCode, int iHand, bool bAdd)
{
	auto itStore = m_QMapStoreInfo.find(sProdCode);
	if (itStore == m_QMapStoreInfo.end()) // 不存在则插入
	{
		StoreInfo body;
		//auto Insert_Pair = m_QMapStoreInfo.insert(sProdCode, body);
		//itStore = Insert_Pair.key();
		itStore = m_QMapStoreInfo.insert(sProdCode, body);

	}

	ProdCodeInfo info;
	if (GetProdCodeInfo(sProdCode, info))
	{
		StoreInfo &stStore = itStore.value();

#ifdef _WRITE_SPOT_LOG
		QString csLog1;
		csLog1.Format("处理现货成交库存，品种%s，处理前分别为%d,%d", stStore.iUseFulAmt, stStore.iExchFreeze);
		g_Global.WriteLog(csLog1);
#endif

		// 获得品种基本单位（即一手的克数）
		int iTotal = iHand*(int)(info.measure_unit + 0.5);

		// 处理总库存
		bAdd ? stStore.iAllAmt += iTotal : stStore.iAllAmt -= iTotal;

		// added by Jerry Lee, 2013-3-25, 避免可用库存小于零的情况
		if (stStore.iAllAmt < 0)
		{
			stStore.iAllAmt = 0;
		}

		// 处理可用库存，如果是买入，则增加库存，如果是卖出，由于报单的时候已经减少了，所以这里不做处理
		if (bAdd)
		{
			stStore.iUseFulAmt += iTotal;
		}

		// 如果是卖出，则冻结减少，如果是买入，则不做处理
		if (!bAdd)
		{
			stStore.iExchFreeze -= iTotal;
		}

#ifdef _WRITE_SPOT_LOG
		QString csqDebug;
		csqDebug.Format("处理现货成交库存，品种%s，处理前分别为%d,%d", stStore.iUseFulAmt, stStore.iExchFreeze);
		g_Global.WriteLog(csqDebug);
#endif

		return true;
	}


	return false;
}

/*
处理报单时的库存冻结或解冻
bFreeze：true则为冻结，false则为解冻
sDDAProdCode：该值为空时表示处理的是普通的现货报单
当该值不为空的时候表示这时候处理的是延期交收或者中立仓，处理的是因为交收时可采用代替交割品种，
这时候冻结的品种采用sDDAProdCode的
*/
bool CTraderCpMgr::HandleTradeStore(const QString &sProdCode, int iHand, bool bFreeze, const QString &sDDAProdCode)
{
	// 根据合约代码找到该笔库存
	QMap<QString, StoreInfo>::iterator itStore = m_QMapStoreInfo.find(sProdCode);
	if (itStore != m_QMapStoreInfo.end())
	{
#ifdef _WRITE_SPOT_LOG
		QString csLog1;
		csLog1.Format("处理报单库存，品种%s，处理前分别为%d,%d", itStore.value.iUseFulAmt, itStore.value.iExchFreeze);
		g_Global.WriteLog(csLog1);
#endif

		// 获得品种信息
		ProdCodeInfo info;

		// 这里主要是为了获取品种的计量单位，如果sDDAProdCode不为空，则获取sDDAProdCode的计量单位，为空则获取sProdCode的计量单位
		bool bGetInfoSuc = false;
		if (sDDAProdCode.isEmpty())
			bGetInfoSuc = GetProdCodeInfo(sProdCode, info);
		else
			bGetInfoSuc = GetProdCodeInfo(sDDAProdCode, info);

		if (bGetInfoSuc)
		{
			// 获得品种库存信息
			StoreInfo &stStore = itStore.value();

			// 获取一手的克数，加上0.5是解决浮点数的不精确问题
			int iUnit = (int)(CHJCommon::HandToGram(sProdCode, info.measure_unit) + 0.5);

			// 处理可用库存：如果是冻结，则可用库存减少；如果是解冻，则可用库存增加
			bFreeze ? stStore.iUseFulAmt -= iHand*iUnit : stStore.iUseFulAmt += iHand*iUnit;

			// 处理交易冻结：如果是冻结，则交易冻结增加；如果是解冻，则交易冻结减少
			bFreeze ? stStore.iExchFreeze += iHand*iUnit : stStore.iExchFreeze -= iHand*iUnit;

#ifdef _WRITE_SPOT_LOG
			QString csqDebug;
			csqDebug.Format("处理报单库存，品种%s，处理后分别为%d,%d", itStore.value.iUseFulAmt, itStore.value.iExchFreeze);
			g_Global.WriteLog(csqDebug);
#endif

			return true;
		}
	}

	return false;
}
// 初始化代替交割品种信息
bool CTraderCpMgr::Ini1005()
{
	m_htmSubVariety.clear();

	//QString strDataPath = g_Global.GetSystemPath() + CONSTANT_DATA_INI_NAME;
	//char buf[500] = {0};

	//// 获取本地保存的交割替代品种
	//   // mod by Jerry Lee, 2013-3-27, 增加解析本地文件出错后从服务器端取数据的处理
	//::GetPrivateProfileQString("info", "VARIETY_SUBSTITUE", "", buf, sizeof(buf), strDataPath);
	//   int nParseRet = -1;
	//if(strlen(buf) > 10)
	//{
	//	// 解析
	//	QString str = buf;
	//	nParseRet = m_htmSubVariety.Parse(str);
	//}
	//
	//   if (nParseRet != 0)
	//   {
	//	// 本地没有的话则向服务器请求
	//	Rsp1005 rsp1005;

	//	// 发送报文
	//	int iReturn = CTranMessage::Handle1005(rsp1005);
	//	if(iReturn == 0)
	//	{
	//           // 保存数据
	//		::WritePrivateProfileQString("Info", "VARIETY_SUBSTITUE", rsp1005.htm_variety_info.ToQString(),    
	//               strDataPath);
	//		// 记录到内存
	//		m_htmSubVariety = rsp1005.htm_variety_info;
	//	}
	//	else
	//	{
	//		return false;
	//	}
	//}

	return true;
}

bool CTraderCpMgr::GetCusSubVarietyName(const QString &sProdCode, QString &sSubVAriety, const QString &sSeq)
{
	// 获取第一位的交割品种的ID
	QString sVarietyID = GetSubVarietyBySeqNo(sProdCode, sSeq);
	if (!sVarietyID.isEmpty())
	{
		// 根据ID获取交割品种信息
		VarietyInfo info;
		if (GetVarietyInfo(sVarietyID, info))
		{
			// 判断客户是否该品种的库存
			if (m_QMapStoreInfo.count(info.name) > 0)
			{
				if (m_QMapStoreInfo[info.name].iUseFulAmt > 0)
				{
					sSubVAriety = info.name;
					return true;
				}
			}
		}
	}

	return false;
}
// 根据延期交金报单的品种获取代替的交割品种的名称
QString CTraderCpMgr::GetSubVarietyProdCode(const QString &sProdCode)
{
	// 确保内存中已经加载
	if (m_htmSubVariety.size() == 0)
	{
		if (!Ini1005())
		{
			return "";
		}
	}

	QString sSubVAriety;
	if (!GetCusSubVarietyName(sProdCode, sSubVAriety, "1"))
	{
		GetCusSubVarietyName(sProdCode, sSubVAriety, "2");
	}

	return sSubVAriety;

	/*
	// 获取第一位的交割品种的ID
	QString sVarietyID = GetSubVarietyBySeqNo(sProdCode, "1");
	if(!sVarietyID.isEmpty())
	{
	// 根据ID获取交割品种信息
	VarietyInfo info;
	if(GetVarietyInfo(sVarietyID, info))
	{
	// 判断客户是否该品种的库存
	if(m_QMapStoreInfo.count(info.name) > 0)
	{
	return info.name;
	}
	}
	}
	else
	{
	// 如果第一位的交割品种处理没品种，则处理第二位
	sVarietyID = GetSubVarietyBySeqNo(sProdCode, "2");

	// 根据ID获取交割品种信息
	VarietyInfo info;
	if(GetVarietyInfo(sVarietyID, info))
	{
	// 判断客户是否该品种的库存，至于库存是否足够，报单的时候服务器已经做了判断，所以不需要处理了
	if(m_QMapStoreInfo.count(info.name) > 0)
	{
	return info.name;
	}
	}
	}

	return "";
	*/
}

// 根据交割品种ID获取交割品种信息，没有找到则返回false，否则为true
bool CTraderCpMgr::GetVarietyInfo(const QString &sVarietyID, VarietyInfo &info)
{
	for (size_t i = 0; i < m_vVariety.size(); i++)
	{
		if (m_vVariety.at(i).variety_id == sVarietyID)
		{
			info = m_vVariety.at(i);
			return true;
		}
	}

	return false;
}

// 根据合约品种和交割顺序找到交割代替品种的ID
QString CTraderCpMgr::GetSubVarietyBySeqNo(const QString &sProdCode, const QString &sSeqNo)
{
	for (size_t i = 0; i < m_htmSubVariety.size(); i++)
	{
		if (m_htmSubVariety.GetString(i, "prod_code") == sProdCode.toStdString()
			&& m_htmSubVariety.GetString(i, "seq_no") == sSeqNo.toStdString())
		{
			return m_htmSubVariety.GetString(i, "variety_id").c_str();
		}
	}

	return "";
}
// 获取延期交收对资金处理所采用的价格
double CTraderCpMgr::GetDeferDeliveryPrice(const QString &sProdCode)
{
	// 获取涨停板率和价格方式
	QString sPriceModePara = g_TraderCpMgr.GetParaValue("DeferDeliveryTestPriceBasePrice", "lastSettle");
	QString sUpRatePara = g_TraderCpMgr.GetParaValue("DeferDeliveryTestPriceUpRate", "0.07");

	// 记录最后返回的价格
	double dPrice;

	// 获取行情信息
	const QUOTATION& qt = g_TraderCpMgr.m_QMapQuotation[sProdCode.toStdString()];

	// 根据配置的价格方式取行情对应的价格
	unsigned int uiPrice;
	if (sPriceModePara == "average")
		uiPrice = qt.m_uiAverage;
	else if (sPriceModePara == "lastClose")
		uiPrice = qt.m_uilastClose;
	else
		uiPrice = qt.m_uiLastSettle;

	// 价格的异常处理
	if (uiPrice == 0)
		uiPrice = qt.m_uilastClose;

	// 获取浮点类型的价格，参照服务器对其格式化
	dPrice = CHJGlobalFun::DoubleFormat(uiPrice / g_Global.m_dQTFactor);

	// 加上涨停板率
	dPrice *= (1 + sUpRatePara.toFloat());

	// 参照服务器对数据格式化
	return CHJGlobalFun::DoubleFormat(dPrice);
}

// 根据系统参数ID获取系统参数值，获取失败的话则用默认的（sDefaultValue）
QString CTraderCpMgr::GetParaValue(const QString &sParaID, const QString &sDefaultValue)
{
	QMap<QString, QString>::const_iterator it = m_QMapSystemPara.find(sParaID);
	//if( it != m_QMapSystemPara.end() )
	//{
	//	return it.value;
	//}
	//else
	//{
	//	QString sValue;

	//	Rsp9030 rsp9030;
	//	if(CTranMessage::Handle9030(rsp9030, sParaID) == 0 && rsp9030.htm_para_info.size() > 0)
	//	{
	//		sValue = rsp9030.htm_para_info.GetQString(0, "para_value");
	//		m_QMapSystemPara[sParaID] = sValue;
	//	}
	//	else
	//	{
	//		// 如果向服务器请求失败则用默认的参数
	//		sValue = sDefaultValue;
	//	}

	//	return sValue;
	//}


	return "";
}

// 刷新报单信息
void CTraderCpMgr::RefreshOrderInfo(OrderBase *OrgOrder, const OrderBase *NewOrder)
{
	OrgOrder->status = NewOrder->status;

	// 记录新的广播信息
	if (NewOrder->orderNo != "")
	{
		// 因为一开始记录报单的时候状态为"正在申报"，这时候是没有报单号（二级系统的报单号）的，在金交所报单成功后会有报单号
		OrgOrder->orderNo = NewOrder->orderNo;
	}

	if (OrgOrder->remainAmount >= NewOrder->remainAmount)
	{
		OrgOrder->remainAmount = NewOrder->remainAmount;
		OrgOrder->applyTime = NewOrder->applyTime;

		if (NewOrder->RspMsg.length() > 3)
		{
			OrgOrder->RspMsg = NewOrder->RspMsg;
		}
	}
}

double CTraderCpMgr::GetOpenFare(const QString &sProdCode, bool bLong, double fUnite, const FareInfo &fi, int iHand /*= 1*/)
{
	if (fi.fare_value.isEmpty())
	{
		return 0;
	}


	double dTotalMoney = 0;
	if (fi.fare_mode_id == CONSTANT_CT_FARE_MODE_BAL) // 按金额模式计算保证金
	{
		// 根据持仓方向获取对应的持仓信息
		QMap<QString, DeferPosi>::iterator it = m_QMapDeferPosi.find(sProdCode);
		const DeferPosiInfo &info = bLong ? it->infoLong : it->infoShort;

		// 获取行情中上的上日结算价
		const QUOTATION& qt = g_TraderCpMgr.m_QMapQuotation[sProdCode.toStdString()];
		double dLastSettle = qt.m_uiLastSettle / g_Global.m_dQTFactor;


		// 获取该方向上的持仓信息，判断昨仓是否满足
		if (info.iYesAmt >= iHand)
		{
			dTotalMoney = dLastSettle*fUnite*iHand;
		}
		else
		{
			// 昨仓未能平掉的部分
			iHand -= info.iYesAmt;
			dTotalMoney = dLastSettle*fUnite*info.iYesAmt;

			// 加上今仓的平仓手续费
			const LISTDeferMatch &listMatch = info.listMatch;
			for (LISTDeferMatch::const_iterator it = listMatch.begin(); it != listMatch.end(); )
			{
				if (it->iHand > iHand || it->iHand == iHand) // 如果当前流水的手数不小于剩余手数
				{
					dTotalMoney += iHand * it->dPrice;
					break;
				}
				else // 如果当前流水的手数小于剩余手数，则记录未被平掉的手数，并删除当前流水
				{
					iHand -= it->iHand;
					dTotalMoney += it->iHand * it->dPrice;
					it++;
				}
			}
		}

		dTotalMoney *= fi.fare_value.toFloat();
	}

	return dTotalMoney;
}

double CTraderCpMgr::GetOpenFareOrg(const QString &sProdCode, bool bLong, double fUnite, int iHand /*= 1*/)
{
	double dTotalMoney;

	// 根据持仓方向获取对应的持仓信息
	QMap<QString, DeferPosi>::iterator it = m_QMapDeferPosi.find(sProdCode);
	const DeferPosiInfo &info = bLong ? it->infoLong : it->infoShort;

	// 获取行情中上的上日结算价，也可用从内存中的持仓信息获得
	const QUOTATION& qt = g_TraderCpMgr.m_QMapQuotation[sProdCode.toStdString()];
	double dLastSettle = qt.m_uiLastSettle / g_Global.m_dQTFactor;

	// 判断昨仓是否满足
	if (info.iYesAmt >= iHand)
	{
		dTotalMoney = dLastSettle*fUnite*iHand; // 是否要格式化
	}
	else
	{
		// 昨仓未能平掉的部分
		iHand -= info.iYesAmt;
		dTotalMoney = dLastSettle*fUnite*info.iYesAmt;

		// 加上今仓的平仓手续费
		const LISTDeferMatch &listMatch = info.listMatch;
		for (LISTDeferMatch::const_iterator it = listMatch.begin(); it != listMatch.end(); )
		{
			if (it->iHand > iHand || it->iHand == iHand) // 如果当前流水的手数不小于剩余手数
			{
				dTotalMoney += iHand * it->dPrice;
				break;
			}
			else // 如果当前流水的手数小于剩余手数，则记录未被平掉的手数，并删除当前流水
			{
				iHand -= it->iHand;
				dTotalMoney += it->iHand * it->dPrice;
				it++;
			}
		}
	}

	return dTotalMoney;
}

double CTraderCpMgr::CalculateMagin(const QString &sProdCode, double dPrice, int iHand, bool bSumSingle /*= true*/)
{
	ProdCodeInfo info;
	if (GetProdCodeInfo(sProdCode, info))
	{
		// 获取每手的计量单位 （克）
		double dUnite = CHJCommon::HandToGram(sProdCode, info.measure_unit);

		// 交易所需要的总金额
		double dTotalMoney = 0.00;

		// 本金（发生货款）   计量单位*委托价格*手数，（注：参照服务器代码，对结果做了处理）
		double dOrgCost = CHJGlobalFun::DoubleFormat(dUnite * iHand * dPrice);

		// 保证金计算（会员和交易所）
		ExchFare ef_margin = CHJCommon::GetExchFareValueByExchBal(sProdCode, dUnite, dOrgCost, iHand, GetBFareValue(sProdCode, CONSTANT_EXCH_BAIL_FARE_ID), GetMFareValue(sProdCode, CONSTANT_EXCH_BAIL_FARE_ID));

		// 开仓手续费-开仓保证金
		if (bSumSingle)
			dTotalMoney = ef_margin.sumSingle();
		else
			dTotalMoney = ef_margin.sum();

		return dTotalMoney;
	}
	else
	{
		return 0.00;
	}
}



// 注册报单广播 hHandle：注册消息的窗口句柄 bSubscrib：是否是订阅，否则为取消订阅 sMarketID：市场类型ID
void CTraderCpMgr::SubscribeOrder(QWidget* hHandle, bool bSubscrib /*= true*/, const QString &sMarketID /*= ""*/)
{
	if (sMarketID.isEmpty() || sMarketID == CONSTANT_B_MARKET_ID_SPOT)
	{
		if (bSubscrib)
		{
			Subscribe(E_ONRECVRTNSPOTORDER, hHandle);
			Subscribe(E_ONRECVRSPSPOTORDER, hHandle);
			Subscribe(E_ONRECVSPOTORDER, hHandle);
		}
		else
		{
			Unsubscribe(E_ONRECVRTNSPOTORDER, hHandle);
			Unsubscribe(E_ONRECVRSPSPOTORDER, hHandle);
			Unsubscribe(E_ONRECVSPOTORDER, hHandle);
		}
	}

	if (sMarketID.isEmpty() || sMarketID == CONSTANT_B_MARKET_ID_FORWARD)
	{
		if (bSubscrib)
		{
			Subscribe(E_ONRECVRSPFORWARDORDER, hHandle);
			Subscribe(E_ONRECVRTNFORWARDORDER, hHandle);
			Subscribe(E_ONRECVDEFERORDER, hHandle);
		}
		else
		{
			Unsubscribe(E_ONRECVRSPFORWARDORDER, hHandle);
			Unsubscribe(E_ONRECVRTNFORWARDORDER, hHandle);
			Unsubscribe(E_ONRECVDEFERORDER, hHandle);
		}
	}

	if (sMarketID.isEmpty() || sMarketID == CONSTANT_B_MARKET_ID_DEFER)
	{
		if (bSubscrib)
		{
			Subscribe(E_ONRECVRTNDEFERORDER, hHandle);
			Subscribe(E_ONRECVRSPDEFERORDER, hHandle);
			Subscribe(E_ONRECVDEFERORDER, hHandle);

			Subscribe(E_ONRECVRTNDEFERDELIVERYAPPORDER, hHandle);
			Subscribe(E_ONRECVRSPDEFERDELIVERYAPPORDER, hHandle);
			Subscribe(E_ONRECVDEFERDELIVERYAPPORDER, hHandle);

			Subscribe(E_ONRECVRTNMIDDLEAPPORDER, hHandle);
			Subscribe(E_ONRECVRSPMIDDLEAPPORDER, hHandle);
			Subscribe(E_ONRECVMIDDLEAPPORDER, hHandle);
		}
		else
		{
			Unsubscribe(E_ONRECVRTNDEFERORDER, hHandle);
			Unsubscribe(E_ONRECVRSPDEFERORDER, hHandle);
			Unsubscribe(E_ONRECVDEFERORDER, hHandle);

			Unsubscribe(E_ONRECVRTNDEFERDELIVERYAPPORDER, hHandle);
			Unsubscribe(E_ONRECVRSPDEFERDELIVERYAPPORDER, hHandle);
			Unsubscribe(E_ONRECVDEFERDELIVERYAPPORDER, hHandle);

			Unsubscribe(E_ONRECVRTNMIDDLEAPPORDER, hHandle);
			Unsubscribe(E_ONRECVRSPMIDDLEAPPORDER, hHandle);
			Unsubscribe(E_ONRECVMIDDLEAPPORDER, hHandle);
		}

	}
}
// 注册撤单广播 hHandle：注册消息的窗口句柄 bSubscrib：是否是订阅，否则为取消订阅 sMarketID：市场类型ID
void CTraderCpMgr::SubscribeOrderCancel(QWidget * hHandle, bool bSubscrib /*= true*/, const QString &sMarketID /*= ""*/)
{
	if (sMarketID.isEmpty() || sMarketID == CONSTANT_B_MARKET_ID_SPOT)
	{
		if (bSubscrib)
		{
			Subscribe(E_ONRECVRTNSPOTORDERCANCEL, hHandle);
			Subscribe(E_ONRECVRSPSPOTORDERCANCEL, hHandle);

		}
		else
		{
			Unsubscribe(E_ONRECVRTNSPOTORDERCANCEL, hHandle);
			Unsubscribe(E_ONRECVRSPSPOTORDERCANCEL, hHandle);
		}
	}

	if (sMarketID.isEmpty() || sMarketID == CONSTANT_B_MARKET_ID_FORWARD)
	{
		if (bSubscrib)
		{
			Subscribe(E_ONRECVRTNFORWARDORDERCANCEL, hHandle);
			Subscribe(E_ONRECVRSPFORWARDORDERCANCEL, hHandle);
		}
		else
		{
			Unsubscribe(E_ONRECVRTNFORWARDORDERCANCEL, hHandle);
			Unsubscribe(E_ONRECVRSPFORWARDORDERCANCEL, hHandle);
		}
	}

	if (sMarketID.isEmpty() || sMarketID == CONSTANT_B_MARKET_ID_DEFER)
	{
		if (bSubscrib)
		{
			Subscribe(E_ONRECVRTNDEFERORDERCANCEL, hHandle);
			Subscribe(E_ONRECVRSPDEFERORDERCANCEL, hHandle);

			Subscribe(E_ONRECVRTNDEFERDELIVERYAPPORDERCANCEL, hHandle);
			Subscribe(E_ONRECVRSPDEFERDELIVERYAPPORDERCANCEL, hHandle);

			Subscribe(E_ONRECVRTNMIDDLEAPPORDERCANCEL, hHandle);
			Subscribe(E_ONRECVRSPMIDDLEAPPORDERCANCEL, hHandle);
		}
		else
		{
			Unsubscribe(E_ONRECVRTNDEFERORDERCANCEL, hHandle);
			Unsubscribe(E_ONRECVRSPDEFERORDERCANCEL, hHandle);

			Unsubscribe(E_ONRECVRTNDEFERDELIVERYAPPORDERCANCEL, hHandle);
			Unsubscribe(E_ONRECVRSPDEFERDELIVERYAPPORDERCANCEL, hHandle);

			Unsubscribe(E_ONRECVRTNMIDDLEAPPORDERCANCEL, hHandle);
			Unsubscribe(E_ONRECVRSPMIDDLEAPPORDERCANCEL, hHandle);
		}
	}
}
// 注册成交单消息 hHandle：注册消息的窗口句柄 bSubscrib：是否是订阅，否则为取消订阅 sMarketID：市场类型ID
void CTraderCpMgr::SubscribeMatch(QWidget* hHandle, bool bSubscrib /*= true*/, const QString &sMarketID /*= ""*/)
{
	if (sMarketID.isEmpty() || sMarketID == CONSTANT_B_MARKET_ID_SPOT)
	{
		if (bSubscrib)
		{
			Subscribe(E_ONRECVRTNSPOTMATCH, hHandle);
		}
		else
		{
			Unsubscribe(E_ONRECVRTNSPOTMATCH, hHandle);
		}
	}

	if (sMarketID.isEmpty() || sMarketID == CONSTANT_B_MARKET_ID_FORWARD)
	{
		if (bSubscrib)
		{
			Subscribe(E_ONRECVRTNFORWARDMATCH, hHandle);
		}
		else
		{
			Unsubscribe(E_ONRECVRTNFORWARDMATCH, hHandle);
		}
	}

	if (sMarketID.isEmpty() || sMarketID == CONSTANT_B_MARKET_ID_DEFER)
	{
		if (bSubscrib)
		{
			Subscribe(E_ONRECVRTNDEFERMATCH, hHandle);
			Subscribe(E_ONRECVRTNDEFERDELIVERYAPPMATCH, hHandle);
		}
		else
		{
			Subscribe(E_ONRECVRTNDEFERMATCH, hHandle);
			Unsubscribe(E_ONRECVRTNDEFERDELIVERYAPPMATCH, hHandle);
		}
	}
}
// 注册所有报单相关的广播
void CTraderCpMgr::SubscribeAll(QWidget* hHandle, bool bSubscrib /*= true*/)
{
	SubscribeOrder(hHandle, bSubscrib);
	SubscribeOrderCancel(hHandle, bSubscrib);
	SubscribeMatch(hHandle, bSubscrib);
}

void CTraderCpMgr::SubscribeDefer(QWidget* hHandle, bool bSubscribe /*= true*/, bool bNormal /*= true*/, bool bDDA /*= true*/, bool bMA /*= true*/)
{
	if (bSubscribe)
	{
		if (bNormal)
		{
			Subscribe(E_ONRECVRTNDEFERORDER, hHandle);
			Subscribe(E_ONRECVRSPDEFERORDER, hHandle);
			Subscribe(E_ONRECVDEFERORDER, hHandle);

			Subscribe(E_ONRECVRTNDEFERORDERCANCEL, hHandle);
			Subscribe(E_ONRECVRSPDEFERORDERCANCEL, hHandle);

			Subscribe(E_ONRECVRTNDEFERMATCH, hHandle);
		}

		if (bDDA)
		{
			Subscribe(E_ONRECVRTNDEFERDELIVERYAPPORDER, hHandle);
			Subscribe(E_ONRECVRSPDEFERDELIVERYAPPORDER, hHandle);
			Subscribe(E_ONRECVDEFERDELIVERYAPPORDER, hHandle);

			Subscribe(E_ONRECVRTNDEFERDELIVERYAPPORDERCANCEL, hHandle);
			Subscribe(E_ONRECVRSPDEFERDELIVERYAPPORDERCANCEL, hHandle);

			Subscribe(E_ONRECVRTNDEFERDELIVERYAPPMATCH, hHandle);
		}

		if (bMA)
		{
			Subscribe(E_ONRECVRTNMIDDLEAPPORDER, hHandle);
			Subscribe(E_ONRECVRSPMIDDLEAPPORDER, hHandle);
			Subscribe(E_ONRECVMIDDLEAPPORDER, hHandle);

			Subscribe(E_ONRECVRTNMIDDLEAPPORDERCANCEL, hHandle);
			Subscribe(E_ONRECVRSPMIDDLEAPPORDERCANCEL, hHandle);
		}
	}
	else
	{
		if (bNormal)
		{
			Unsubscribe(E_ONRECVRTNDEFERORDER, hHandle);
			Unsubscribe(E_ONRECVRSPDEFERORDER, hHandle);
			Unsubscribe(E_ONRECVDEFERORDER, hHandle);

			Unsubscribe(E_ONRECVRTNDEFERORDERCANCEL, hHandle);
			Unsubscribe(E_ONRECVRSPDEFERORDERCANCEL, hHandle);

			Subscribe(E_ONRECVRTNDEFERMATCH, hHandle);
		}

		if (bDDA)
		{
			Unsubscribe(E_ONRECVRTNDEFERDELIVERYAPPORDER, hHandle);
			Unsubscribe(E_ONRECVRSPDEFERDELIVERYAPPORDER, hHandle);
			Unsubscribe(E_ONRECVDEFERDELIVERYAPPORDER, hHandle);

			Unsubscribe(E_ONRECVRTNDEFERDELIVERYAPPORDERCANCEL, hHandle);
			Unsubscribe(E_ONRECVRSPDEFERDELIVERYAPPORDERCANCEL, hHandle);

			Unsubscribe(E_ONRECVRTNDEFERDELIVERYAPPMATCH, hHandle);
		}

		if (bMA)
		{
			Unsubscribe(E_ONRECVRTNMIDDLEAPPORDER, hHandle);
			Unsubscribe(E_ONRECVRSPMIDDLEAPPORDER, hHandle);
			Unsubscribe(E_ONRECVMIDDLEAPPORDER, hHandle);

			Unsubscribe(E_ONRECVRTNMIDDLEAPPORDERCANCEL, hHandle);
			Unsubscribe(E_ONRECVRSPMIDDLEAPPORDERCANCEL, hHandle);
		}
	}

}



//#include "Mgr/WindVaneMgr.h"
void CTraderCpMgr::IniSystemSetting()
{
	QString csPath = g_Global.GetSystemIniPath();

	QString sfile = g_Global.GetUserIniPath();
	QString str;

	//// 锁屏时间 
	//GetPrivateProfileQString("Display", "ScreenTimeout", "0", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	//g_Global.m_TimeOutScreen = atoi(str);
	App::GetPriProfileString(sfile, "Display", "ScreenTimeout", str);
	g_Global.m_TimeOutScreen = str.toInt();
	//// 报单窗口位置
	//GetPrivateProfileQString("Display", "OrderPosition", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Display", "OrderPosition", str);
	g_Global.m_bCommitLeft = (str == "1") ? true : false;
	//// 界面风格
	//GetPrivateProfileQString("Display", "UIStyle", "0", str.GetBuffer(), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Display", "UIStyle", str);
	g_Global.m_iInterfaceStyle = str.toInt();
//// 下单是否确认
	//GetPrivateProfileQString("Order", "ConfirmOrder", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "ConfirmOrder", str);
	g_Global.m_bAskConfirm = (str == "1") ? TRUE : FALSE;
	//// 是否保证买卖队列可见
	//GetPrivateProfileQString("Order", "AlwayShowBS5", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "AlwayShowBS5", str);
	g_Global.m_bAlwaysShowBS5 = (str == "1") ? TRUE : FALSE;
	//// 选择五档行情时修改买卖方向
	//GetPrivateProfileQString("Order", "ChangeBS", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "ChangeBS", str);
	g_Global.m_bChangeBS = (str == "1") ? TRUE : FALSE;

	//// 市价平仓确认
	//GetPrivateProfileQString("Order", "OppCovConfirm", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "OppCovConfirm", str);
	g_Global.m_bOppCovConfirm = (str == "1") ? TRUE : FALSE;
	//// 显示延期方式行
	//GetPrivateProfileQString("Order", "ShowDeferMode", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	//g_Global.m_bShowDeferMode = (str == "1") ? TRUE : FALSE;
	App::GetPriProfileString(sfile, "Order", "ShowDeferMode", str);
	g_Global.m_bShowDeferMode = (str == "1") ? TRUE : FALSE;

	//// 在报单面板选择合约与持仓、库存信息联动
	//GetPrivateProfileQString("Order", "ChiCang", "1" ,str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "ChiCang", str);
	g_Global.m_bInsTriggerInfo = (str == "1") ? TRUE : FALSE;
	//// 双击持仓、库存直接报单
	//GetPrivateProfileQString("Order", "ChiCnag_Click", "0" ,str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	g_Global.m_bChiCang_Click = (str == "1") ? FALSE : TRUE;
	App::GetPriProfileString(sfile, "Order", "ChiCnag_Click", str);
	g_Global.m_bChiCang_Click = (str == "1") ? false : true;

	//// 撤单确认
	//GetPrivateProfileQString("OrderModify", "ConfirmUnorder", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "OrderModify", "ConfirmUnorder", str);
	g_Global.m_bConfirmUnorder = (str == "1") ? TRUE : FALSE;
	//// 平仓确认
	//GetPrivateProfileQString("Order", "CovPosiConfirm", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "CovPosiConfirm", str);
	g_Global.m_bCovPosiConfirm = (str == "1") ? TRUE : FALSE;
	//// 预埋单发送确认
	//GetPrivateProfileQString("Order", "PreOrderConfirm", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "PreOrderConfirm", str);
	g_Global.m_bPreOrderConfirm = (str == "0") ? false : true;
	//// 报单时超过当前持仓量提示
	//GetPrivateProfileQString("Order", "AskOFCfm", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Display", "AskOFCfm", str);
	g_Global.m_bPosiOFConfirm = (str == "0") ? false : true;
	//// 创建预埋单提示
	//GetPrivateProfileQString("Order", "CreatePreOrderTip", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "CreatePreOrderTip", str);
	g_Global.m_bTipsAddPreOrder = (str == "0") ? false : true;
	//// 下单成功提示
	//GetPrivateProfileQString("Order", "TipsAskSus", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "TipsAskSus", str);
	g_Global.m_bTipsAskSus = (str == "1") ? TRUE : FALSE;
	//GetPrivateProfileQString("Order", "SoundAskSus", "", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "SoundAskSus", str);
	g_Global.m_csSoundAskSus = str;
//// 下单失败提示
	//GetPrivateProfileQString("Order", "TipsAskFail", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "TipsAskFail", str);
	g_Global.m_bTipsAskFail = (str == "1") ? TRUE : FALSE;
	//GetPrivateProfileQString("Order", "SoundAskFail", "", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "SoundAskFail", str);
	g_Global.m_csSoundAskFail = str;
	//// 挂单成交
	//GetPrivateProfileQString("Order", "TipsOrderMatch", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "TipsOrderMatch", str);
	g_Global.m_bTipsOrderMatch = (str == "1") ? TRUE : FALSE;

	//GetPrivateProfileQString("Order", "SoundOrderMatch", "", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "SoundOrderMatch", str);
	g_Global.m_csSoundOrderMatch = str;
	//// 撤单成功
	//GetPrivateProfileQString("Order", "TipsCancelSus", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "TipsCancelSus", str);
	g_Global.m_bTipsCancelSus = (str == "1") ? TRUE : FALSE;
	//GetPrivateProfileQString("Order", "SoundCancelSus", "", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "SoundCancelSus", str);
	g_Global.m_csSoundCancelSus = str;
	//// 撤单失败
	//GetPrivateProfileQString("Order", "TipsCancelFail", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "TipsCancelFail", str);
	g_Global.m_bTipsCancelFail = (str == "1") ? TRUE : FALSE;
	//GetPrivateProfileQString("Order", "SoundCancelFail", "", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "SoundCancelFail", str);
	g_Global.m_csSoundCancelFail = str;

	//// 是否显示客户信息
	//GetPrivateProfileQString("Display", "ShowCusInfo", "1", str.GetBuffer(MAX_PATH), MAX_PATH, sfile);
	App::GetPriProfileString(sfile, "Order", "ShowCusInfo", str);
	g_Global.m_bShowCusInfo = (str == "1") ? TRUE : FALSE;
	// 加载风向标显示品种
	//CWindVaneMgr::GetInstance().LoadShowInsID();
}

void CTraderCpMgr::SetBitSet( bitset<5> &bitvec, bool bFund /*= false*/, bool bStore /*= false*/, bool bPosi /*= false*/, bool bLong /*= false*/, bool bBuy /*= false*/ )
{
	if(bFund) bitvec.flip(0);
	if(bStore) bitvec.flip(1);
	if(bPosi) bitvec.flip(2);
	if(bLong) bitvec.flip(3);
	if(bBuy) bitvec.flip(4);
}

bool CTraderCpMgr::IsUnFindLocalOrderNo(const QString &sLocalOrderNo)
{
	for (int i = 0; i < m_arrUnFindLocalOrderNo.size(); i++)
	{
		if (m_arrUnFindLocalOrderNo[i] == sLocalOrderNo) return true;
	}

	return false;

}

double CTraderCpMgr::GetRealeaseMoney(const QString &localOrderNo, int iHand)
{
	// 释放资金
	double dFrozenTradeFee = 0.00;
	if (!localOrderNo.isEmpty())
	{
		// 获取该笔单的冻结信息
		QMap<QString, OrderFrozeInfo>::iterator it = m_QMapOrderFroze.find(localOrderNo);
		if (it != m_QMapOrderFroze.end())
		{
			// 获取解冻资金
			//if(it.value().CancelBal(iHand, dFrozenTradeFee))
			//{
			//	 删除该笔流水
			//}
		}
	}

	return dFrozenTradeFee;
}

double CTraderCpMgr::GetMatchUnReleaseMoney(const QString &localOrderNo)
{
	double dFrozenTradeFee = 0.00;
	QMap<QString, int>::iterator it = m_QMapMatchUnFindInfo.find(localOrderNo);
	if (it != m_QMapMatchUnFindInfo.end())
	{
		dFrozenTradeFee = GetRealeaseMoney(localOrderNo, it.value());
	}

	return dFrozenTradeFee;
}

// 获取一笔成交单后应该解冻的资金
double CTraderCpMgr::HandleMatchReleaseMoney(const QString &localOrderNo, int iHand)
{
	double dFrozenTradeFee = 0.00; //

								   // 根据本地报单号查找该笔报单的冻结信息
	QMap<QString, OrderFrozeInfo>::iterator it = m_QMapOrderFroze.find(localOrderNo);
	if (it != m_QMapOrderFroze.end()) // 如果存在
	{
		// 获取该笔单的冻结信息
		OrderFrozeInfo &stFrozeInfo = it.value();
		// 解冻资金
		//if(stFrozeInfo.CancelBal(iHand, dFrozenTradeFee))
		//{
		//	// 删除该笔流水
		//}
	}
	else
	{
		// 没找到则表示处理成交的解冻资金失败，记录本地报单号和手数
		QMap<QString, int>::iterator it = m_QMapMatchUnFindInfo.find(localOrderNo);
		if (it != m_QMapMatchUnFindInfo.end())
		{
			it.value() += iHand;
		}
		else
		{
			m_QMapMatchUnFindInfo[localOrderNo] = iHand;
		}
	}

	return dFrozenTradeFee;
}

// 根据报单状态ID判断该笔单是否要进行自动计算的处理
bool CTraderCpMgr::IsOrderCanBeProcess(const QString &sOrderState)
{
	// o：已报入 c：全部成交 p：部分成交  3｜部成部撤 1：正在申报
	// 去掉正在申报的状态的情况
	if (sOrderState == "o" || sOrderState == "c" || sOrderState == "p" || sOrderState == "3") // 3｜部成部撤  d｜已撤销
		return true;
	else
		return false;
}

// 根据域名（ip）获取ip（如果是ip的话，则返回一样的值）
QString CTraderCpMgr::GetIPFromDomain(const QString &sDomainIP)
{/*
 if(sDomainIP.isEmpty())
 {
 return "";
 }
 else
 {
 WSAData wsData;
 int WSA_return = WSAStartup(0x0101,&wsData);

 DWORD dwIP = 0;
 //域名解析
 HOSTENT* pHS = gethostbyname(sDomainIP.toLatin1().data());
 if(   pHS   !=   NULL)
 {
 in_addr addr;
 CopyMemory(&addr.S_un.S_addr, pHS->h_addr_list[0], pHS->h_length);
 dwIP = addr.S_un.S_addr;
 }

 BYTE   by1,   by2,   by3,   by4;
 char   szIP[30] = {0};
 by1   =   HIBYTE(HIWORD(dwIP)),   by2   =   LOBYTE(HIWORD(dwIP)),   by3   =   HIBYTE(LOWORD(dwIP)),   by4   =   LOBYTE(LOWORD(dwIP));
 sprintf(szIP,   "%d.%d.%d.%d",   by4,   by3,   by2,   by1);

 // remove by  20121219 去掉的话后面则使用socket
 // WSACleanup();

 QString sReturn = szIP;
 return sReturn;
 }
 */


	QString sReturn = sDomainIP;
	return sReturn;
}

//kenny  20180309   byte ?? hex
template<typename TInputIter>
std::string make_hex_string(TInputIter first, TInputIter last, bool use_uppercase = true, bool insert_spaces = false)
{
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    if (use_uppercase)
        ss << std::uppercase;
    while (first != last)
    {
        ss << std::setw(2) << static_cast<int>(*first++);
        if (insert_spaces && first != last)
            ss << " ";
    }
    return ss.str();
}

// 话v媽儯?ǖ儶??祪簲v?O O=獡獥?）（pszPwd．v槍僾嫭蔚??哎器?捨）（bShort． ??厰v?胿嫭螙?O（O垜/MD5簲v?睌枖v）
QString CTraderCpMgr::GetEncryptPSW(const char* pszPwd, bool bShort /*= false*/)
{
	QString sFinal;

#ifdef _WIN32
	//kenny  20171128  攙祪摽康鴤祪簲v嫞?娴�?
    sFinal = MD5::ToMD5(pszPwd, strlen(pszPwd)).c_str();// 攙v嫭?
#else
    MD5_CTX ctx;
    unsigned char outmd[16];
    //????openssl?64??????
    int i=0;
    memset(outmd,0,sizeof(outmd));
    MD5_Init(&ctx);
    MD5_Update(&ctx,pszPwd,strlen(pszPwd));
    MD5_Final(outmd,&ctx);


    string str;
    std::vector<uint8_t> byte_vector(std::begin(outmd), std::end(outmd));
    str = make_hex_string(byte_vector.begin(), byte_vector.end(), false);
    sFinal = str.c_str();
#endif


	if (bShort)
	{
		sFinal = sFinal.mid(8, 16);
	}


	return sFinal;
}

// 加载配置文件中的当日出入金流水
void CTraderCpMgr::IniTodayTransfer()
{
	char buf[500] = { 0 };
	QString sValue;

	// 读取文件中保存的客户号
	//::GetPrivateProfileQString("info", "InOutRecords", "", buf, sizeof(buf), g_Global.GetDataIniPath());
	//sValue = buf;
	//int iPos = sValue.find("∧"); // 获取时间与值的分隔符
	//if(iPos != -1)
	//{
	//	// 如果客户号相同
	//	if(sValue.substr(0, iPos) == g_Global.m_strUserID)
	//	{
	//		// 获取上一交易日+值的字符串
	//		sValue = sValue.substr(iPos+2);

	//		iPos = sValue.find("∧"); // 获取时间与值的分隔符
	//		if(iPos != -1)
	//		{
	//			// 获取数据对应的交易日
	//			QString sExchDate = sValue.mid(0, iPos);
	//			// 与当前实际的交易日对比
	//			if(sExchDate == g_Global.m_strExchDate)
	//			{
	//				// 加载当日出入金流水
	//				ArrayListMsg alm_result;
	//				alm_result.Parse(sValue.mid(iPos+2));
	//				for(size_t i = 0; i < alm_result.size(); i++)
	//				{
	//					const ArrayListMsg &aMsg = alm_result.GetValue(i); // 获取结果的一组数据
	//					if(!aMsg.GetQStringEx(0).isEmpty())
	//					{
	//						QString sMoney = aMsg.GetQStringEx(1);
	//						auto Insert_Pair = m_QMapTransfer.insert(aMsg.GetQStringEx(0), sMoney);
	//						m_stFundInfo.dAllBalance += sMoney.toFloat();
	//						m_stFundInfo.dUseFulBal += sMoney.toFloat();
	//					}
	//				}
	//			}
	//			else
	//			{
	//				// 是否要清空
	//			}
	//		}
	//	}
	//}
}

// 
QString CTraderCpMgr::FormatCurTransferStr()
{
	QString sValue;
	for (QMap<QString, QString>::iterator it = m_QMapTransfer.begin(); it != m_QMapTransfer.end(); it++)
	{
		sValue = sValue + it.key() + "｜" + it.value() + "｜" + "∧";;
	}

	return sValue;
}

void CTraderCpMgr::CustomerUsefulPosi(const QString& prodCode, int& iUsefullong, int& iUsefulshort)
{
	iUsefullong = iUsefulshort = 0;
	// 获取该品种的仓位信息
	QMap<QString, DeferPosi>::const_iterator it = g_TraderCpMgr.m_QMapDeferPosi.find(prodCode);
	if (it != g_TraderCpMgr.m_QMapDeferPosi.end())
	{
		iUsefullong = it->infoLong.iUsefulAmt;
		iUsefulshort = it->infoShort.iUsefulAmt;
	}
}

// 获取中立仓报单的价格
double CTraderCpMgr::GetMAOrderPrice(const QString &sProdCode)
{
	// 记录最后返回的价格
	double dPrice;

	// 获取行情信息
	const QUOTATION& qt = m_QMapQuotation[sProdCode.toStdString()];

	// 根据配置的价格方式取行情对应的价格
	unsigned int uiPrice = qt.m_uiAverage;

	if (uiPrice != 0)
	{		// 获取浮点类型的价格，参照服务器对其格式化
		dPrice = CHJGlobalFun::DoubleFormat(uiPrice / g_Global.m_dQTFactor);
	}
	else
	{
		dPrice = GetDeferDeliveryPrice(sProdCode);
	}

	return dPrice;
}

void CTraderCpMgr::WriteTodayTransferToFile()
{
	QString sWriteData = g_Global.m_strUserID + "∧" + g_Global.m_strExchDate + "∧" + FormatCurTransferStr();
	//::WritePrivateProfileQString("info", "InOutRecords", sWriteData,g_Global.GetDataIniPath()); 
}

// 处理出入金流水查询中一笔入账流水的信息，如果还没记录，则存入内存中的出入金流水记录并更新到本地资金信息
// 流水则内存中不存在则返回true，存在则返回false
bool CTraderCpMgr::DealOneHandTransfer(const QString &sSerialNo, QString sExchBal, bool bIn)
{
	QMap<QString, QString>::iterator it = m_QMapTransfer.find(sSerialNo);
	if (it == m_QMapTransfer.end())
	{
		// 如果是出金，则判断原来的金额字符串是否带负号，没有则加上
		if (!bIn)
		{
			int iPos = sExchBal.indexOf("-"); // 获取时间与值的分隔符
			if (iPos == -1)
			{
				sExchBal = "-" + sExchBal;
			}
		}

		// 记录到内存 
		m_QMapTransfer[sSerialNo] = sExchBal;

		m_stFundInfo.dAllBalance += sExchBal.toFloat();
		m_stFundInfo.dUseFulBal += sExchBal.toFloat();

		m_mapBdr[E_REFRESHFUND].Broadcast(WM_REFRESH_FUND, 0, 0);

		return true;
	}

	return false;
}

// 向服务器请求今天出入金流水
bool CTraderCpMgr::GetTodayTransfer()
{
	// 	// 获取请求的列ID 
	// 	ArrayListMsg almViewField;  
	// 	QVector< pair<QString,QString> > vecPara;
	// 	ArrayListMsg alm_result;
	// 
	// 	almViewField.Parse("in_account_flag∧serial_no∧access_way∧exch_bal∧"); // 是否入账∧转账流水号∧出入金∧转账金额∧
	// 	CHJGlobalFun::PairAdd(vecPara, "access_way", "");
	// 
	// 	Rsp6002 rsp6002;
	// 	HEADER_RSP headrsp;
	// 
	// 	if(CTranMessage::Handle6002(rsp6002, headrsp, "AcctFundAutoInOutFlow", almViewField, vecPara, 500 ) == 0 && rsp6002.alm_result.size() > 0)
	// 	{
	// 
	// 	}


#if (defined _VERSION_GF) || (defined _VERSION_JH) || (defined _VERSION_YC) || (defined _VERSION_PA)// 交行和金联通出入金流水查询采用一样的报文

	Rsp3101 rsp;
	if (CTranMessage::Handle3101(rsp, 5) == 0)
	{
		LoadTodayTransfer(rsp.htm_result);

		return true;
	}
	else
	{
		return false;
	}
#else
	Rsp3021 rsp; //应答报文体
				 /*if(CTranMessage::Handle3021(rsp, 1, "") == 0)
				 {
				 LoadTodayTransfer(rsp.alm_custtype_list);

				 return true;
				 }
				 else*/
	{
		return false;
	}

#endif
}

// 金联通版本
void CTraderCpMgr::LoadTodayTransfer(HashtableMsg &htm_result)
{
	int iRewriteCount = 0;

	for (size_t i = 0; i < htm_result.size(); i++)
	{
		if (htm_result.GetString(i, "in_account_flag") == "是")
		{
			bool bIn = htm_result.GetString(i, "access_way") == "存入" ? true : false;

			if (DealOneHandTransfer(htm_result.GetString(i, "serial_no").c_str(), htm_result.GetString(i, "exch_bal").c_str(), bIn))
			{
				iRewriteCount++;
			}
		}
	}

	if (iRewriteCount > 0)
	{
		WriteTodayTransferToFile();
	}
}

void CTraderCpMgr::LoadTodayTransfer(ArrayListMsg &alm)
{
	int iRewriteCount = 0;

	for (size_t i = 0; i < alm.size(); i++)
	{
		if (alm.GetStringEx(i, 12) == "是")
		{
			bool bIn = alm.GetStringEx(i, 6) == "存入" ? true : false;

			if (DealOneHandTransfer(alm.GetStringEx(i, 0).c_str(), alm.GetStringEx(i, 7).c_str(), bIn))
			{
				iRewriteCount++;
			}
		}
	}

	if (iRewriteCount > 0)
	{
		WriteTodayTransferToFile();
	}
}

void CTraderCpMgr::IniOrderSerial()
{
	QDateTime current_date_time = QDateTime::currentDateTime();
	QString current_date = current_date_time.toString("ddhhmmss");
	current_date = g_Global.m_strUserID + current_date;// 客户号加上时间标识作为客户序列号
	m_uOrderSerial = current_date.toULongLong();
}

QString CTraderCpMgr::GetOrderSerialStr()
{
	return QString::number(m_uOrderSerial);
}

void CTraderCpMgr::RefreshCusFundInfo()
{
#ifdef _NOAUTO_FUND
	Rsp1020 rsp1020;
	if (CTranMessage::Handle1020(rsp1020, '0', '1', '0', '0', '0', '1', true) == 0)
	{
		// 获取成功则将客户资金信息复制到内存变量
		m_rsp1020.CopyFundInfo(rsp1020);
		m_mapBdr[E_ONREFRESHCUSTOMDATA].Broadcast(WM_REFRESH_CUSTOM_INFO, 0, 0);
	}
#endif
}

void CTraderCpMgr::CalculatePosiMargin()
{
	m_stFundInfo.dPosiMargin = 0.00;

	for (auto it = m_QMapDeferPosi.begin(); it != m_QMapDeferPosi.end(); it++)
	{
		//auto &stDeferPosi = it.value();
		if (it->infoLong.iCurrAllAmt > 0)
		{
			m_stFundInfo.dPosiMargin += it->infoLong.dPosi_PL;
		}
		if (it->infoShort.iCurrAllAmt > 0)
		{
			m_stFundInfo.dPosiMargin += it->infoShort.dPosi_PL;
		}
	}

	// added by Jerry Lee, 2013-3-24, 确保和服务器的浮动盈亏一致
	/*
	Rsp1020 rsp1020;
	if(CTranMessage::Handle1020(rsp1020, '1', '1', '0', '0', '0', '0', true) == 0)
	{
	m_stFundInfo.dPosiMargin = atof(rsp1020.r_surplus);
	}
	*/
	//
}

QString CTraderCpMgr::GetInsStateNameFromID(const QString &sInsID)
{
	for (size_t i = 0; i < m_vInstState2.size(); i++)
	{
		if (m_vInstState2.at(i).code_id == sInsID)
		{
			return m_vInstState2.at(i).code_desc;
		}
	}

	return "";
}

int CTraderCpMgr::CommitOrder(const QString &sProdCode, double dPrice, int iAmount, const QString &sExchID)
{
	Rsp4001 rsp;
	int nRet = CTranMessage::Handle4001(rsp,sProdCode, dPrice, iAmount, sExchID,0);

	  //增加对报单错误码的判断
	  if (rsp.rsp_code.find("HJ4034") != string::npos)  
	  {
	      nRet = 4034;
	  }

	return nRet;
	return  0;
}

/*
modify by  20130806 添加了bAuto的字段，如果bAuto为true，则采用快金自定义的提示窗口，如果不是则直接弹出提示窗口
*/
int CTraderCpMgr::CommitOrder(const QString &sProdCode, const QString &csPrice, const QString &csAmount, const QString &sExchID, int orderType,bool bAuto/* = true*/)
{
	// 发送报单
	Rsp4001 rsp;
	int iRspID = CTranMessage::Handle4001(rsp, sProdCode, csPrice.toFloat(), csAmount.toInt(), sExchID, orderType, false);

	   // added by Jerry Lee, 2013-4-1, 增加对报单错误码的判断
	   string strCode = rsp.rsp_code;
	   if (rsp.rsp_code.find("HJ4034") != string::npos)  
	   {
	       iRspID = 4034;

	       return iRspID;
	   }

	// 声音提示处理，将声音提示放在弹出提示的前面，否则弹出对话框会有空白的瞬间
	//if((iRspID == 0 && g_Global.m_csSoundAskSus != "") || (iRspID != 0 && g_Global.m_csSoundAskFail != ""))
	//{
	//	QString *pCS = (iRspID == 0) ? &g_Global.m_csSoundAskSus : &g_Global.m_csSoundAskFail;
	//	::PostMessage(m_hMain, WM_PLAY_TIPS_MUSIC, (WPARAM)pCS, 0);
	//	//AfxGetMainWnd()->PostMessage(WM_PLAY_TIPS_MUSIC, (WPARAM)pCS);
	//}

	// 如果需要提示，则弹出提示
	if((iRspID == 0 && g_Global.m_bTipsAskSus) || (iRspID != 0 && g_Global.m_bTipsAskFail))
	{
		if( bAuto )
		{
			TipsOrderInfo TipsInfo;
			// 获取提示类型
			TipsType eType = (iRspID == 0) ? E_Tips2_Asksus : E_Tips2_AskFail;
			// 获取提示语
			QString sMsg = (iRspID == -1) ? CONSTANT_CONNECT_SERVER_FAIL_TIPS : CHJGlobalFun::str2qstr(rsp.rsp_msg);
			// 初始化化提示结构
			//TipsInfo.IniData()
			TipsInfo.IniData(eType, CHJGlobalFun::qstr2str( sProdCode), CHJGlobalFun::qstr2str(sExchID), csPrice, csAmount, sMsg, "");
			// 将提示内容加载进内存
			int nAddPos = g_TipsManager.Add(TipsInfo);


			QParamEvent *msg = new QParamEvent(WM_SHOW_ORDER_TIPS);
			//msg->setWParam(&nAddPos);
			msg->setInt(nAddPos);
			QApplication::postEvent(m_hMain, msg);

		}
		else
		{
			// 获取提示语
			QString sMsg = (iRspID == -1) ? CONSTANT_CONNECT_SERVER_FAIL_TIPS : CHJGlobalFun::str2qstr(rsp.rsp_msg);

			//kenny  2018-1-22  工作线程中弹出对话框，会中断工作线程，报错
			//QMessageBox::information(nullptr, "提示", sMsg , QMessageBox::Ok);
		}
	}

	return iRspID;

	return  0;
}

//收到成交回报消息后，进行界面提示
void CTraderCpMgr::ShowMatchTips(const QString &sProdCode, const double &dPrice, const int &iAmount, const QString &sExchID, const QString &sOrderNo)
{
	TipsOrderInfo TipsInfo;
	QString csPrice, csHand;
	csPrice = QString::number(dPrice,'f',2);//Format("%.2f", dPrice);
	csHand = QString::number(iAmount);
	//csHand.Format("%d", iAmount);
	QString  tips = "挂单成交";
	TipsInfo.IniData(E_Tips2_OrderMatch, CHJGlobalFun::qstr2str(sProdCode), CHJGlobalFun::qstr2str(sExchID), csPrice, csHand,tips, sOrderNo);
	int nAddPos = g_TipsManager.Add(TipsInfo);

	QParamEvent * msg = new QParamEvent(WM_SHOW_ORDER_RESULT_TIPS);
	msg->setInt(nAddPos);

	QApplication::postEvent(m_hMain, msg);

}


int CTraderCpMgr::CancelOrder(QString &csOrderNo)
{
	Rsp4061 rsp; //应答报文体

	int iRspID = CTranMessage::Handle4061( rsp,csOrderNo );

	// 声音提示处理，将声音提示放在弹出提示的前面，否则弹出对话框会有空白的瞬间
	/*if((iRspID == 0 && g_Global.m_csSoundCancelSus != "") || (iRspID != 0 && g_Global.m_csSoundCancelFail != ""))
	{
		//QString pCS = (iRspID == 0) ? g_Global.m_csSoundCancelSus : g_Global.m_csSoundCancelFail;
		
		TipsOrderInfo TipsInfo;
		// 获取提示类型
		TipsType eType = (iRspID == 0) ? E_Tips2_CancelSus : E_Tips2_CancelFail;

		TipsInfo.IniData(eType, csOrderNo);
		// 将提示内容加载进内存
		int nAddPos = g_TipsManager.Add(TipsInfo);


		QParamEvent *msg = new QParamEvent(WM_SHOW_ORDER_CANCEL_TIPS);
		msg->setInt(nAddPos);
		QApplication::postEvent(m_hMain, msg);

	}*/

	if((iRspID == 0 && g_Global.m_bTipsCancelSus) || (iRspID != 0 && g_Global.m_bTipsCancelFail))
	{
		TipsOrderInfo TipsInfo;
		TipsType eType = (iRspID == 0) ? E_Tips2_CancelSus : E_Tips2_CancelFail;
		QString sMsg = (iRspID == -2 || iRspID == -1) ? CONSTANT_CONNECT_SERVER_FAIL_TIPS : CHJGlobalFun::str2qstr(rsp.rsp_msg);
		TipsInfo.IniData(eType, csOrderNo);
		int nAddPos = g_TipsManager.Add(TipsInfo);


		QParamEvent *msg = new QParamEvent(WM_SHOW_ORDER_CANCEL_TIPS);
		msg->setInt(nAddPos);
		QApplication::postEvent(m_hMain,msg);


	}

	return iRspID;

}

int CTraderCpMgr::GetDlgHandleFromID(const EDLGID &eDlgID)
{
	QMap<EDLGID, int>::const_iterator it = m_QMapDlgHandle.find(eDlgID);
	if (it != m_QMapDlgHandle.end())
		return it.value();
	else
		return NULL;
}

void CTraderCpMgr::AddDlgHandle(const EDLGID &eDlgID, const int &hDlg)
{
	m_QMapDlgHandle[eDlgID] = hDlg;
}

/*
判断
*/
bool CTraderCpMgr::bIsOrderUnMatch(const QString &sLocalOrderNo, bool bErase)
{
	for (list<QString>::iterator it = m_QMapUnMatchOrder.begin(); it != m_QMapUnMatchOrder.end(); it++)
	{
		if ((*it == sLocalOrderNo))
		{
			if (bErase)
			{
				m_QMapUnMatchOrder.erase(it);
			}
			return true;
		}
	}

	return false;
}



bool CTraderCpMgr::InitEx()
{
	// 限制只初始化一次
	static bool bInitSucs = false;

	if (!bInitSucs)
	{
		QString sIP;
		QString sPort;
		// 获得认证的ip和端口
		if (GetAuthenticateInfo(sIP, sPort))
		{
			Init();

			if (CCommHandler::Instance()->OpenInterfaceB1C(g_TraderCpMgr.GetIPFromDomain(sIP), sPort) == 0)
			{
				// 初始化路由表，将m_tblIfRouterCfg数组中的配置加载到m_tblIfRouter（QMap类型），对每个报文加载对应的接口类
				InitRouterTbl();

				// 请求报文头设置
				g_HeaderReq.SetUserID("");
				g_HeaderReq.SetSeqNo(g_SeqNo++);

				g_HeaderReq.SetTermType("03"); //表示登陆渠道（03为交易终端）
				g_HeaderReq.SetUserType("2");  //表示用户类型（2为客户）
				g_HeaderReq.SetMsgFlag("1");   //请求报文标识（未知）
				g_HeaderReq.SetMsgType("1");   //交易报文类型（1为交易）

				bInitSucs = true;
			}
		}
	}

	return bInitSucs;
}

// 获取认证需要的ip和端口
bool CTraderCpMgr::GetAuthenticateInfo(QString &sIP, QString &sPort)
{
	QString buf;
	QString strConfigPath( g_Global.GetSystemIniPath() );

	// 获取登录IP
	App::GetPriProfileString(strConfigPath, "Info", "LOGIN.ip", sIP);

	// 获取登录端口
	App::GetPriProfileString(strConfigPath, "Info", "LOGIN.port", sPort);

	if( sIP.isEmpty() || sPort.isEmpty() )
		return false;
	else
		return true;
}

void CTraderCpMgr::ClearHistoryInfo(void)
{
	QString csPath(g_Global.GetDataIniPath());

	App::WritePriProfileString(csPath, "Info", "FundInfo", "");
	App::WritePriProfileString(csPath, "Info", "DeferPosiInfo", "");
	App::WritePriProfileString(csPath, "Info", "StoreInfo", "");
	App::WritePriProfileString(csPath, "Info", "ForwardPosiInfo", "");
}

// 判断一个用户在当前交易日是否有有效的交易
bool CTraderCpMgr::HasValidTrade(void)
{
	// 是否有成交
	if (m_QMapSpotMatch.size() > 0 || m_QMapForwardMatch.size() > 0 || m_QMapDeferMatch.size() > 0 || m_QMapDDAMatch.size() > 0)
		return true;

	//// 是否有有效的报单
	if (HasValidOrder(m_QMapSpotOrder) || HasValidOrder(m_QMapForwardOrder) || HasValidOrder(m_QMapDeferOrder)
		|| HasValidOrder(m_QMapDDAOrder) || HasValidOrder(m_QMapMiddleAppOrder))
		return true;

	// 是否有出入金记录
	if (m_QMapTransfer.size() > 0)
		return true;

	return false;
}


void CTraderCpMgr::ClearCusInfo(void)
{
	// 清除本地文件历史信息
	ClearHistoryInfo();

	// 清空客户内存基本信息
	m_QMapDeferPosi.clear();
	m_QMapStoreInfo.clear();
	m_stFundInfo.ClearData();

	// 考虑是否清除成交流水信息
	int wval = 1;
	int lval = 0;
	// 发送广播
	m_mapBdr[E_ONSYSINIT].Broadcast( WM_ON_SYS_INIT, &wval, &lval, TRUE );
}


void CTraderCpMgr::RecordVersionInfo(QString strVersion)
{
	// 更新配置文件的版本号
	if (strVersion == "")
	{
		strVersion = "1.0.0";
	}
	//::WritePrivateProfileQString("Info", "HJ.ver_num", strVersion, g_Global.GetSystemIniPath());

	// 下次不需要下载参数
	//::WritePrivateProfileQString("Info", "HJ.is_down_para", "0", g_Global.GetSystemIniPath());
}

//#include "IniFilesManager.h"
void CTraderCpMgr::IniQuotationInsID()
{
	QIniFilesManager mgr;
	std::vector<QString> vecUserNames;
	mgr.GetIniValueToVector(g_Global.GetListIniPath("InsID"), "Info", "AllColumnIDs", QUERY_INI_SPLITER, "", vecUserNames );
	for (size_t i = 0; i < vecUserNames.size(); i++)
	{
		QUOTATION qt;
		qt.instID = vecUserNames[i].toStdString();
		m_QMapQuotation[qt.instID] = qt;
	}
}


bool CTraderCpMgr::OpenedBillExists()
{
	for (auto it = m_QMapDeferOrder.begin(); it != m_QMapDeferOrder.end(); it++)
	{
		DeferOrder& dOrder = it.value();

		QString nId = CHJCommon::GetDeferId(dOrder.offSetFlag.c_str(), dOrder.buyOrSell.c_str());

		if (((CONSTANT_EXCH_CODE_DEFER_OPEN_LONG == nId) || (CONSTANT_EXCH_CODE_DEFER_OPEN_SHORT == nId)))
		{
			QString strStatus = dOrder.status.c_str();
			if (strStatus != "c"  && strStatus != "d")
			{
				return true;
			}
		}

	}

	return false;
}

#define UPDATE_INI_NAME ("Update.ini")
// 唤起升级程序
bool CTraderCpMgr::CallUpdateExe(void) const
{
	// 判断升级程序是否存在，如果存在，则唤起。
	QString buf;

	App::GetPriProfileString(g_Global.GetSystemPath() + UPDATE_INI_NAME, "Update", "Update1", buf);
	if (buf == 0)
	{
		return false;
	}
	else
	{
		QString csFullName = buf;

		QString csExeName;
		QString csPara;
		int nPos = csFullName.indexOf(" ");
		if (nPos != -1)
		{
			csExeName = csFullName.left(nPos);
			csPara = csFullName.mid(nPos + 1);
		}
		else
		{
			csExeName = csFullName;
		}

		//ShellExecute( NULL, "open", g_Global.GetSystemPath()+csExeName, csPara, NULL, NULL );

		return true;

		/*QString csUpdateExeName = buf;
		BOOL bRet = CHJGlobalFun::RunFileExe( g_Global.GetSystemPath()+csUpdateExeName);
		if( bRet )
		LOG("启动升级程序成功");
		else
		LOG("启动升级程序失败");
		return bRet;*/
	}
}

#if (defined _VERSION_JSZX) || (defined _VERSION_ZHLHY)
void CTraderCpMgr::SetTransferMode(const ArrayListMsg &alm_menu_info)
{
	for (size_t i = 0; i < alm_menu_info.size(); i++)
	{
		const ArrayListMsg &aMsg = alm_menu_info.GetValue(i);
		QString sMenuID(aMsg.GetQStringEx(0));
		if (sMenuID == MenuIdByAuto)
		{
			g_Global.m_bAutoTransfer = true;
			return;
		}
		else if (sMenuID == MenuIdByHand)
		{
			g_Global.m_bAutoTransfer = false;
			return;
		}
	}
}
#endif


void CTraderCpMgr::SwitchServer()
{
	CCommHandler::Instance()->SwitchToNextServer();
}

bool CTraderCpMgr::AddPrice(QVector<unsigned int> &vPrice, unsigned int dPrice)
{
	if (dPrice > 0)
	{
		vPrice.push_back(dPrice);
		return true;
	}
	return false;
}

void CTraderCpMgr::SetServerList(HashtableMsg &htm_server_list)
{
	QString csPath = g_Global.GetSystemIniPath();
	QString sMode;

	App::GetPriProfileString(csPath, "info", "DirectMode", sMode);
	if (sMode == "1")
	{
		QString sValue;

		App::GetPriProfileString(csPath, "info", "DirectModeInfo", sValue);

		HashtableMsg htm(sValue.toStdString());
		for (size_t mm = 0; mm < htm.size(); mm++)
		{
			map<string, string> QMapValue = htm.GetMap(mm);
			CCommHandler::Instance()->InsertServer(QMapValue);
		}
	}
	else
	{
		// 保存服务器列表
		for (size_t mm = 0; mm < htm_server_list.size(); mm++)
		{
			map<string, string> QMapValue = htm_server_list.GetMap(mm);

			// 如果为域名，则修改为ip
			QString sID[5] = { "broadcast_ip", "query_ip", "risk_broadcast_ip", "trans_ip", "risk_trans_ip" };
			for (int i = 0; i < 5; i++)
			{
				map<string, string>::iterator it = QMapValue.find(sID[i].toStdString());
				if (it != QMapValue.end())
				{
					// mod by Jerry Lee, 2013-3-27, 不需要转成ip
					if (g_Global.m_nProxyType == 0)
					{
						it->second = GetIPFromDomain(it->second.c_str()).toStdString();
					}
				}
			}

			CCommHandler::Instance()->InsertServer(QMapValue);
		}
	}
}

void CTraderCpMgr::resetAccount(QString && str)
{
	UserLogout();

	// 保存广播报文中成交流水数据
	m_QMapSpotMatch.clear();
	m_QMapForwardMatch.clear();
	m_QMapDeferMatch.clear();
	m_QMapDDAMatch.clear();
	// 保存广播报文中报单流水数据
	m_QMapSpotOrder.clear();
	m_QMapForwardOrder.clear();
	m_QMapDeferOrder.clear();
	m_QMapDDAOrder.clear();
	m_QMapMiddleAppOrder.clear();
	// 撤单流水
	m_QMapDeferOrderCancel.clear(); // 延期的撤单单独处理
	m_QMapOrderCancel.clear();      // 不包括延期的撤单
									// 记录客户的持仓信息
	m_QMapDeferPosi.clear();
	// 库存信息
	m_QMapStoreInfo.clear();
	// 资金信息
	m_stFundInfo.ClearData();
	// 当日出入金信息
	m_QMapTransfer.clear();
	m_QMapOrderFroze.clear();
	m_arrUnFindLocalOrderNo.clear();

	m_QMapMatchUnFindInfo.clear();

	int nRet = 0;
    //根据别名，读取账户信息，进行登陆
	for (int i = 0; i < App::accMgr.size(); i++)
	{
		if (App::accMgr[i].alias == str)
		{
			g_Global.m_strUserID = App::accMgr[i].user; // 用户ID    
			g_Global.m_strPwdMD5 = g_TraderCpMgr.GetEncryptPSW(App::accMgr[i].psw.toStdString().c_str());

			g_Global.m_bShowLastLoginInfo = false;
			nRet = g_TraderCpMgr.UserLogin();
			break;
		}

	}

	if (nRet < 0)
		return;

	
}