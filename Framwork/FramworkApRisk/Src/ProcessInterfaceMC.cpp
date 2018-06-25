#include "ProcessInterfaceMC.h"
#include "IpcPacket.h"
#include "Logger.h"
#include "ApConstant.h"
using namespace ApConst;

CProcessInterfaceMC::GessPktInfo CProcessInterfaceMC::m_GessPktInfo = 
{
	8,//DEFAULT_IDLE_TIMEOUT,
	4,//DEFAULT_HELLO_RESEND_INTERVAL,
	8,//DEFAULT_HELLO_RESEND_COUNT,
	false
};
bool CProcessInterfaceMC::m_blGessPktInfoInited = false;

CProcessInterfaceMC::CProcessInterfaceMC()
:m_pCfg(0)
,m_uiCountNoAlive(0)
,m_uiCountSended(0)
{
}

CProcessInterfaceMC::~CProcessInterfaceMC(void)
{
}

int CProcessInterfaceMC::Init(CConfig * pCfg)
{
	CTcpAppProcessClient::Init(pCfg);
	if (!m_blGessPktInfoInited)
	{
		m_blGessPktInfoInited = true;
		

		
		string sLoginFlag;
		if (0 == pCfg->GetProperty("login",sLoginFlag))
		{
			if (1 == FromString<int>(sLoginFlag))
				m_GessPktInfo.blNeedLogin = true;
		}

		string sVal;
		if (0 == pCfg->GetProperty("max_idle",sVal))
		{
			m_GessPktInfo.ulIdleInterval = strutils::FromString<unsigned long>(sVal);
			if (m_GessPktInfo.ulIdleInterval > 300 || m_GessPktInfo.ulIdleInterval < 2)
				m_GessPktInfo.ulIdleInterval = DEFAULT_IDLE_TIMEOUT;
		}

		if (0 == pCfg->GetProperty("resend_interval",sVal))
		{
			m_GessPktInfo.ulIntervalReSend = strutils::FromString<unsigned long>(sVal);
			if (m_GessPktInfo.ulIntervalReSend > 60 || m_GessPktInfo.ulIntervalReSend < 2)
				m_GessPktInfo.ulIntervalReSend = DEFAULT_HELLO_RESEND_INTERVAL;
		}

		if (0 == pCfg->GetProperty("resend_count",sVal))
		{
			m_GessPktInfo.ulHelloReSend = strutils::FromString<unsigned long>(sVal);
			if (m_GessPktInfo.ulHelloReSend > 30 || m_GessPktInfo.ulHelloReSend < 1)
				m_GessPktInfo.ulHelloReSend = DEFAULT_HELLO_RESEND_COUNT;
		}
		CRLog(E_DEBUG, "MC:max_idle:%u, resend_interval:%u, resend_count:%u",m_GessPktInfo.ulIdleInterval, m_GessPktInfo.ulIntervalReSend, m_GessPktInfo.ulHelloReSend);

		CConfig* pCfgGlobal = pCfg->GetCfgGlobal();
		pCfgGlobal->GetProperty("node_type",m_GessPktInfo.node_type);
		pCfgGlobal->GetProperty("node_id",m_GessPktInfo.node_id);
		pCfgGlobal->GetProperty("host_id",m_GessPktInfo.node_name);
		pCfgGlobal->GetProperty("host_id",m_GessPktInfo.host_id);	
	}

	m_pCfg = pCfg;	
	m_uiCountNoAlive = 0;
	m_uiCountSended = 0;
	return 0;
}

/******************************************************************************
��������:��ȡЭ�鶨��ı��ĸ�ʽ��Ϣ
���ú���:�������ص�
�������:PacketInfo & stInfo ���ĸ�ʽ��Ϣ
        eLengthType �����ֶ����� ����/10�����ַ���/16�����ַ���
		nLengthBytes ��Ϣ�������ֶεĳ���,Ϊ����ʱֻ֧�� 1 2 4�ֽ�
		nLengthPos �����ֶ���ʼλ��
		blLenIncludeHeader �����Ƿ񺬱���ͷ
		nFixHeadLen  �������ֶ�ָʾ�ĳ�����,���Ĺ̶����ֳ���
������  :��ΰ
��������:2008.07.22
�޸ļ�¼:
******************************************************************************/
void CProcessInterfaceMC::GetPacketInfo(PacketInfo & stInfo)
{
	stInfo.eLengthType = ltCharactersDec;
	stInfo.nLengthPos = 0;
	stInfo.nLengthBytes = IPC_LENGTH_BYTES;
	stInfo.blLenIncludeHeader = false;
	stInfo.nFixHeadLen = stInfo.nLengthPos + stInfo.nLengthBytes;
}

