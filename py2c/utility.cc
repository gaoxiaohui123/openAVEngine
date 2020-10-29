#include <time.h>

#include <queue>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <stdio.h> 
#include <windows.h> 
#pragma comment(lib, "ws2_32.lib") 
#include <winsock2.h>
typedef int socklen_t;
typedef SOCKET NativeSocket;
#else
#include <stdio.h>  
#include <string.h>  
#include <arpa/inet.h>  
#include <netdb.h>  
#include <stdlib.h>  
#include <unistd.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
//#include <stdint.h>

#define closesocket close
typedef int NativeSocket;

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET  static_cast<NativeSocket>(-1)
#endif
#endif
#include "webrtc/base/helpers.h"
#include "webrtc/base/basictypes.h"
#include "webrtc/system_wrappers/interface/tick_util.h"
#include "webrtc/base/common.h"
#include "webrtc/base/utility.h"
#include "openssl/ssl.h"
#include "webrtc/avengine/source/gxhlog.h"
//edited by gxh
#if defined(WEBRTC_IOS) || defined(IOS)
#include "webrtc/modules/av_coding/codecs/ffmpeg/main/interface/file_ios.h"
#endif

unsigned char GlobPubKey[kECDHSize << 4] = "";
void *GlobECKeyHnd = NULL;
//char GlobSturnServerAddr[2][kAddrSize] = {"172.16.226.143:19302" , "192.168.1.109:19302"};
char GlobSturnServerAddr[2][kAddrSize] = { "10.155.11.3:19302", "10.155.11.9:19302" };
//const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";

void handleErrors()
{
	printf("Error occurred.\n");
}
void disp(const char *str, const void *pbuf, const int size)
{
	int i = 0;
	if (str != NULL){
		printf("%s:\n", str);
	}
	if (pbuf != NULL && size > 0){
		for (i = 0; i<size; i++)
			printf("%02x", *((unsigned char *)pbuf + i));
		putchar('\n');
	}
	putchar('\n');
}
#ifdef WEBRTC_LINUX

#else
static EC_KEY *genECDHtemppubkey(unsigned char *pubkey)
{
	int len;
	EC_KEY *ecdh = EC_KEY_new();

	//Generate Public  
	ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);//NID_secp521r1  
	EC_KEY_generate_key(ecdh);
	const EC_POINT *point = EC_KEY_get0_public_key(ecdh);
	const EC_GROUP *group = EC_KEY_get0_group(ecdh);

	//unsigned char *pubkey = malloc(kECDHSize); 
	len = EC_POINT_point2oct(group, point, POINT_CONVERSION_COMPRESSED, pubkey, kECDHSize, NULL);
	if (len != kECDHSize)
	{
		//handleErrors();
		EC_KEY_free(ecdh);
		return NULL;
	}
	printf("len=%d\n", len);
	disp("pubkey", pubkey, kECDHSize - 1);
	//return pubkey;  
	return ecdh;
}

static unsigned char *genECDHsharedsecret(unsigned char *shared, EC_KEY *ecdh, unsigned char *peerkey, size_t secret_len)
{
	int len;
	if (shared == NULL)
	{
		shared = (unsigned char *)malloc(kECDHSize);
	}
	const EC_GROUP *group = EC_KEY_get0_group(ecdh);

	//ComputeKey  
	EC_POINT *point_peer = EC_POINT_new(group);
	EC_POINT_oct2point(group, point_peer, peerkey, kECDHSize, NULL);

	//if (0 != EC_POINT_cmp(group, point2, point2c, NULL)) handleErrors();  
	len = ECDH_compute_key(shared, secret_len, point_peer, ecdh, NULL);
	if (len != (kECDHSize - 1))
	{
		//handleErrors();
		return NULL;
	}
	printf("len=%d\n", len);
	disp("shared", shared, secret_len);

	return shared;
}
#endif
int base64_encode(unsigned char * bindata, char * base64, int binlength)
{
	int i, j;
	unsigned char current;

	for (i = 0, j = 0; i < binlength; i += 3)
	{
		current = (bindata[i] >> 2);
		current &= (unsigned char)0x3F;
		base64[j++] = base64char[(int)current];

		current = ((unsigned char)(bindata[i] << 4)) & ((unsigned char)0x30);
		if (i + 1 >= binlength)
		{
			base64[j++] = base64char[(int)current];
			base64[j++] = '=';
			base64[j++] = '=';
			break;
		}
		current |= ((unsigned char)(bindata[i + 1] >> 4)) & ((unsigned char)0x0F);
		base64[j++] = base64char[(int)current];

		current = ((unsigned char)(bindata[i + 1] << 2)) & ((unsigned char)0x3C);
		if (i + 2 >= binlength)
		{
			base64[j++] = base64char[(int)current];
			base64[j++] = '=';
			break;
		}
		current |= ((unsigned char)(bindata[i + 2] >> 6)) & ((unsigned char)0x03);
		base64[j++] = base64char[(int)current];

		current = ((unsigned char)bindata[i + 2]) & ((unsigned char)0x3F);
		base64[j++] = base64char[(int)current];
	}
	base64[j] = '\0';
	return strlen(base64);
}

