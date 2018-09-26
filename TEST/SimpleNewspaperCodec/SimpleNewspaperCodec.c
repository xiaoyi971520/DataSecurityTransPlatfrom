
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

//接口的封装设计

// Teacher 编码
int TeacherEncode(Teacher *pteacher,unsigned char **out,int *outlen)
{
	int ret = 0;
	ITCAST_ANYBUF	*pOutData = NULL;
	ITCAST_ANYBUF	*pHeadBuf = NULL;
	ITCAST_ANYBUF	*pTemp = NULL;

	// 把 pteacher->name 转化为ITCAST_ANYBUF
	ret = DER_ITCAST_String_To_AnyBuf(&pOutData,pteacher->name,strlen(pteacher->name));
	if(0 != ret)
	{
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d \n",ret);
		return ret;
	}

	// 编码 pteacher->name
	ret = DER_ItAsn1_WritePrintableString(pOutData,&pHeadBuf);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pOutData);
		printf("func DER_ItAsn1_WritePrintableString() err:%d \n",ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pOutData);
	pTemp = pHeadBuf;


	//编码age
	ret = DER_ItAsn1_WriteInteger(pteacher->age,&pTemp->next);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_WriteInteger() err:%d \n",ret);
		return ret;
	}
	pTemp = pTemp->next;

	//编码 pteacher->p
	ret = EncodeChar(pteacher->p,pteacher->len,&pTemp->next);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func EncodeChar() err:%d \n",ret);
		return ret;
	}
	pTemp = pTemp->next;

	// 编码pteacher->len
	ret = DER_ItAsn1_WriteInteger(pteacher->len,&pTemp->next);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_WriteInteger() err:%d \n",ret);
		return ret;
	}
	/////////////////// Teacher 接口中4个成员变量编码完成 ////////////

	// 对大结构提 Teacher 进行 TLV 操作
	ret = DER_ItAsn1_WriteSequence(pHeadBuf,&pOutData);
	if(0 != ret)
	{
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_WriteSequence() err:%d \n",ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pHeadBuf);

	// 间接赋值
	*out = (unsigned char*)malloc(pOutData->dataLen);
	if(NULL == *out)
		return 2;

	memcpy(*out,pOutData->pData,pOutData->dataLen);
	*outlen = pOutData->dataLen;
	DER_ITCAST_FreeQueue(pOutData);

	return ret;
}

// Teacher 解码
int TeacherDecode(unsigned char *in,int inlen,Teacher **pteacher)
{
	int ret = 0;
	ITCAST_ANYBUF	*inAnyBuf = NULL;
	ITCAST_ANYBUF	*pHead = NULL;
	ITCAST_ANYBUF	*pTemp = NULL;
	Teacher			*pTmpStru = NULL;
	ITCAST_ANYBUF	*pTmpDaBuf = NULL;

	//把C内存块的数据 转化成 ITCAST_ANYBUF
	inAnyBuf = malloc(sizeof(ITCAST_ANYBUF));
	if(NULL == inAnyBuf)
	{
		printf("fun malloc() err：1\n");
		return 1;
	}
	memset(inAnyBuf,0,sizeof(inAnyBuf));
	inAnyBuf->next = NULL; 

	inAnyBuf->pData = (unsigned char*)malloc(inlen);
	if(NULL == inAnyBuf->pData)
	{
		printf("fun malloc() err：1 \n");
		return 1;
	}
	memcpy(inAnyBuf->pData,in,inlen);
	inAnyBuf->dataLen = inlen;

	//解析大结构体 得到链表
	ret = DER_ItAsn1_ReadSequence(inAnyBuf,&pHead);
	if(NULL == inAnyBuf->pData)
	{
		printf("fun DER_ItAsn1_ReadSequence() err：1 \n");
		return 1;
	}
	pTemp = pHead;
	
	//创建结构体
	pTmpStru = (Teacher *)malloc(sizeof(Teacher));
	if(NULL == pTmpStru)
	{
		ret = 3;
		printf("fun malloc() err : %d \n",ret);
		return ret;
	}
	memset(pTmpStru,0,sizeof(Teacher));

	//解码 Teacher::name
	ret = DER_ItAsn1_ReadPrintableString(pTemp,&pTmpDaBuf);
	if(0 != ret)
	{
		printf("fun DER_ItAsn1_ReadPrintableString() : err %d\n",ret);
		return ret;
	}
	memcpy(pTmpStru->name,pTmpDaBuf->pData,pTmpDaBuf->dataLen);
	pTemp = pTemp->next;

	//解码 age
	ret = DER_ItAsn1_ReadInteger(pTemp,&pTmpStru->age);
	if(0 != ret)
	{
		printf("fun DER_ItAsn1_ReadInteger() err:%d",ret);
		return ret;
	}
	pTemp = pTemp->next;

	//解码 p
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

	//解码 plen
	ret = DER_ItAsn1_ReadInteger(pTemp,&pTmpStru->len);
	if(0 != ret)
	{
		printf("fun DER_ItAsn1_ReadInteger() err:%d \n",ret);
		return ret;
	}
	
	//赋值
	*pteacher = pTmpStru;

	return ret;
}


int main()
{
//	int ret = 0; //返回结果码
//	unsigned char *myout = NULL; //编码返回的字 TLV 流
//	int myoutlen = 0;  //返回的字符长度
//	Teacher*	myteacher = NULL;
//
//	//初始化一个Teacher结构
//	Teacher	teacher;
//	memset(&teacher,0,sizeof(Teacher));
//	strcpy(teacher.name,"wangwu");
//	teacher.age = 32;
//	teacher.p = (char*)malloc(100);
//	memset(teacher.p,0,100);
//	strcpy(teacher.p,"This is Teacher wang!");
//	teacher.len = strlen("This is Teacher wang!");
//
//	// Teacher 编码
//	ret = TeacherEncode(&teacher,&myout,&myoutlen);
//	if(0 != ret)
//	{
//		printf("fun TeacherEncode() err:%d \n",ret);
//		goto End;
//	}
//
//	// Teacher 解码
//	ret = TeacherDecode(myout,myoutlen,&myteacher);
//	if(0 != ret)
//	{
//		printf("func TeacherDecode() err:%d \n",ret);
//		goto End;
//	}
//
//	//校验数据是否正确
//	if(0 == strcmp(teacher.name,myteacher->name) && teacher.age == myteacher->age)
//	{
//		printf("编码和解码的结果 ok\n");
//	}
//	else
//	{
//		printf("编码和解码的结果 err \n");
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