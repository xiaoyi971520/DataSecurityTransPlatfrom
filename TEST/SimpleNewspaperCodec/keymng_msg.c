
#define _CRT_SECURE_NO_WARNINGS
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "keymng_msg.h"
#include "itcastderlog.h"
#include "itcast_asn1_der.h"

///////////////////////����

/*
 pstruct :	����ı������� ; (ָ����Ӧ�ṹ���ָ��) 
 type :		��������ͱ�ʶ(�����ڲ�ͨ��type �õ� pstruct ��ָ��ı�������)
 poutData:	����ı����ı��� ; 
 outlen :	��������ݳ���;
*/
int MsgEncode(void			*pStruct , /*in*/
			  int				type,
			  unsigned char	**outData, /*out*/
			  int				*outLen )
{
	ITCAST_ANYBUF		*pHeadbuf = NULL,*pTemp = NULL;
	ITCAST_ANYBUF		*pOutData = NULL;
	int					ret = 0;

	if(NULL == pStruct || type < 0 || NULL == outData || NULL == outLen)
	{
		ret = KeyMng_ParamErr;
		printf("func MsgEncode() check(NULL == pStruct || type < 0 || NULL == outData || NULL == outLen) : %d",ret);
		return ret;
	}

	//����type
	ret = DER_ItAsn1_WriteInteger(type, &pHeadbuf);
	if (ret != 0)
	{
		printf("func DER_ItAsn1_WriteInteger() err:%d \n", ret);
		return ret;
	}

	//����type ����ṹ��
	switch (type)
	{
	case ID_MsgKey_Req:
		ret = MsgKey_Req_Encode(pStruct,&pTemp);
		//TODO
		break;
	case ID_MsgKey_Res:
		//TODO
		break;
	default:
		ret = KeyMng_TypeErr;
		printf("type���Ͳ�֧��:type:%d\n",type);
		return ret;
	}

	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHeadbuf);
		printf("����ṹ��ʧ��:%d\n",ret);
		return ret;
	}

	//��������
	pHeadbuf->next = pTemp;
	
	ret = DER_ItAsn1_WriteSequence(pHeadbuf,&pOutData);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHeadbuf);
		printf("func DER_ItAsn1_WriteSequence() err:%d \n",ret);
		return ret;
	}
	
	*outData = (unsigned char *)malloc(pOutData->dataLen);
	if(NULL == *outData)
	{
		DER_ITCAST_FreeQueue(pHeadbuf);
		DER_ITCAST_FreeQueue(pOutData);
		ret = KeyMng_MallocErr;
		printf("func malloc() err:%d\n",ret);
		return ret;
	}

	//��Ӹ�ֵ �����������ݸ����ܱ���������
	memcpy(*outData,pOutData->pData,pOutData->dataLen);
	*outLen = pOutData->dataLen;
	DER_ITCAST_FreeQueue(pHeadbuf);
	DER_ITCAST_FreeQueue(pOutData);

	return 0;
}

/*
 inData		: ����ı���������;
 inLen		: ��������ݳ��� ;
 pstruct	: ����Ľ���������; (��ռ������ڲ����ٵģ�Ҳ��Ҫ���ڲ������free���������ͷ�)
 type		: �ṹ�����ͱ�ʶ(�������ͱ�ʶ��ʹ�õ�����ͨ��flag�����жϣ���pstruct ת��Ϊ��Ӧ�Ľṹ)
*/
int MsgDecode(unsigned char *inData,/*in*/
	int           inLen,
	void          **pStruct /*out*/,
	int           *type /*out*/)
{
	int 			ret = 0;
	ITCAST_ANYBUF		*pHeadBuf = NULL,*pTemp = NULL;
	int				itype = 0;

	//�������Ƿ���ȷ
	if(NULL == inData || inLen<0 || NULL == pStruct || NULL == type)
	{
		ret = KeyMng_ParamErr;
		printf("func MsgDecode() err��%d \n",ret);
		return ret;
	}

	//���ַ���תΪ ITCAST_ANYBUF
	pTemp = (ITCAST_ANYBUF *)malloc(sizeof(ITCAST_ANYBUF));
	if(NULL == pTemp)
	{
		ret = KeyMng_MallocErr;
		printf("func malloc() err:%d \n",ret);
		return ret;
	}
	memset(pTemp,0,sizeof(ITCAST_ANYBUF));

	pTemp->pData = (unsigned char *)malloc(inLen);
	if(NULL == pTemp->pData)
	{
		ret = KeyMng_MallocErr;
		printf("func malloc() err: \n",ret);
		return ret;
	}
	memset(pTemp->pData,0,inLen);
	memcpy(pTemp->pData,inData,inLen);
	pTemp->dataLen = inLen;

	//������ṹ��
	ret = DER_ItAsn1_ReadSequence(pTemp,&pHeadBuf);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pTemp);
		printf("func DER_ItAsn1_ReadSequence() err:%d \n",ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pTemp);

	//����type
	ret = DER_ItAsn1_ReadInteger(pHeadBuf,&itype);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pTemp);
		printf("func DER_ItAsn1_ReadInteger() err:%d \n",ret);
		return ret;
	}

	//����type ������Ӧ�Ľṹ��
	switch(itype)
	{
	case ID_MsgKey_Req:
		MsgKey_Req_Decode(pHeadBuf->next->pData,pHeadBuf->next->dataLen,pStruct);
		//TODO
		break;
	case  ID_MsgKey_Res:
		//TODO
		break;
	default:
		ret = KeyMng_TypeErr;
		printf("���Ͳ�֧�� %d \n",ret);
		break;
	}

	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHeadBuf);
		return ret;
	}

	//��ֵ
	type = itype;
	DER_ITCAST_FreeQueue(pHeadBuf);
	return ret;
	
}