int base64_decode(char * base64, unsigned char * bindata)
{
	int i, j;
	unsigned char k;
	unsigned char temp[4];
	for (i = 0, j = 0; base64[i] != '\0'; i += 4)
	{
		memset(temp, 0xFF, sizeof(temp));
		for (k = 0; k < 64; k++)
		{
			if (base64char[k] == base64[i])
				temp[0] = k;
		}
		for (k = 0; k < 64; k++)
		{
			if (base64char[k] == base64[i + 1])
				temp[1] = k;
		}
		for (k = 0; k < 64; k++)
		{
			if (base64char[k] == base64[i + 2])
				temp[2] = k;
		}
		for (k = 0; k < 64; k++)
		{
			if (base64char[k] == base64[i + 3])
				temp[3] = k;
		}

		bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2)) & 0xFC)) |
			((unsigned char)((unsigned char)(temp[1] >> 4) & 0x03));
		if (base64[i + 2] == '=')
			break;

		bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4)) & 0xF0)) |
			((unsigned char)((unsigned char)(temp[2] >> 2) & 0x0F));
		if (base64[i + 3] == '=')
			break;

		bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6)) & 0xF0)) |
			((unsigned char)(temp[3] & 0x3F));
	}
	return j;
}
int char2uchar(char *src, unsigned char *dst, int len)
{
	for (int i = 0; i < len; i++)
	{
		sscanf(&src[i << 1], "%02x", &dst[i]);

		/*char value[3] = "";
		strncpy(value, &src[i << 1], 2);
		sscanf(value, "%02x", &dst[i]);*/
	}
	//totlelen = len + 3;
	return len;
}
int uchar2char(unsigned char *src, char *dst, int len)
{
	for (int i = 0; i < len; i++)
	{
		//sprintf(&dst[i], "%x", src[i]);
		sprintf(dst + (i << 1), "%02x", src[i]);//格式化输出到buffer,每个unsigned char 转换为字符后占两个位置，%x小写输出，%X大写输出
	}

	return len << 1;
}
//微增变换：
//统计字符概率
//未出现字符替换'\0'字符
//二进制中出现'\0'会导致字符串断开，算法主要是解决如何处理'\0'字符的问题；
//如果二进制数据块中出现'\0'字符即二进制的0，则用未出现过的字符来替代；
//并将未出现过字符存储在数据块的最后一字节；
//数据按最大254个进行分割，输出是255个
//254个数据，必定会存在至少有一个除了0之外的字符是未出现过的；
//这样产生的荣誉数据仅为1/255
int uchar2char256(unsigned char *src, char *dst, int len, int isForce)
{
	
	int maxlen = 253;// 254;
	int ret = len;
	uint8_t key0 = '\0';
	uint8_t key1 = '\n';
	uint8_t signature = 0;
	uint8_t signature1 = 0;
	
	//uint8_t replacedCode = '\0';//0
	if (isForce || len > 255)
	{
		int i = 0, k = 0;
		for (i = 0; i < len; i += maxlen, k+=2)
		{
			uint8_t histgram[256] = "";
			int count = 0;
			int j = 0;
			for (j = 0; j < maxlen && (i + j) < len; j++)
			{
				histgram[src[i + j]]++;
			}
			for (j = 1; j < 256; j++)
			{
				if (j == key0 || j == key1)
				{
					continue;
				}
				if (!histgram[j])
				{
					if (count == 0)
					{
						signature = j;
					}
					else
					{
						signature1 = j;
						break;
					}
					count++;
						
				}
			}
			for (j = 0; j < maxlen && (i + j) < len; j++)
			{
				if (src[i + j] == key0)
				{
					dst[i + k + j] = signature;
				}
				else if (src[i + j] == key1)
				{
					dst[i + k + j] = signature1;
				}
				else
				{
					dst[i + k + j] = src[i + j];
				}
			}
			dst[i + k + j] = signature;
			dst[i + (k+1) + j] = signature1;
		}
		ret = len + k;
	}
	/*else
	{
		int zero = 0;
		uint8_t histgram[256] = "";
		int j = 0;
		for (j = 0; j < len; j++)
		{
			histgram[src[j]]++;
			zero |= src[j] == 0;
		}
		if (zero)
		{
			for (j = 1; j < 256; j++)
			{
				if (!histgram[j])
				{
					signature = j;
					break;
				}
			}
			for (j = 0; j < len; j++)
			{
				if (!src[j])
				{
					dst[j] = signature;
				}
				else
				{
					dst[j] = src[j];
				}
			}
			dst[j] = signature;
			ret = len + 1;
		}
	}*/
	return ret;
}
int char2uchar256(char *src, unsigned char *dst, int refLen)
{
	int ret = strlen(src);
	int maxlen = 255;// 255;// 254;
	int len = strlen(src);
	int i = 0, k = 0;
	uint8_t key0 = '\0';
	uint8_t key1 = '\n';
	if (ret == refLen)
	{
		return ret;
	}
	else
	{
		unsigned char *p = (unsigned char *)src;
		for (i = 0; i < len; i += maxlen, k+=2)
		{
			uint8_t signature = (i + maxlen) < len ? p[i + maxlen - 2] : p[len - 2];
			uint8_t signature1 = (i + maxlen) < len ? p[i + maxlen - 1] : p[len - 1];
			int j = 0;
			for (j = 0; j < (maxlen - 2) && (i + j) < len; j++)
			{
				if (p[i + j] == signature)
				{
					dst[i + j - k] = key0;
				}
				else if (p[i + j] == signature1)
				{
					dst[i + j - k] = key1;
				}
				else
				{
					dst[i + j - k] = p[i + j];
				}
			}
		}
		ret = len - k;
	}
	return ret;
}
/*
1/2：A ^ B = C ==> A ^ C = B; B ^ C = A
1/3：(A ^ B) ^ D = C ==> (A ^ B) = D ^ C; D = (A ^ B) ^ C; A = B ^ (A ^ B); B = A ^ (A ^ B)
依次类推到1/N
若A < B, B扩展零；
*/
int fecEnc(unsigned char *src0, unsigned char *src1, int size0, int size1, unsigned char *dst)
{
	int ret = 0;
//	unsigned char uZero = 0;
	int len = size0 > size1 ? size0 : size1;

	for (int i = 0; i < size0; i++)
	{
		dst[i] = src0[i] ^ src1[i];
	}
	ret = len;
	return ret;
}

