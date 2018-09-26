
#define _CRT_SECURE_NO_WARNINGS
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "keymng_msg.h"
#include "itcastderlog.h"
#include "itcast_asn1_der.h"

///////////////////////编码

/*
 pstruct :	输入的报文数据 ; (指向相应结构体的指针) 
 type :		输入的类型标识(函数内部通过type 得到 pstruct 所指向的报文类型)
 poutData:	输出的编码后的报文 ; 
 outlen :	输出的数据长度;
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

	//编码type
	ret = DER_ItAsn1_WriteInteger(type, &pHeadbuf);
	if (ret != 0)
	{
		printf("func DER_ItAsn1_WriteInteger() err:%d \n", ret);
		return ret;
	}

	//根据type 编码结构体
	switch (type)
	{
	case ID_MsgKey_Req:
		ret = MsgKey_Req_Encode(pStruct,&pTemp);
		//TODO
		break;
	case ID_MsgKey_Res:
		ret = MsgKey_Res_Encode(pStruct,&pTemp);
		//TODO
		break;
	default:
		ret = KeyMng_TypeErr;
		printf("type类型不支持:type:%d\n",type);
		return ret;
	}

	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHeadbuf);
		printf("编码结构体失败:%d\n",ret);
		return ret;
	}

	//做成链表
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

	//间接赋值 将运算结果传递给接受变量并传出
	memcpy(*outData,pOutData->pData,pOutData->dataLen);
	*outLen = pOutData->dataLen;
	DER_ITCAST_FreeQueue(pHeadbuf);
	DER_ITCAST_FreeQueue(pOutData);

	return 0;
}

/*
 inData		: 输入的编码后的数据;
 inLen		: 输入的数据长度 ;
 pstruct	: 输出的解码后的数据; (其空间是在内部开辟的，也需要用内部定义的free函数进行释放)
 type		: 结构的类型标识(返回类型标识，使得调用者通过flag进行判断，将pstruct 转换为相应的结构)
*/
int MsgDecode(unsigned char *inData,/*in*/
	int           inLen,
	void          **pStruct /*out*/,
	int           *type /*out*/)
{
	int 			ret = 0;
	ITCAST_ANYBUF		*pHeadBuf = NULL,*pTemp = NULL;
	int				itype = 0;

	//检查参数是否正确
	if(NULL == inData || inLen<0 || NULL == pStruct || NULL == type)
	{
		ret = KeyMng_ParamErr;
		printf("func MsgDecode() err：%d \n",ret);
		return ret;
	}

	//把字符流转为 ITCAST_ANYBUF
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

	//解析大结构体
	ret = DER_ItAsn1_ReadSequence(pTemp,&pHeadBuf);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pTemp);
		printf("func DER_ItAsn1_ReadSequence() err:%d \n",ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pTemp);
	pTemp = NULL;

	//解析type
	ret = DER_ItAsn1_ReadInteger(pHeadBuf,&itype);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pTemp);
		printf("func DER_ItAsn1_ReadInteger() err:%d \n",ret);
		return ret;
	}
	pTemp = pHeadBuf->next;

	//根据type 解析对应的结构体
	switch(itype)
	{
	case ID_MsgKey_Req:
		ret = MsgKey_Req_Decode(pTemp->pData,pTemp->dataLen,pStruct);
		//TODO
		break;
	case  ID_MsgKey_Res:
		ret = MsgKey_Res_Decode(pTemp->pData,pTemp->dataLen,pStruct);
		//TODO
		break;
	default:
		ret = KeyMng_TypeErr;
		printf("类型不支持 %d \n",ret);
		break;
	}

	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHeadBuf);
		return ret;
	}

	//赋值
	type = itype;
	DER_ITCAST_FreeQueue(pHeadBuf);
	return ret;
}

