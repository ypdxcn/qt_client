//协议流程 适用于如下接口的服务端流程:
//风控服务器与风控客户端之间的F2接口
#ifndef _PROCESS_INTERFACE_F2S_H
#define _PROCESS_INTERFACE_F2S_H

#include "CommApRisk.h"
// #include "ProtocolProcess.h"
#include "BroadcastPacket.h"
#include "ProcessInterfaceNm.h"
using namespace std;

class COMMAPRISK_CLASS CBroadcastPacket;
class COMMAPRISK_CLASS CProcessInterfaceF2S : public CProcessInterfaceSvrNm
{
public:
	CProcessInterfaceF2S(void);
	~CProcessInterfaceF2S(void);

	typedef struct tagGessPktInfo
	{
		unsigned long ulIdleInterval;		//空闲时正常发送Hello间隔时间，也是链路上的最长净空时间
		unsigned long ulIntervalReSend;		//无应答后重检查间隔
		unsigned long ulHelloReSend;		//无应答而重发心跳的次数
		bool blNeedLogin;					//是否需要登录

		std::string	node_type;
		std::string	node_id;
		std::string	node_name;
		std::string	host_id;
		std::string	sUserName;				//做客户时向服务端登录用户名
		std::string	sPassword;				//做客户时向服务端登录密码
	} GessPktInfo,*PGessPktInfo;

	int Init(CConfig * pCfg);
	int SendPacket(CPacket & packet);
//	int OnSend(int nSize);

	virtual int OnAccept();		// 作服务端接收到连接后回调
	virtual void OnClose();		// 断开连接时调用
	int GetNmKey(string& sKey);
protected:
	//父类定义的回调函数实现
	virtual int OnPacket(char * pData, size_t nSize);
	virtual void GetPacketInfo(PacketInfo & stInfo);//报文格式信息
	int HandleTimeout(unsigned long& ulTmSpan);
	
	//virtual bool IsLogin() {return m_blIsLogin;} //登录状态

	//协议定义的交互报文
    //virtual int OnLogin(CBroadcastPacket & GessPacket);
private:
	int SendHello();
	int OnHello(CBroadcastPacket & GessPacket);

	static GessPktInfo m_GessPktInfo;
	static bool	m_blGessPktInfoInited;
	//bool		m_blIsLogin;			//登录状态
	
	CConfig *	m_pCfg;
	//std::deque<CBroadcastPacket> m_qSend;
	CGessMutex m_csF2S;

	unsigned long m_uiCountNoAlive;			//超时无来报文次数
	unsigned long m_uiCountSended;			//无应答后已发送心跳次数
};
#endif