//获取字符后的字符串
static char *mystrcat(char *szPath, char tok)
{
	char* pPos = NULL;
	pPos = strrchr(szPath, tok);//'__');//查找一个字符在另一个字符串中"最后一次"出现的位置
	if (pPos == NULL)
		return NULL;
	*pPos = '\0';
	return ++pPos;
}
//获取字符前的字符串
static char *mystrcat2(char *szPath, char tok)
{
	char* pPos = NULL;
	pPos = strrchr(szPath, tok);//'__');//查找一个字符在另一个字符串中"最后一次"出现的位置
	if (pPos == NULL)
		return NULL;//szPath;
	*pPos = '\0';
	return szPath;
}
static int replaceChar(char *szPath, int num, char srcTok, char dstTok)
{
	int ret = 0;
	int i = 0;
//	int flag = 0;
	char* pPos = szPath;
	do
	{
		pPos = strrchr(pPos, srcTok);
		i = (int)(pPos - szPath);
		if (i >= num && num > 0)
		{
			break;
		}
		if (pPos)
		{
			*pPos = dstTok;
		}
		else
		{
			break;
		}
		ret = i;
		//flag = (i < num) || (!num);
	} while (1);

	return ret;
}
static int getStr(char *szPath, char *out, int idx, char tok)
{
	int i = 0;
	char *in = szPath;//mystrcat(szPath, '#');
	char *tmp = NULL;
	//int flag = 0;
	do
	{
		tmp = mystrcat(in, tok);
		if (tmp && i == idx)
		{
			strcpy(out, tmp);
			break;
		}
		else if (in && i == idx)
		{
			strcpy(out, in);
			break;
		}
		i++;
	} while (tmp);

	return i;
}
int ipv4toUint64(unsigned long long *iaddr, char *caddr)
{
	int ret = 0;
	char* pPos = caddr;
	char* pHead = NULL;
	uint64_t iDst = 0;
	int i = 0;
	do
	{
		char cValue[8] = "";
		pHead = pPos;
		pPos = strchr(pPos, '.');
		if (pPos == NULL)
		{
			pPos = caddr;
			pPos = strchr(pPos, ':');
			strncpy(cValue, pHead, (int)((uint64_t)pPos - (uint64_t)pHead));
			uint64_t iValue = (uint64_t)atoi(cValue);
			iValue <<= (i << 3);
			iDst |= iValue;// atoi(cValue) << (i << 3);
			pPos++;	i++;
			strcpy(cValue, pPos);
			iValue = (uint64_t)atoi(cValue);
			iValue <<= (i << 3);
			iDst |= iValue;//(uint64_t)atoi(cValue) << (i << 3);
			break;
		}
		strncpy(cValue, pHead, (int)((uint64_t)pPos - (uint64_t)pHead));
		iDst |= atoi(cValue) << (i << 3);
		pPos++;	i++;
	} while (pPos);
	*iaddr = iDst;
	return ret;
}
int uint64toIpv4(unsigned long long *iaddr, char *caddr)
{
	int ret = 0;
    LogOut("uint64toIpv4: 00 \n", NULL);
	//uint64_t iDst = *iaddr;
    unsigned long long iDst = 0;
    memcpy(&iDst, iaddr, kIPV4EncryptSize);
    LogOut("uint64toIpv4: 01 \n", NULL);
	unsigned short value = 0;
	int i = 0;
	do
	{
		char cValue[8] = "";
		if (i < 3)
		{
            LogOut("uint64toIpv4: 0 \n", NULL);
			value = (iDst >> (i << 3)) & 0xFF;	i++;
			sprintf(cValue, "%d", value);
			strcat(caddr, cValue);	strcat(caddr, ".");
		}
		else if (i == 3)
		{
            LogOut("uint64toIpv4: 1 \n", NULL);
			value = (iDst >> (i << 3)) & 0xFF;	i++;
			sprintf(cValue, "%d", value);
			strcat(caddr, cValue);	strcat(caddr, ":");
		}
		else
		{
            LogOut("uint64toIpv4: 2 \n", NULL);
			value = (iDst >> (i << 3)) & 0xFFFF;	i++;
			sprintf(cValue, "%d", value);
			strcat(caddr, cValue);
		}
        LogOut("uint64toIpv4: 3 \n", NULL);
	} while (i < 5);
	ret = strlen(caddr);
    LogOut("uint64toIpv4: ok \n", NULL);
	return ret;
}
int secrete_encoder(unsigned char *src, char *dst, unsigned char * sharekey, int offset, int len)
{
	int ret = 0;
	for (int i = offset, j = 0; i < len; i++)
	{
		src[j] = src[j] ^ sharekey[i];
		j++;
	}
	ret = uchar2char(src, dst, len);
	return ret;
}
int secrete_decode(char *src, unsigned char *dst, unsigned char * sharekey, int offset, int len)
{
	int ret = 0;
	ret = char2uchar(src, dst, len);
	//uint32_t uId = (uint32_t)my_uid_;
	//unsigned int secretCode = GetScreteCode(0, sizeof(unsigned int));
	for (int i = offset, j = 0; i < len; i++)
	{
		dst[j] = dst[j] ^ sharekey[i];
		j++;
	}
	return ret;
}
std::string GetEnvVarOrDefault(const char* env_var_name,
	const char* default_value) {
	std::string value;
	const char* env_var = getenv(env_var_name);
	if (env_var)
		value = env_var;

	if (value.empty())
		value = default_value;

	return value;
}
std::string GetPeerName() 
{
	char computer_name[256];
	if (gethostname(computer_name, ARRAY_SIZE(computer_name)) != 0)
		strcpy(computer_name, "host");
	std::string ret(GetEnvVarOrDefault("USERNAME", "user"));
	ret += '@';
	ret += computer_name;
	return ret;
}
int IGetPeerName(char *name)
{
	std::string ret = GetPeerName();
	strcpy(name, (char *)ret.c_str());
	int size = strlen(name);
	return size;
}
/*
int get_remote_addr(SOCKET socket, char *szPeerAddress)
{
	sockaddr_storage addr_storage = { 0 };
	socklen_t addrlen = sizeof(addr_storage);
	sockaddr* addr = reinterpret_cast<sockaddr*>(&addr_storage);
	int result = ::getpeername(socket, addr, &addrlen);
	if (result >= 0) {
		//Sets buffers to a specified character.  
		memset((void *)szPeerAddress, 0, sizeof(szPeerAddress));
		//If no error occurs, inet_ntoa returns a character pointer to a static buffer   
		//containing the text address in standard ".'' notation  
		//strcpy(szPeerAddress, inet_ntoa((*(SOCKADDR *)addr).sin_addr));
		if (addr_storage.ss_family == AF_INET)
		{
			const sockaddr_in* saddr = reinterpret_cast<const sockaddr_in*>(&addr_storage);
			strcpy(szPeerAddress, inet_ntoa(saddr->sin_addr));
			strcat(szPeerAddress, ":");
			//itoa(saddr->sin_port, &szPeerAddress[strlen(szPeerAddress)], 10);
            sprintf(&szPeerAddress[strlen(szPeerAddress)], "%d", saddr->sin_port);

		}
		else if (addr_storage.ss_family == AF_INET6)
		{
			//const sockaddr_in6* saddr = reinterpret_cast<const sockaddr_in6*>(&addr_storage);

		}
	}
	return strlen(szPeerAddress);
}
 */
