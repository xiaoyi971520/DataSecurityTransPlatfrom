
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "itcast_asn1_der.h"
#include "itcastderlog.h"
#include "keymng_msg.h"

typedef struct _Teacher
{
	char name[64];
	int age;
	char *p;
	int len;
} Teacher;

//�ӿڵķ�װ���

// Teacher ����
int TeacherEncode(Teacher *pteacher,unsigned char **out,int *outlen)
{
	int ret = 0;
	ITCAST_ANYBUF	*pOutData = NULL;
	ITCAST_ANYBUF	*pHeadBuf = NULL;
	ITCAST_ANYBUF	*pTemp = NULL;

	// �� pteacher->name ת��ΪITCAST_ANYBUF
	ret = DER_ITCAST_String_To_AnyBuf(&pOutData,pteacher->name,strlen(pteacher->name));
	if(0 != ret)
	{
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d \n",ret);
		return ret;
	}

	// ���� pteacher->name
	ret = DER_ItAsn1_WritePrintableString(pOutData,&pHeadBuf);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pOutData);
		printf("func DER_ItAsn1_WritePrintableString() err:%d \n",ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pOutData);
	pTemp = pHeadBuf;


	//����age
	ret = DER_ItAsn1_WriteInteger(pteacher->age,&pTemp->next);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_WriteInteger() err:%d \n",ret);
		return ret;
	}
	pTemp = pTemp->next;

	//���� pteacher->p
	ret = EncodeChar(pteacher->p,pteacher->len,&pTemp->next);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func EncodeChar() err:%d \n",ret);
		return ret;
	}
	pTemp = pTemp->next;

	// ����pteacher->len
	ret = DER_ItAsn1_WriteInteger(pteacher->len,&pTemp->next);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_WriteInteger() err:%d \n",ret);
		return ret;
	}
	/////////////////// Teacher �ӿ���4����Ա����������� ////////////

	// �Դ�ṹ�� Teacher ���� TLV ����
	ret = DER_ItAsn1_WriteSequence(pHeadBuf,&pOutData);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_WriteSequence() err:%d \n",ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pHeadBuf);

	// ��Ӹ�ֵ
	*out = (unsigned char*)malloc(pOutData->dataLen);
	if(NULL == *out)
		return 2;

	memcpy(*out,pOutData->pData,pOutData->dataLen);
	*outlen = pOutData->dataLen;
	DER_ITCAST_FreeQueue(pOutData);

	return ret;
}

// Teacher ����
int TeacherDecode(unsigned char *in,int inlen,Teacher **pteacher)
{
	int ret = 0;
	ITCAST_ANYBUF	*inAnyBuf = NULL;
	ITCAST_ANYBUF	*pHead = NULL;
	ITCAST_ANYBUF	*pTemp = NULL;
	Teacher			*pTmpStru = NULL;
	ITCAST_ANYBUF	*pTmpDaBuf = NULL;

	//��C�ڴ������� ת���� ITCAST_ANYBUF
	inAnyBuf = malloc(sizeof(ITCAST_ANYBUF));
	if(NULL == inAnyBuf)
	{
		printf("fun malloc() err��1\n");
		return 1;
	}
	memset(inAnyBuf,0,sizeof(inAnyBuf));
	inAnyBuf->next = NULL; 

	inAnyBuf->pData = (unsigned char*)malloc(inlen);
	if(NULL == inAnyBuf->pData)
	{
		printf("fun malloc() err��1 \n");
		return 1;
	}
	memcpy(inAnyBuf->pData,in,inlen);
	inAnyBuf->dataLen = inlen;

	//������ṹ�� �õ�����
	ret = DER_ItAsn1_ReadSequence(inAnyBuf,&pHead);
	if(NULL == inAnyBuf->pData)
	{
		printf("fun DER_ItAsn1_ReadSequence() err��1 \n");
		return 1;
	}
	pTemp = pHead;
	
	//�����ṹ��
	pTmpStru = (Teacher *)malloc(sizeof(Teacher));
	if(NULL == pTmpStru)
	{
		ret = 3;
		printf("fun malloc() err : %d \n",ret);
		return ret;
	}
	memset(pTmpStru,0,sizeof(Teacher));

	//���� Teacher::name
	ret = DER_ItAsn1_ReadPrintableString(pTemp,&pTmpDaBuf);
	if(0 != ret)
	{
		printf("fun DER_ItAsn1_ReadPrintableString() : err %d\n",ret);
		return ret;
	}
	memcpy(pTmpStru->name,pTmpDaBuf->pData,pTmpDaBuf->dataLen);
	pTemp = pTemp->next;

	//���� age
	ret = DER_ItAsn1_ReadInteger(pTemp,&pTmpStru->age);
	if(0 != ret)
	{
		printf("fun DER_ItAsn1_ReadInteger() err:%d",ret);
		return ret;
	}
	pTemp = pTemp->next;

	//���� p
	ret = DER_ItAsn1_ReadPrintableString(pTemp,&pTmpDaBuf);
	if(0 != ret)
	{
		printf("fun DER_ItAsn1_ReadPrintableString() err:%d \n",ret);
		return ret;
	}
	pTmpStru->p = (char *)malloc(pTmpDaBuf->dataLen + 1);
	if(NULL == pTmpStru->p)
	{
		printf("fun malloc() err:%d \n",ret);
		return ret;
	}
	memset(pTmpStru->p,0,pTmpDaBuf->dataLen + 1);
	memcpy(pTmpStru->p,pTmpDaBuf->pData,pTmpDaBuf->dataLen);
	pTmpStru->p[pTmpDaBuf->dataLen] = '\0';
	pTemp = pTemp->next;

	//���� plen
	ret = DER_ItAsn1_ReadInteger(pTemp,&pTmpStru->len);
	if(0 != ret)
	{
		printf("fun DER_ItAsn1_ReadInteger() err:%d \n",ret);
		return ret;
	}
	
	//��ֵ
	*pteacher = pTmpStru;

	return ret;
}


