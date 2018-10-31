// DrebMsgThread.h: interface for the CDrebMsgThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DREBMSGTHREAD_H__52F83741_0772_4053_96E3_452305F6E404__INCLUDED_)
#define AFX_DREBMSGTHREAD_H__52F83741_0772_4053_96E3_452305F6E404__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DrebMsgProcBase.h"
#include "Xdp.h"
#include "BF_Timer.h"
#include "IOfferingBase.h"
#include "PoolData.h"
#include "DllMgr.h"
#include "FlowData.h"

class CDrebMsgThread : public CDrebMsgProcBase  
{
public:
	void ClearApiLog();
	

	// ������: SendQueryHostStatus
	// ���  : ������ 2016-3-29 11:40:59
	// ����  : void 
	// ����  : ���Ͳ�ѯ��������������
	void SendQueryHostStatus();

	void PrintHostStatus();

	// ������: OnArbQuery
	// ���  : ������ 2016-3-29 11:41:24
	// ����  : void 
	// ����  : S_BPC_RSMSG &rcvdata
	// ����  : �����ٲõĲ�ѯ����
	void OnArbQuery(S_BPC_RSMSG &rcvdata);

	// ������: OnArbControl
	// ���  : ������ 2016-3-29 11:41:50
	// ����  : void 
	// ����  : S_BPC_RSMSG &rcvdata
	// ����  : �����ٲõĿ�������������л�
	void OnArbControl(S_BPC_RSMSG &rcvdata);

	// ������: OnSvrRequest
	// ���  : ������ 2016-3-29 11:42:16
	// ����  : void 
	// ����  : S_BPC_RSMSG &rcvdata
	// ����  : ��������
	void OnSvrRequest(S_BPC_RSMSG &rcvdata);

	// ������: OnChangeHostStatus
	// ���  : ������ 2016-3-29 11:42:31
	// ����  : void 
	// ����  : unsigned int status
	// ����  : ����״̬�任
	void OnChangeHostStatus(unsigned int status);

	// ������: OnQueryArbAns
	// ���  : ������ 2016-3-29 11:42:53
	// ����  : void 
	// ����  : S_BPC_RSMSG &rcvdata
	// ����  : �����ٲ�Ӧ���������ѯ״̬
	void OnQueryArbAns(S_BPC_RSMSG &rcvdata);

	// ������: Send2Arb
	// ���  : ������ 2016-3-29 11:43:35
	// ����  : void 
	// ����  : ���͸��ٲ�
	void Send2Arb();

	bool  m_bIsQueryAns;
	
	CDrebMsgThread();
	virtual ~CDrebMsgThread();

//	vector <S_SGE_GEMS2_API> *m_vApiDllClass;//sge��api��̬��
	CDllMgr        *m_pSgeMgr;
	CPoolDataMsg   *m_pRcvQueue;//

	CXdp      m_pXdp;
	CXdp      m_pSendXdp;
	CBF_Timer m_pTime;
	bool      m_bIsCheck;
	CIErrlog  *m_pLog;
	CFlowData *m_pFlowData;//���ļ���ָ��

	virtual  bool Init(CBF_DrebServer   *bus,CBF_DrebResource *res);

	// ������: Start
	// ���  : ������ 2015-4-23 15:52:29
	// ����  : virtual bool 
	// ����  : ���������߳�
	virtual bool Start();
	
	// ������: Stop
	// ���  : ������ 2015-4-23 15:52:43
	// ����  : virtual void 
	// ����  : ֹͣ�����߳�
	virtual void Stop();
protected:

	int System(const char *cmd);

	static int OnTimer(unsigned int event,void *p);


	virtual void OnMsgReportBpc(S_BPC_RSMSG &rcvdata);

	virtual int Run();
	
	

	// ������: OnMsgRequest
	// ���  : ������ 2015-4-23 15:54:38
	// ����  : virtual void 
	// ����  : S_BPC_RSMSG &rcvdata
	// ����  : �������߹�������������
	virtual void OnMsgRequest(S_BPC_RSMSG &rcvdata);

	

	// ������: OnMsgConnectBack
	// ���  : ������ 2015-4-23 15:55:29
	// ����  : virtual void 
	// ����  : S_BPC_RSMSG &rcvdata
	// ����  : �����������߳ɹ�����Ӧ
	virtual void OnMsgConnectBack(S_BPC_RSMSG &rcvdata);

	// ������: OnMsgDrebAns
	// ���  : ������ 2015-4-23 15:55:58
	// ����  : virtual void 
	// ����  : S_BPC_RSMSG &rcvdata
	// ����  : ��Ӧ���߹�����Ӧ������
	virtual void OnMsgDrebAns(S_BPC_RSMSG &rcvdata);

	


};

#endif // !defined(AFX_DREBMSGTHREAD_H__52F83741_0772_4053_96E3_452305F6E404__INCLUDED_)