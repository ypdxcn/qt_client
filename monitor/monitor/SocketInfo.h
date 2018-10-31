// SocketInfo.h: interface for the CSocketInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKETINFO_H__836AC69E_8563_408B_BFBD_E4195C73718A__INCLUDED_)
#define AFX_SOCKETINFO_H__836AC69E_8563_408B_BFBD_E4195C73718A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "public.h"
#include "BF_SocketTcp.h"
#include "BF_Mutex.h"
#include "BF_SimpleLog.h"
#include "Resource.h"
#include "MoniHead.h"


#ifdef _WINDOWS
#ifdef _DEBUG
#pragma comment(lib, "bf_dbpubd.lib") 
#pragma message("Automatically linking with   bf_dbpubd.lib ")
#else
#pragma comment(lib, "bf_dbpub.lib") 
#pragma message("Automatically linking with   bf_dbpub.lib ")
#endif

#endif




class CSocketInfo  
{
public:
	
	CSocketInfo();
	virtual ~CSocketInfo();


	// ������: Send
	// ���  : ������ 2013-12-10 12:24:11
	// ����  : int �������ݳ���  >0ʵ�ʷ��ĳ��� =0�Զ˻������� <0���ͳ���
	// ����  : const char *data ����
	// ����  : int len ���ݳ���
	// ����  : ��������
	int Send(const char *data,int len);

	// ������: NeedSend
	// ���  : ������ 2013-12-10 12:25:42
	// ����  : bool 
	// ����  : �Ƿ���Ҫ����
	bool NeedSend();
	

	// ������: SetAccept
	// ���  : ������ 2013-12-10 12:25:56
	// ����  : void 
	// ����  : SOCKET_HANDLE sock
	// ����  : const char *address
	// ����  : ����Ϊ������
	void SetAccept(SOCKET_HANDLE sock,const char *address);
	

	// ������: NeedRead
	// ���  : ������ 2010-7-22 9:28:09
	// ����  : bool 
	// ����  : �����Ƿ���Ҫ��
	bool NeedRead();
	
	// ������: NeedConnect
	// ���  : ������ 2010-7-22 9:24:20
	// ����  : bool 
	// ����  : �Ƿ���Ҫ�����������ӣ�ֻ����������ΪSOCK_TODREBʱ����Ҫ����
	bool NeedConnect();


	/// ������: OnRecv
	/// ���  : ������ 2009-7-3 15:35:23
	/// ����  : int ������С��0���ʾ���ӹرգ�С��ͨѶͷ��Ҫ�����գ�����ͨѶͷ����ȡ���ݽ��д�����
	/// ����  : ��������
	int OnRecv();
	


	/// ������: OnShutdown
	/// ���  : ������ 2009-7-3 10:17:25
	/// ����  : void 
	/// ����  : ǿ�йر����ӣ��ø������Ϊ��ʼֵ
	void OnShutdown();
	
	/// ������: OnClose
	/// ���  : ������ 2009-7-3 10:16:06
	/// ����  : void 
	/// ����  : const char *msg
	/// ����  : �ر����ӣ��������������ø������
	void OnClose(const char *msg,const char *filename,int fileline);
	
	/// ������: IsCanSelect
	/// ���  : ������ 2009-7-3 10:14:49
	/// ����  : bool 
	/// ����  : �������Ƿ����select����
	bool IsCanSelect();

	void Init(CResource *res);

	/// ������: Listen
	/// ���  : ������ 2009-7-3 12:48:10
	/// ����  : bool 
	/// ����  : bool blockflag=false  ������ʽ trueΪ����
	/// ����  : int queuenum=5
	/// ����  : ��ʼ�����������ڲ�����
	bool Listen(bool blockflag=false,int queuenum=5);
	
	/// ������: IsFree
	/// ���  : ������ 2009-7-3 10:18:17
	/// ����  : bool 
	/// ����  : acceptʱȡ�������Ƿ����accept
	bool IsFree();


	std::string    m_sDrebIp; //dreb ip    ת��IP
	int            m_nDrebPort; //dreb port   ת���˿�
	int            m_nDrebId;//�����ڵ��
	int            m_nDrebPrivateId;//˽�нڵ��

	char           m_sAddress[33]; //�ͻ���IP,���ӹ�����IP
	SOCKET_HANDLE  m_sock;                   //socket id
	int            m_nType;                  //SOCKET����
	int            m_nPort;                  //�����˿�
	bool           m_bChecked;
	bool           m_bNeedread;              //�Ƿ���Ҫ������
	bool           m_bNeedConnect;//�Ƿ���Ҫ����
    char           m_cCreateDate[9];         //���ӽ�������
	int            m_nConntime;              //���ӽ���ʱ��
	int            m_nReadtime;              //�ϴζ�����ʱ��
	int            m_nSendtime;              //�ϴη�������ʱ�� 
	char           m_cCloseDate[9];         //���ӹر�����
	int            m_nCloseTime;             //�ر�ʱ��
	int            m_nPingTime;              //onpingʱ��
	int            m_nPid;                   //����������bpu�Ľ���ID
	char m_sRcvBuffer[MAXCOMMBUFLEN];            //�������ݻ���   
	unsigned int  m_nRcvBufLen;                //���յ����ݳ���  �ɶ�ν���һ�����ݰ�
	char m_sSendBuffer[MAXCOMMBUFLEN];            //�������ݻ���   
	unsigned int  m_nSendBufLen;                //���͵����ݳ���  �ɶ�ν���һ�����ݰ�
	
	int  m_index;

	bool m_bIsAffirm;//���������Ƿ񷢹�������
	std::string   m_sNodeId;   //���õĽ���ID
	
protected:

	void ResetInfo();

	char   m_errMsg[256];
	CBF_SocketTcp m_tcpSocket;
	
	CBF_SimpleLog m_log;	
	CBF_Date_Time m_datetime;
	
	CResource  *m_gRes;
	CBF_Mutex  m_mutex;
	int m_sendNum;
	
		

};

#endif // !defined(AFX_SOCKETINFO_H__836AC69E_8563_408B_BFBD_E4195C73718A__INCLUDED_)