int main()
{
//	int ret = 0; //���ؽ����
//	unsigned char *myout = NULL; //���뷵�ص��� TLV ��
//	int myoutlen = 0;  //���ص��ַ�����
//	Teacher*	myteacher = NULL;
//
//	//��ʼ��һ��Teacher�ṹ
//	Teacher	teacher;
//	memset(&teacher,0,sizeof(Teacher));
//	strcpy(teacher.name,"wangwu");
//	teacher.age = 32;
//	teacher.p = (char*)malloc(100);
//	memset(teacher.p,0,100);
//	strcpy(teacher.p,"This is Teacher wang!");
//	teacher.len = strlen("This is Teacher wang!");
//
//	// Teacher ����
//	ret = TeacherEncode(&teacher,&myout,&myoutlen);
//	if(0 != ret)
//	{
//		printf("fun TeacherEncode() err:%d \n",ret);
//		goto End;
//	}
//
//	// Teacher ����
//	ret = TeacherDecode(myout,myoutlen,&myteacher);
//	if(0 != ret)
//	{
//		printf("func TeacherDecode() err:%d \n",ret);
//		goto End;
//	}
//
//	//У�������Ƿ���ȷ
//	if(0 == strcmp(teacher.name,myteacher->name) && teacher.age == myteacher->age)
//	{
//		printf("����ͽ���Ľ�� ok\n");
//	}
//	else
//	{
//		printf("����ͽ���Ľ�� err \n");
//	}
//
//End:
//	if(teacher.p != NULL)
//		free(teacher.p);
//
//	system("pause");

	unsigned char *pout = NULL;
	int poutlen = 0;

	int ret = 0;

	MsgKey_Req *poutReq = NULL;
	MsgKey_Res *poutRes = NULL;
	int itype = 0;

	//MsgKey_Req *pReq = (MsgKey_Req *)malloc(sizeof(MsgKey_Req));
	//memset(pReq,0,sizeof(MsgKey_Req));
	//pReq->cmdType = 3;

	//strcpy(pReq->clientId ,"192.168.31.1");
	//strcpy(pReq->authCode , "test1111");
	//strcpy(pReq->serverId , "192.168.31.2");
	//strcpy(pReq->r1 , "729438394304");
	//ret = MsgEncode((void*)pReq,ID_MsgKey_Req,&pout,&poutlen);
	//ret = MsgDecode(pout,poutlen,&poutReq,&itype);
	
	MsgKey_Res *pRes = (MsgKey_Res *)malloc(sizeof(MsgKey_Res));
	memset(pRes,0,sizeof(MsgKey_Res));

	pRes->rv = 2;
	memcpy(pRes->clientId,"clientId",strlen("clientId"));
	memcpy(pRes->serverId,"serverId",strlen("serverId"));
	memcpy(pRes->r2,"r2r2r2r2",strlen("r2r2r2r2"));
	pRes->seckeyid = 3456;

	ret = MsgEncode((void*)pRes,ID_MsgKey_Res,&pout,&poutlen);
	ret = MsgDecode(pout,poutlen,&poutRes,&itype);

	return 0;
}