/*
�ͷ� MsgEncode( )�����е�outData; ������MsgMemFree((void **)outData, 0); 
�ͷ�MsgDecode( )�����е�pstruct�ṹ�壬MsgMemFree((void **)outData, type);
type : �������,���ں����жϵ����ĸ��ṹ���free����
*/ 

int MsgMemFree(void **point,int type)
{
	if (point == NULL)
	{
		return 0;
	}

	if (type == 0)
	{
		if (*point != NULL)
		{
			free(*point);
			*point = NULL;
		}
	}
	else
	{
		switch (type)
		{
		case ID_MsgKey_Req:
			MsgKey_Req_Free(point);
			break;
		case ID_MsgKey_Res:
			break;
		default:
			break;
		}
	}

	return 0;
}

// ��Կ������ ����
int MsgKey_Req_Encode(MsgKey_Req *pReq,ITCAST_ANYBUF **out)
{
	int ret = 0;
	ITCAST_ANYBUF	*pHead = NULL,*pTemp = NULL,*pStringTemp = NULL;

	//����MsgKey_Req::cmdType
	ret = DER_ItAsn1_WriteInteger(pReq->cmdType,&pHead);
	if(0!=ret)
	{
		ret = 1;
		printf("Encode MsgKey_Req::cmdType err:%d \n",ret);
		return ret;
	}
	pTemp = pHead;

	//����MsgKey_Req::ClientId[12] �ͻ���IP����
	ret = DER_ITCAST_String_To_AnyBuf(&pStringTemp,pReq->clientId,strlen(pReq->clientId));
	if(0 != ret)
	{
		ret = 1;
		printf("Encode MsgKey_Req::ClientId[12] DER_ITCAST_String_To_AnyBuf err:%d \n",ret);
		return ret;
	}

	ret = DER_ItAsn1_WritePrintableString(pStringTemp,&pTemp->next);
	if(0!=ret)
	{
		DER_ITCAST_FreeQueue(pStringTemp);
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf("Encode MsgKey_Req::ClientId[12] err:%d \n",ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pStringTemp);
	pStringTemp = NULL;

	pTemp = pTemp->next;

	//����MsgKey_Req::authCode[16] �ͻ���IP����
	ret = DER_ITCAST_String_To_AnyBuf(&pStringTemp,pReq->authCode,strlen(pReq->authCode));
	if(0 != ret)
	{
		ret = 1;
		printf("Encode MsgKey_Req::ClientId[12] DER_ITCAST_String_To_AnyBuf err:%d \n",ret);
		return ret;
	}

	ret = DER_ItAsn1_WritePrintableString(pStringTemp,&pTemp->next);
	if(0!=ret)
	{
		DER_ITCAST_FreeQueue(pStringTemp);
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf("Encode MsgKey_Req::authCode[16] err:%d \n",ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pStringTemp);
	pStringTemp = NULL;
	pTemp = pTemp->next;

	//����MsgKey_Req::serverId[12] �ͻ���IP����
	ret = DER_ITCAST_String_To_AnyBuf(&pStringTemp,pReq->serverId,strlen(pReq->serverId));
	if(0 != ret)
	{
		ret = 1;
		printf("Encode MsgKey_Req::ClientId[12] DER_ITCAST_String_To_AnyBuf err:%d \n",ret);
		return ret;
	}
	ret = DER_ItAsn1_WritePrintableString(pStringTemp,&pTemp->next);
	if(0!=ret)
	{
		DER_ITCAST_FreeQueue(pStringTemp);
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf("Encode MsgKey_Req::serverId[12] err:%d \n",ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pStringTemp);
	pStringTemp = NULL;
	pTemp = pTemp->next;

	//����MsgKey_Req::r1[64] �ͻ���IP����
	ret = DER_ITCAST_String_To_AnyBuf(&pStringTemp,pReq->r1,strlen(pReq->r1));
	if(0 != ret)
	{
		ret = 1;
		printf("Encode MsgKey_Req::ClientId[12] DER_ITCAST_String_To_AnyBuf err:%d \n",ret);
		return ret;
	}
	ret = DER_ItAsn1_WritePrintableString(pStringTemp,&pTemp->next);
	if(0!=ret)
	{
		DER_ITCAST_FreeQueue(pStringTemp);
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf("Encode MsgKey_Req::r1[64] err:%d \n",ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pStringTemp);
	pStringTemp = NULL;

	// �����ṹ��
	ret = DER_ItAsn1_WriteSequence(pHead,out);
	if(0 != ret)
	{
		ret = 1;
		printf(" Encode MsgKey_Req err:%d \n",ret);
		return ret;
	}

	return 0;
}


// ��Կ������ ����
int MsgKey_Req_Decode(unsigned char *indata,int inlen,MsgKey_Req **pReq)
{
	int ret = 0;
	ITCAST_ANYBUF		*pHead = NULL,*pTmp = NULL,*pStingTemp = NULL;
	MsgKey_Req			*pmsgkeyreq = NULL;
	ITCAST_ANYBUF		*itAnybuf = NULL;

	//���� ITCAST_ANYBUF �ṹ��
	itAnybuf = (ITCAST_ANYBUF *)malloc(sizeof(ITCAST_ANYBUF));
	if (itAnybuf == NULL)
	{
		ret = 1;
		printf("func decode MsgKey_Req ITCAST_ANYBUF err:%d malloc err \n", ret);
		return ret;
	}
	memset(itAnybuf,0,sizeof(ITCAST_ANYBUF));
	itAnybuf->next = NULL;
	itAnybuf->prev = NULL;

	itAnybuf->pData = (unsigned char*)malloc(inlen);
	if (itAnybuf->pData == NULL)
	{
		ret = 2;
		printf("func decode MsgKey_Req ITCAST_ANYBUF pData err:%d malloc err \n", ret);
		return ret;
	}
	memset(itAnybuf->pData,0,inlen);
	memcpy(itAnybuf->pData,indata,inlen);
	itAnybuf->dataLen = inlen;

	// �����ṹ��
	ret = DER_ItAsn1_ReadSequence(itAnybuf,&pHead);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(itAnybuf);
		ret = 1;
		printf(" Decode MsgKey_Req err:%d \n",ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(itAnybuf);

	pmsgkeyreq = (MsgKey_Req *)malloc(sizeof(MsgKey_Req));
	if (NULL == pmsgkeyreq)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 2;
		printf("func decode MsgKey_Req err:%d malloc err \n", ret);
		return ret;
	}
	memset(pmsgkeyreq,0,sizeof(MsgKey_Req));
	pTmp = pHead;
	// ����cmdType
	ret = DER_ItAsn1_ReadInteger(pTmp,&(pmsgkeyreq->cmdType));
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf(" Decode MsgKey_Req::cmdType err:%d \n",ret);
		return ret;
	}
	pTmp = pTmp->next;

	// ����ͻ���IP
	ret = DER_ItAsn1_ReadPrintableString(pTmp,&pStingTemp);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf(" Decode MsgKey_Req::clientId err:%d \n",ret);
		return ret;
	}
	memcpy(pmsgkeyreq->clientId,pStingTemp->pData,pStingTemp->dataLen);
	DER_ITCAST_FreeQueue(pStingTemp);
	pStingTemp = NULL;
	pTmp = pTmp->next;

	//������֤��
	ret = DER_ItAsn1_ReadPrintableString(pTmp,&pStingTemp);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf(" Decode MsgKey_Req::authcode err:%d \n",ret);
		return ret;
	}
	memcpy(pmsgkeyreq->authCode,pStingTemp->pData,pStingTemp->dataLen);
	DER_ITCAST_FreeQueue(pStingTemp);
	pStingTemp = NULL;
	pTmp = pTmp->next;

	//��������IP
	ret = DER_ItAsn1_ReadPrintableString(pTmp,&pStingTemp);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf(" Decode MsgKey_Req::serverId err:%d \n",ret);
		return ret;
	}
	memcpy(pmsgkeyreq->serverId,pStingTemp->pData,pStingTemp->dataLen);
	DER_ITCAST_FreeQueue(pStingTemp);
	pStingTemp = NULL;
	pTmp = pTmp->next;
	
	//����ͻ��������
	ret = DER_ItAsn1_ReadPrintableString(pTmp,&pStingTemp);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf(" Decode MsgKey_Req::serverId err:%d \n",ret);
		return ret;
	}
	memcpy(pmsgkeyreq->r1,pStingTemp->pData,pStingTemp->dataLen);
	DER_ITCAST_FreeQueue(pStingTemp);
	pStingTemp = NULL;
	DER_ITCAST_FreeQueue(pHead);
	
	*pReq = pmsgkeyreq;

	return 0;
}

// ��Կ������ ��Դ�ͷ�
int MsgKey_Req_Free(MsgKey_Req **pReq)
{
	MsgKey_Req *pTemp = NULL;
	if(NULL == pReq)
		return 0;

	pTemp = *pReq;
	if(NULL == pTemp)
		return 0;

	free(pTemp);
	*pReq = NULL;

	return 0;
}