#ifdef _WIN32

#else

int get_ip_from_host(char *ipbuf, const char *host, int maxlen)
{
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	if (inet_aton(host, &sa.sin_addr) == 0)
	{
		struct hostent *he;
		he = gethostbyname(host);
		if (he == NULL)
			return -1;
		memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
	}
	strncpy(ipbuf, inet_ntoa(sa.sin_addr), maxlen);
	return 0;
}
#endif
void *IECDHEncode(unsigned char *pubkey)
{
#ifdef WEBRTC_LINUX
    return NULL;
#else
	EC_KEY *ret = genECDHtemppubkey(pubkey);
	return (void *)ret;
#endif
}
unsigned char *IECDHDecode(unsigned char *shared, void *ecdh, unsigned char *peerkey, size_t secret_len)
{
#ifdef WEBRTC_LINUX
    return NULL;
#else
	unsigned char *ret = genECDHsharedsecret(shared, (EC_KEY *)ecdh, peerkey, secret_len);
	return ret;
#endif
}
int IGetRemoteAddr(void *pSocket, char *szPeerAddress)
{
	///SOCKET socket = (SOCKET)pSocket;
    return 0;///get_remote_addr(socket, szPeerAddress);
}
char *IGetTokBackStr(char *szPath, char tok)
{
	return mystrcat(szPath, tok);
}
char *IGetTokFrontStr(char *szPath, char tok)
{
	return mystrcat2(szPath, tok);
}
int IGetStr(char *szPath, char *out, int idx, char tok)
{
	return getStr(szPath, out, idx, tok);
}
/*
char testc[32] = "1234 5678";
IReplaceChar((char *)testc, kStunTransactionIdLength, '\0', 'x');
IReplaceChar((char *)testc, kStunTransactionIdLength, ' ', 'x');
IReplaceChar((char *)uStunRequest, kStunTransactionIdLength, '\0', 'x');
*/
int IReplaceChar(char *szPath, int num, char srcTok, char dstTok)
{
	return replaceChar(szPath, num, srcTok, dstTok);
}
uint32 IRandId(int flag)
{
	uint32 ret = 0;
	if (!flag)
	{
		ret = (uint32)(rtc::CreateRandomId() & 0x7FFFFFFF);
	}
	else
	{
		ret = rtc::CreateRandomId();
	}
	return ret;
}
unsigned long long IRandId64(int flag)
{
	unsigned long long ret = (unsigned long long)rtc::CreateRandomId64();
	return ret;
}
int IRandUChar(unsigned char *buf, int len)
{
#if 0
	int size = len << 1;
	std::string str = rtc::CreateRandomString(size);
	int ret = char2uchar((char *)str.c_str(), buf, size);
#else
	int ret = 0;

#endif
	return ret;
}
void IRandString(char *buf, int len)
{
	std::string ret = rtc::CreateRandomString(len);
	strcpy(buf, (char *)ret.c_str());
}
int ICreateHMAC(unsigned char *dst, char *caddr, unsigned char *rndId, int id)
{
	int ret = 0;
	unsigned long long iaddr = 0;
	ret = ipv4toUint64(&iaddr, caddr);
	memcpy(dst, &id, sizeof(int));//id(4)
	memcpy(&dst[4], rndId, 8);
	memcpy(&dst[12], &iaddr, kIPV4EncryptSize);//12+6 = 18;
	ret = kHMACSize;
	return ret;
}
int IUnCreateHMAC(unsigned char *src, char *caddr, unsigned char *rndId, int *id)
{
	int ret = 0;
	unsigned long long iaddr = 0;
	int offset = 0;
	memcpy(id, &src[offset], sizeof(int));	offset += sizeof(int); //id(4)
	memcpy(rndId, &src[offset], 8);			offset += 8;
	memcpy(&iaddr, &src[offset], kIPV4EncryptSize);		offset += kIPV4EncryptSize;//12+6 = 18;
	ret = uint64toIpv4(&iaddr, caddr);
	ret = offset;// kHMACSize;
	return ret;
}
int IHMACEncrypt(unsigned char *dst, char *caddr, unsigned char *rndId, unsigned char *shr_key, int id)
{
	//int ret = 0;
	//uint64_t iaddr = 0;
	//ret = ipv4toUint64(&iaddr, caddr);
	//memcpy(dst, &id, sizeof(int));//id(4)
	//memcpy(&dst[4], rndId, 8);
	//memcpy(&dst[12], &iaddr, 6);//12+6 = 18;
	ICreateHMAC(dst, caddr, rndId, id);
	int offset = 0;
	int len = kHMACSize;
	for (int i = offset, j = 0; i < len; i++)
	{
		dst[j] = dst[j] ^ shr_key[i];
		j++;
	}

	return len;
}
int IHMACDecrypt(unsigned char *src, char *caddr, unsigned char *rndId, unsigned char *shr_key, int *id, int len = kHMACSize)
{
	int ret = 0;
	int offset = 0;
	for (int i = offset, j = 0; i < len; i++)
	{
		src[j] = src[j] ^ shr_key[i];
		j++;
	}
	//uint64_t iaddr = 0;
	unsigned char iaddr[sizeof(uint64_t)+4] = "";
	memcpy(id, src, sizeof(int));//id(4)
	memcpy(rndId, &src[4], 8);
	memcpy(iaddr, &src[12], kIPV4EncryptSize);//12+6 = 18;
	if (strcmp(caddr, ""))
	{
		return -1;
	}
	ret = uint64toIpv4((unsigned long long *)iaddr, caddr);

	return ret;
}
long long IGetTime()
{
	long long time = webrtc::TickTime::MillisecondTimestamp();
	return time;
}
void *IConfigRead(void *fp, char *configPath0, char * text)
{
	FILE *g_config = (FILE *)fp;
	char configPath[256] = "";
	
#ifdef _WIN32
    char* pPos = NULL;
	GetModuleFileNameA(NULL, configPath, sizeof(configPath));
	pPos = strrchr(configPath, '\\');
	*++pPos = '\0';
	//strcat(configPath, "\\log\\");
	strcat(configPath, configPath0);
#elif defined(__ANDROID__)
    strcpy(configPath, "/sdcard/");
    strcat(configPath, configPath0);
#elif defined(WEBRTC_IOS) || defined(IOS)
    char *pName = MAKE_FILE_NAME(configPath0);//makePreferencesFilename("webrtc_log.txt");
    strcpy(configPath, pName);
    free(pName);
#else
    strcpy(configPath, configPath0);
#endif
	if (!g_config)
	{
		//char configPath[128] = "MonitorConfig.txt";
		g_config = fopen(configPath, "rb+");
	}
	if (g_config)
	{
		fseek(g_config, 0, SEEK_END);
		int size = ftell(g_config);
		rewind(g_config);
		fread(text, sizeof(char), size, g_config);
		text[size] = '\0';
#if 0
		char cDevNum[16] = "";
		itoa(ret, cDevNum, 10);
		//
		char cStreamNumber[16] = "";
		int m = sscanf(text, "%s%s%s%s%d%d", cStreamNumber, cPlayer, cDevNum, cServerIP, &iWidth, &iHeight);
		itoa(ret, cDevNum, 10);
		char text2[256] = "";
		sprintf(text2, "%s %s %s %s %d %d ", cStreamNumber, cPlayer, cDevNum, cServerIP, iWidth, iHeight);
		//
		rewind(g_config);
		fprintf(g_config, "%s", text2);//fflush(g_config);
#endif
		//fclose(g_config);
	}
	return (void *)g_config;
}
void *IConfigWrite(void *fp, char *configPath0, char * text)
{
	FILE *g_config = (FILE *)fp;
    char configPath[256] = "";
	
#ifdef _WIN32
    char* pPos = NULL;
	GetModuleFileNameA(NULL, configPath, sizeof(configPath));
	pPos = strrchr(configPath, '\\');
	*++pPos = '\0';
	//strcat(configPath, "\\log\\");
	strcat(configPath, configPath0);
#elif defined(__ANDROID__)
    strcpy(configPath, "/sdcard/");
    strcat(configPath, configPath0);
#elif defined(WEBRTC_IOS) || defined(IOS)
    char *pName = MAKE_FILE_NAME(configPath0);//makePreferencesFilename("webrtc_log.txt");
    strcpy(configPath, pName);
    free(pName);
#elif defined(WEBRTC_LINUX)
    printf("gxhtest: IConfigWrite: linux version \n");
    strcpy(configPath, configPath0);
#elif defined(WEBRTC_MAC)
    ///printf("gxhtest: IConfigWrite: mac version \n");
    strcpy(configPath, configPath0);
#else
    printf("gxhtest: IConfigWrite: ? version \n");
    strcpy(configPath, configPath0);
#endif
	if (!g_config)
	{
		//char configPath[128] = "MonitorConfig.txt";//ab+
		g_config = fopen(configPath, "rb+");
		if (!g_config)
		{
			g_config = fopen(configPath, "ab+");
			//
			if (g_config)
			{
				fclose(g_config);
				g_config = fopen(configPath, "rb+");
			}
		}
	}
	if (g_config)
	{
		fprintf(g_config, "%s", text);	fflush(g_config);
		//fclose(g_config);
	}
	return (void *)g_config;
}
void IRWConfig(char *text, char *key, int *value, char *cValue, int RWFlag)
{
	if (!RWFlag)
	{
		std::string strText(text);
		int found = strText.find(key);
		size_t begin = found + strlen(key);
		
		if (cValue)
		{
			//int m = 
			//sscanf(text, "%s%s", key, cValue);
			sscanf(&text[begin], "%s", cValue);
		}
		else
		{
			//int m = 
			sscanf(&text[begin], "%d", value);
		}
	}
	else
	{
		if (cValue)
		{
			sprintf(text, "%s %s ", key, cValue);
		}
		else
		{
			sprintf(text, "%s %d ", key, *value);
		}
	}
}