/******************************************************************************
��������:Э�鱨�Ĵ�������Ҫ����,���ݱ��������ֽ���ֱ�Ӵ�����ת��
���ú���:�������ص�
��������:OnRecvPacket,�ϴ�����
�������:char * pData���ջ���, size_t nSize�����С
����ֵ  :int �������⴦��
������  :��ΰ
��������:2008.07.22
�޸ļ�¼:
******************************************************************************/
int CProcessInterfaceMC::OnPacket(char * pData, size_t nSize)
{
	CIpcPacket GessPacket;
	GessPacket.Decode(pData, nSize);

	bool blFlag = false;
	m_csMC.Lock();
	if (m_uiCountSended > 1)
		blFlag = true;
	m_uiCountNoAlive = 0;
	m_csMC.Unlock();

	std::string sCmdID = GessPacket.GetCmdID();
	if (sCmdID == "Hello")
	{		
		OnHello(GessPacket);
		return 0;
	}
	else if (sCmdID == "HelloRsp")
	{
		if (blFlag)
			CRLog(E_PROINFO,"MC�յ�����Ӧ��:%s",GessPacket.Print().c_str());
		return 0;
	}

	string sLocalIp,sPeerIp;
	int nLocalPort,nPeerPort;
	GetPeerInfo(sLocalIp,nLocalPort,sPeerIp,nPeerPort);
	CRLog(E_PROINFO,"MC [%s:%d] %s",sPeerIp.c_str(),nPeerPort,GessPacket.Print().c_str());
//	CRLog(E_PROINFO,"MC Recv:%s",GessPacket.Print().c_str());
	OnRecvPacket(GessPacket);
	return 0;
}

int CProcessInterfaceMC::OnConnect()
{
	CProcessInterfaceClnNm::OnConnect();

	string sLocalIp,sPeerIp;
	int nLocalPort,nPeerPort;
	GetPeerInfo(sLocalIp,nLocalPort,sPeerIp,nPeerPort);
	CRLog(E_PROINFO,"MC OnConnect and CreateTimer socket (%d) Peer(%s:%d),Me(%s:%d)",SocketID(),sPeerIp.c_str(),nPeerPort,sLocalIp.c_str(),nLocalPort);
	CreateTimer(m_GessPktInfo.ulIdleInterval);
	return 0;
}

/******************************************************************************
��������:socket�����жϺ���֪ͨ,�ɸ���Э��Ҫ����д���
���ú���:ͨѶ����������ص�
����ֵ  :int �������⴦��
������  :��ΰ
��������:2008.07.22
�޸ļ�¼:
******************************************************************************/
void CProcessInterfaceMC::OnClose()
{
	string sLocalIp,sPeerIp;
	int nLocalPort,nPeerPort;
	GetPeerInfo(sLocalIp,nLocalPort,sPeerIp,nPeerPort);
	CRLog(E_PROINFO,"MC OnClose and DestroyTimer socket (%d) Peer(%s:%d),Me(%s:%d)",SocketID(),sPeerIp.c_str(),nPeerPort,sLocalIp.c_str(),nLocalPort);
	DestroyTimer();
	CProcessInterfaceClnNm::OnClose();
}

//�����ṩ����Ҫ�ӿں���,��ͨѶ���������ͱ���ʱ����
int CProcessInterfaceMC::SendPacket(CPacket & SendGessPacket)
{
	try
	{
		CIpcPacket & pkt = dynamic_cast<CIpcPacket&>(SendGessPacket);
		CRLog(E_PROINFO,"MC Send:%s",pkt.Print().c_str());
		return CAppProcess::SendPacket(SendGessPacket);
	}
	catch(std::bad_cast)
	{
		CRLog(E_ERROR,"%s","packet error!");
		return -1;
	}
	catch(std::exception e)
	{
		CRLog(E_ERROR,"exception:%s!",e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_ERROR,"%s","Unknown exception!");
		return -1;
	}
}

//������ʱ����ʱ
int CProcessInterfaceMC::HandleTimeout(unsigned long& ulTmSpan)
{
	int nRtn = 0;
	m_csMC.Lock();
	if (m_uiCountNoAlive >= 1)
	{//������·������ʱ��δ�յ����ģ���������
		if (m_uiCountSended >= m_GessPktInfo.ulHelloReSend)
		{//�ط��������������涨������׼���ر�
			nRtn = -1;
			CRLog(E_PROINFO,"MC%s(%u-%u)","������ʱ!",m_uiCountNoAlive,m_uiCountSended);
			ReqClose();
		}
		else
		{//���ö�ʱ�����
			ulTmSpan = m_GessPktInfo.ulIntervalReSend;
			m_uiCountSended++;

			if (m_uiCountSended > 1)
				CRLog(E_PROINFO,"MC��������,��%d��!",m_uiCountSended);
			
			if (0 > SendHello())
				nRtn = -1;
		}
	}
	else
	{
		m_uiCountSended = 0;
		ulTmSpan = m_GessPktInfo.ulIdleInterval;
	}
	m_uiCountNoAlive++;
	m_csMC.Unlock();
	return nRtn;
}

//��������
int CProcessInterfaceMC::SendHello()
{
	CIpcPacket pkt("Hello");
	return CAppProcess::SendPacket(pkt);
}

//��������Ӧ��
int CProcessInterfaceMC::OnHello(CIpcPacket& pktHello)
{
	CIpcPacket pkt("HelloRsp");
	return CAppProcess::SendPacket(pkt);
}

int CProcessInterfaceMC::GetNmKey(string& sKey)
{
	sKey = "M�ӿڿͻ�������";
	return 0;
}