/*
释放 MsgEncode( )函数中的outData; 方法：MsgMemFree((void **)outData, 0); 
释放MsgDecode( )函数中的pstruct结构体，MsgMemFree((void **)outData, type);
type : 输入参数,便于函数判断调用哪个结构体的free函数
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
			MsgKey_Res_Free(point);
			break;
		default:
			break;
		}
	}

	return 0;
}

// 密钥请求报文 编码
int MsgKey_Req_Encode(MsgKey_Req *pReq,ITCAST_ANYBUF **out)
{
	int ret = 0;
	ITCAST_ANYBUF	*pHead = NULL,*pTemp = NULL,*pStringTemp = NULL;

	//编码MsgKey_Req::cmdType
	ret = DER_ItAsn1_WriteInteger(pReq->cmdType,&pHead);
	if(0!=ret)
	{
		ret = 1;
		printf("Encode MsgKey_Req::cmdType err:%d \n",ret);
		return ret;
	}
	pTemp = pHead;

	//编码MsgKey_Req::ClientId[12] 客户端IP编码
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

	//编码MsgKey_Req::authCode[16] 客户端IP编码
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

	//编码MsgKey_Req::serverId[12] 客户端IP编码
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

	//编码MsgKey_Req::r1[64] 客户端IP编码
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

	// 编码大结构体
	ret = DER_ItAsn1_WriteSequence(pHead,out);
	if(0 != ret)
	{
		ret = 1;
		printf(" Encode MsgKey_Req err:%d \n",ret);
		return ret;
	}

	return 0;
}

// 密钥请求报文 解码
int MsgKey_Req_Decode(unsigned char *indata,int inlen,MsgKey_Req **pReq)
{
	int ret = 0;
	ITCAST_ANYBUF		*pHead = NULL,*pTmp = NULL,*pStingTemp = NULL;
	MsgKey_Req			*pmsgkeyreq = NULL;
	ITCAST_ANYBUF		*itAnybuf = NULL;

	//创建 ITCAST_ANYBUF 结构体
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

	// 解码大结构体
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
	// 解码cmdType
	ret = DER_ItAsn1_ReadInteger(pTmp,&(pmsgkeyreq->cmdType));
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf(" Decode MsgKey_Req::cmdType err:%d \n",ret);
		return ret;
	}
	pTmp = pTmp->next;

	// 解码客户端IP
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

	//解码认证码
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

	//解码服务端IP
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
	
	//解码客户端随机数
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

// 密钥请求报文 编码
int MsgKey_Res_Encode(MsgKey_Res *pRes,ITCAST_ANYBUF **out)
{
	int ret = 0;
	ITCAST_ANYBUF *pHead = NULL,*pTmp = NULL;
	ITCAST_ANYBUF *pTempString = NULL;

	//编码MsgKey_Res::rv
	ret = DER_ItAsn1_WriteInteger(pRes->rv,&pHead);
	if(0 != ret)
	{
		ret = 1;
		printf("Encode MsgKey_Res::rv err:%d \n",ret);
		return ret;
	}
	pTmp = pHead;

	//编码MsgKey_Res::clientId
	ret = DER_ITCAST_String_To_AnyBuf(&pTempString,pRes->clientId,strlen(pRes->clientId));
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf("Encode MsgKey_Res::clientId DER_ITCAST_String_T0_AnyBuf err:%d \n",ret);
		return ret;
	}
	ret = DER_ItAsn1_WritePrintableString(pTempString,&pTmp->next);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		DER_ITCAST_FreeQueue(pTempString);
		ret = 1;
		printf("Encode MsgKey_Res::clientId err:%d \n",ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pTempString);
	pTempString = NULL;
	pTmp = pTmp->next;
	
	//编码MsgKey_Res::serverId
	ret = DER_ITCAST_String_To_AnyBuf(&pTempString,pRes->serverId,strlen(pRes->serverId));
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf("Encode MsgKey_Res::server DER_ITCAST_String_T0_AnyBuf err:%d \n",ret);
		return ret;
	}
	ret = DER_ItAsn1_WritePrintableString(pTempString,&pTmp->next);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		DER_ITCAST_FreeQueue(pTempString);
		ret = 1;
		printf("Encode MsgKey_Res::serverId err:%d \n",ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pTempString);
	pTempString = NULL;
	pTmp = pTmp->next;

	//编码MsgKey_Res::r2
	ret = DER_ITCAST_String_To_AnyBuf(&pTempString,pRes->r2,strlen(pRes->r2));
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf("Encode MsgKey_Res::r2 DER_ITCAST_String_T0_AnyBuf err:%d \n",ret);
		return ret;
	}
	ret = DER_ItAsn1_WritePrintableString(pTempString,&pTmp->next);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		DER_ITCAST_FreeQueue(pTempString);
		ret = 1;
		printf("Encode MsgKey_Res::r2 err:%d \n",ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pTempString);
	pTempString = NULL;
	pTmp = pTmp->next;

	//编码MsgKey_Res::seckeyid
	ret = DER_ItAsn1_WriteInteger(pRes->seckeyid,&pTmp->next);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf("Encode MsgKey_Res::seckeyid err:%d \n",ret);
		return ret;
	}

	//编码整体结构
	ret = DER_ItAsn1_WriteSequence(pHead,out);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf("Encode MsgKey_Res err:%d \n",ret);
		return ret;
	}

	return 0;
}

//请求报文 解码
int MsgKey_Res_Decode(unsigned char *indata,int inlen,MsgKey_Res **pRes)
{
	int ret;
	ITCAST_ANYBUF *pHead = NULL,*pTmp = NULL,*pStringTemp = NULL;
	MsgKey_Res *pMKR = NULL;
	ITCAST_ANYBUF *itAnyBuf = NULL;

	MsgKey_Res *pMsgKeyRes = (MsgKey_Res *)malloc(sizeof(MsgKey_Res));
	memset(pMsgKeyRes,0,sizeof(MsgKey_Res));

	//创建ITCAST_ANYBUF 结构体
	itAnyBuf = (ITCAST_ANYBUF *)malloc(sizeof(ITCAST_ANYBUF));
	if (itAnyBuf == NULL)
	{
		ret = 1;
		printf("func decode MsgKey_Req ITCAST_ANYBUF err:%d malloc err \n", ret);
		return ret;
	}
	itAnyBuf->pData = (unsigned char *)malloc(inlen);
	if (itAnyBuf->pData == NULL)
	{
		ret = 2;
		printf("func decode MsgKey_Req ITCAST_ANYBUF pData err:%d malloc err \n", ret);
		return ret;
	}
	memcpy(itAnyBuf->pData,indata,inlen);
	itAnyBuf->dataLen = inlen;
	itAnyBuf->next = NULL;
	itAnyBuf->prev = NULL;

	//解码整体结构
	ret = DER_ItAsn1_ReadSequence(itAnyBuf,&pHead);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(itAnyBuf);
		ret = 1;
		printf(" Decode MsgKey_Req err:%d \n",ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(itAnyBuf);
	pTmp = pHead;

	//解码MsgKey_Res::rv
	ret = DER_ItAsn1_ReadInteger(pTmp,&(pMsgKeyRes->rv));
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf(" Decode MsgKey_Req err:%d \n",ret);
		return ret;
	}
	pTmp = pTmp->next;

	//解码MsgKey_Res::clientId
	ret = DER_ItAsn1_ReadPrintableString(pTmp,&pStringTemp);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf(" Decode MsgKey_Req::clientId err:%d \n",ret);
		return ret;
	}
	memcpy(pMsgKeyRes->clientId,pStringTemp->pData,pStringTemp->dataLen);
	DER_ITCAST_FreeQueue(pStringTemp);
	pStringTemp = NULL;
	pTmp = pTmp->next;

	//解码MsgKey_Res::serverId
	ret = DER_ItAsn1_ReadPrintableString(pTmp,&pStringTemp);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf(" Decode MsgKey_Req::serverId err:%d \n",ret);
		return ret;
	}
	memcpy(pMsgKeyRes->serverId,pStringTemp->pData,pStringTemp->dataLen);
	DER_ITCAST_FreeQueue(pStringTemp);
	pStringTemp = NULL;
	pTmp = pTmp->next;

	//解码MsgKey_Res::r2
	ret = DER_ItAsn1_ReadPrintableString(pTmp,&pStringTemp);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf(" Decode MsgKey_Req::r2 err:%d \n",ret);
		return ret;
	}
	memcpy(pMsgKeyRes->r2,pStringTemp->pData,pStringTemp->dataLen);
	DER_ITCAST_FreeQueue(pStringTemp);
	pStringTemp = NULL;
	pTmp = pTmp->next;

	//解码MsgKey_Res::seckeyid
	ret = DER_ItAsn1_ReadInteger(pTmp,&(pMsgKeyRes->seckeyid));
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHead);
		ret = 1;
		printf(" Decode MsgKey_Req::seckeyid err:%d \n",ret);
		return ret;
	}

	DER_ITCAST_FreeQueue(pHead);
	*pRes = pMsgKeyRes;
	return 0;
}


// 密钥请求报文 资源释放
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
// 密钥请求报文 资源释放
int MsgKey_Res_Free(MsgKey_Res **pRes)
{
	MsgKey_Res *pTemp = NULL;
	if(NULL == pRes)
		return 0;

	pTemp = *pRes;
	if(NULL == pTemp)
		return 0;

	free(pTemp);
	*pRes = NULL;

	return 0;
}