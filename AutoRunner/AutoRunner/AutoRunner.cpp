// AutoRunner.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#include <map>
#include <vector>


#define MAX_BASE_SIZE   0x80
#define MAX_NAME_SIZE   0x100
#define MAX_LINE_SIZE   0x400
#define MAX_DATA_SIZE   0x10000

#define SPLIT_BRACES_LEFT			"{"
#define SPLIT_BRACKET_LEFT			"["
#define SPLIT_SLASH_LR				"/\\"
#define SPLIT_COLON					":"
#define SPLIT_CURVE_LEFT			"("
#define SPLIT_COMMA					","
#define SPLIT_CURVE_RIGHT			")"
#define SPLIT_VERTICALBAR			"|"
#define SPLIT_BRACKET_RIGHT			"]"
#define SPLIT_BRACES_RIGHT			"}"
#define SPLIT_ENTER					"\n"

#define DEFAULT_CONFIG				"default.config"

//置顶
#define EXECUTE_TOPSTICK(hWnd) SetForegroundWindow(hWnd)
//最大化
#define EXECUTE_MAXIMIZE(hWnd) SendMessage(hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0)
//最小化
#define EXECUTE_MINIMIZE(hWnd) SendMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0)
//还原
#define EXECUTE_RESTORE(hWnd) SendMessage(hWnd, WM_SYSCOMMAND, SC_RESTORE, 0)
//关闭
#define EXECUTE_CLOSE(hWnd) SendMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0)

typedef struct tagOperatorPosition {
	size_t sX;
	size_t sY;
} OperatorPosition, *POperatorPosition;

typedef std::vector<OperatorPosition> OPERATORPOSITIONVECTOR;
typedef OPERATORPOSITIONVECTOR::iterator OPERATORPOSITIONVECTORIT;

class CAutoData {
public:
	std::string wcname;//窗口类名称
	size_t sdelay;//休眠间隔(单位毫秒)
	OPERATORPOSITIONVECTOR opvector;//坐标列表

	void clear()
	{
		wcname = ("");
		sdelay = 0;
		opvector.clear();
	}
};
typedef std::vector<CAutoData> AUTODATAVECTOR;
typedef AUTODATAVECTOR::iterator AUTODATAVECTORIT;

typedef std::map<std::string, void *> STRINGVOIDPTRMAP;
typedef STRINGVOIDPTRMAP::iterator STRINGVOIDPTRMAPIT;
typedef STRINGVOIDPTRMAP::value_type STRINGVOIDPTRMAPPAIR;
typedef std::map<std::wstring, void *> WSTRINGVOIDPTRMAP;
typedef WSTRINGVOIDPTRMAP::iterator WSTRINGVOIDPTRMAPIT;
typedef WSTRINGVOIDPTRMAP::value_type WSTRINGVOIDPTRMAPPAIR;

typedef std::vector<std::string> STRINGVECTOR;
typedef std::vector<std::wstring> WSTRINGVECTOR;

//获取指定两个字符串之间的字符串数据
__inline static std::string string_reader(std::string strData,
	std::string strStart, std::string strFinal,
	bool bTakeStart = false, bool bTakeFinal = false)
{
	std::string strTmpData = ("");
	std::string::size_type stStartPos = std::string::npos;
	std::string::size_type stFinalPos = std::string::npos;
	stStartPos = strData.find(strStart);
	if (stStartPos != std::string::npos)
	{
		stFinalPos = strData.find(strFinal, stStartPos + strStart.length());
		if (stFinalPos != std::string::npos)
		{
			if (!bTakeStart)
			{
				stStartPos += strStart.length();
			}
			if (bTakeFinal)
			{
				stFinalPos += strFinal.length();
			}
			strTmpData = strData.substr(stStartPos, stFinalPos - stStartPos);
		}
	}

	return strTmpData;
}
__inline static std::string string_replace_all(std::string &strData, std::string strDst, std::string strSrc)
{
	std::string::size_type stPos = std::string::npos;
	while ((stPos = strData.find(strSrc)) != std::string::npos)
	{
		strData.replace(stPos, strSrc.length(), strDst);
	}

	return strData;
}

__inline static size_t string_split_to_vector(STRINGVECTOR & sv, std::string strData, std::string strSplitter)
{
	std::string strTmp = ("");
	std::string::size_type stIdx = 0;
	std::string::size_type stPos = std::string::npos;
	std::string::size_type stSize = strData.length();

	while ((stPos = strData.find(strSplitter, stIdx)) != std::string::npos)
	{
		strTmp = strData.substr(stIdx, stPos - stIdx);
		if (!strTmp.length())
		{
			strTmp = ("");
		}
		sv.push_back(strTmp);

		stIdx = stPos + strSplitter.length();
	}

	if (stIdx < stSize)
	{
		strTmp = strData.substr(stIdx, stSize - stIdx);
		if (!strTmp.length())
		{
			strTmp = ("");
		}
		sv.push_back(strTmp);
	}

	return sv.size();
}

//获取指定两个字符串之间的字符串数据
__inline static std::wstring wstring_reader(std::wstring wstrData,
	std::wstring wstrStart, std::wstring wstrFinal,
	bool bTakeStart = false, bool bTakeFinal = false)
{
	std::wstring wstrTmpData = (L"");
	std::wstring::size_type stStartPos = std::wstring::npos;
	std::wstring::size_type stFinalPos = std::wstring::npos;
	stStartPos = wstrData.find(wstrStart);
	if (stStartPos != std::wstring::npos)
	{
		stFinalPos = wstrData.find(wstrFinal, stStartPos + wstrStart.length());
		if (stFinalPos != std::wstring::npos)
		{
			if (!bTakeStart)
			{
				stStartPos += wstrStart.length();
			}
			if (bTakeFinal)
			{
				stFinalPos += wstrFinal.length();
			}
			wstrTmpData = wstrData.substr(stStartPos, stFinalPos - stStartPos);
		}
	}

	return wstrTmpData;
}
__inline static std::wstring wstring_replace_all(std::wstring &wstrData, std::wstring wstrDst, std::wstring wstrSrc)
{
	std::wstring::size_type stPos = std::wstring::npos;
	while ((stPos = wstrData.find(wstrSrc)) != std::wstring::npos)
	{
		wstrData.replace(stPos, wstrSrc.length(), wstrDst);
	}

	return wstrData;
}

__inline static size_t wstring_split_to_vector(WSTRINGVECTOR & wsv, std::wstring wstrData, std::wstring wstrSplitter)
{
	std::wstring wstrTmp = (L"");
	std::wstring::size_type stIdx = 0;
	std::wstring::size_type stPos = std::wstring::npos;
	std::wstring::size_type stSize = wstrData.length();

	while ((stPos = wstrData.find(wstrSplitter, stIdx)) != std::wstring::npos)
	{
		wstrTmp = wstrData.substr(stIdx, stPos - stIdx);
		if (!wstrTmp.length())
		{
			wstrTmp = (L"");
		}
		wsv.push_back(wstrTmp);

		stIdx = stPos + wstrSplitter.length();
	}

	if (stIdx < stSize)
	{
		wstrTmp = wstrData.substr(stIdx, stSize - stIdx);
		if (!wstrTmp.length())
		{
			wstrTmp = (L"");
		}
		wsv.push_back(wstrTmp);
	}

	return wsv.size();
}

//整型转换为字符串类型
__inline static std::string STRING_FROM_INT(int v) { CHAR tzV[MAX_BASE_SIZE] = { 0 }; memset(tzV, 0, sizeof(tzV)); snprintf(tzV, sizeof(tzV) / sizeof(CHAR), ("%d"), v); return std::string(tzV); }
__inline static std::string STRING_FROM_LONG(long v) { CHAR tzV[MAX_BASE_SIZE] = { 0 }; memset(tzV, 0, sizeof(tzV)); snprintf(tzV, sizeof(tzV) / sizeof(CHAR), ("%ld"), v); return std::string(tzV); }
__inline static std::string STRING_FROM_FLOART(float v) { CHAR tzV[MAX_BASE_SIZE] = { 0 }; memset(tzV, 0, sizeof(tzV)); snprintf(tzV, sizeof(tzV) / sizeof(CHAR), ("%lf"), v); return std::string(tzV); }
__inline static std::string STRING_FROM_HEX(unsigned long v) { CHAR tzV[MAX_BASE_SIZE] = { 0 }; memset(tzV, 0, sizeof(tzV)); snprintf(tzV, sizeof(tzV) / sizeof(CHAR), ("0x%x"), v); return std::string(tzV); }
__inline static std::wstring WSTRING_FROM_INT(int v) { WCHAR tzV[MAX_BASE_SIZE] = { 0 }; memset(tzV, 0, sizeof(tzV)); _snwprintf(tzV, sizeof(tzV) / sizeof(WCHAR), (L"%d"), v); return std::wstring(tzV); }
__inline static std::wstring WSTRING_FROM_LONG(long v) { WCHAR tzV[MAX_BASE_SIZE] = { 0 }; memset(tzV, 0, sizeof(tzV)); _snwprintf(tzV, sizeof(tzV) / sizeof(WCHAR), (L"%ld"), v); return std::wstring(tzV); }
__inline static std::wstring WSTRING_FROM_FLOART(float v) { WCHAR tzV[MAX_BASE_SIZE] = { 0 }; memset(tzV, 0, sizeof(tzV)); _snwprintf(tzV, sizeof(tzV) / sizeof(WCHAR), (L"%lf"), v); return std::wstring(tzV); }
__inline static std::wstring WSTRING_FROM_HEX(unsigned long v) { WCHAR tzV[MAX_BASE_SIZE] = { 0 }; memset(tzV, 0, sizeof(tzV)); _snwprintf(tzV, sizeof(tzV) / sizeof(WCHAR), (L"0x%x"), v); return std::wstring(tzV); }

#if !defined(UNICODE) && !defined(_UNICODE)
#define TSTRINGVOIDPTRMAP STRINGVOIDPTRMAP
#define TSTRINGVOIDPTRMAPIT STRINGVOIDPTRMAPIT
#define TSTRINGVOIDPTRMAPPAIR STRINGVOIDPTRMAPPAIR
#else
#define TSTRINGVOIDPTRMAP WSTRINGVOIDPTRMAP
#define TSTRINGVOIDPTRMAPIT WSTRINGVOIDPTRMAPIT
#define TSTRINGVOIDPTRMAPPAIR WSTRINGVOIDPTRMAPPAIR
#endif

size_t StringParser(CAutoData & cad, std::string data)
{
	size_t result = 0;
	STRINGVECTOR sv;
	std::string in_data = string_reader(data, SPLIT_BRACES_LEFT, SPLIT_BRACES_RIGHT);
	result = string_split_to_vector(sv, in_data, SPLIT_COLON);
	switch (result)
	{
		case 2:
		{
			STRINGVECTOR svTwo;
			cad.clear();
			result = string_split_to_vector(svTwo, sv.at(0), SPLIT_SLASH_LR);
			switch (result)
			{
				case 2:
				{								
					cad.wcname = svTwo.at(0);
					cad.sdelay = strtoul(svTwo.at(1).c_str(), 0, 10);
					STRINGVECTOR svThree;
					result = string_split_to_vector(svThree, sv.at(1), SPLIT_VERTICALBAR);
					for (size_t sIdx = 0; sIdx < svThree.size(); sIdx++)
					{
						std::string temp = string_reader(svThree.at(sIdx), SPLIT_CURVE_LEFT, SPLIT_CURVE_RIGHT);
						STRINGVECTOR svFour;
						result = string_split_to_vector(svFour, temp, SPLIT_COMMA);
						switch (result)
						{
							case 2:
							{
								OperatorPosition op = { 0 };
								op.sX = strtoul(svFour.at(0).c_str(), 0, 10);
								op.sY = strtoul(svFour.at(1).c_str(), 0, 10);
								cad.opvector.push_back(op);
							}
							break;
						}
					}
					result = cad.opvector.size();
				}
				break;
				default: { result = 0; } break;
			}

		}
		break;
		default: { result = 0; } break;
	}
	return result;
}

size_t InitRead(AUTODATAVECTOR & adv, std::string filename)
{
	size_t result = 0;
	CAutoData cad;
	char cdata[MAX_LINE_SIZE] = ("");
	FILE * pf = fopen(filename.c_str(), "rb");
	if (pf)
	{
		while (fgets(cdata, sizeof(cdata) / sizeof(*cdata), pf))
		{
			cad.clear();
			if(StringParser(cad, cdata))
			{
				adv.push_back(cad);
			}
		}
		fclose(pf);
		pf = 0;
	}

	return result;
}
HWND STRING_WINDOW_HANDLE(STRINGVOIDPTRMAP & svm, std::string wcname)
{
	HWND hWnd = 0;
	STRINGVOIDPTRMAPIT itFinder = svm.find(wcname);
	if (itFinder != svm.end())
	{
		hWnd = (HWND)itFinder->second;
	}
	else
	{
		hWnd = ::FindWindowExA(NULL, NULL, wcname.c_str(), NULL);
		if (IsWindow(hWnd))
		{
			svm.insert(STRINGVOIDPTRMAPPAIR(wcname, hWnd));
		}
	}
	return hWnd;
}
HWND WSTRING_WINDOW_HANDLE(WSTRINGVOIDPTRMAP & svm, std::wstring wcname)
{
	HWND hWnd = 0;
	WSTRINGVOIDPTRMAPIT itFinder = svm.find(wcname);
	if (itFinder != svm.end())
	{
		hWnd = (HWND)itFinder->second;
	}
	else
	{
		hWnd = ::FindWindowExW(NULL, NULL, wcname.c_str(), NULL);
		if (IsWindow(hWnd))
		{
			svm.insert(WSTRINGVOIDPTRMAPPAIR(wcname, hWnd));
		}
	}
	return (HWND)itFinder->second;
}
typedef void(__stdcall*P_CALLBACK_FUNCTION)(size_t);

void __stdcall click_sleep(size_t sDelay)
{
	Sleep(sDelay);
}
void mouse_group_click(AUTODATAVECTOR * pAdv, P_CALLBACK_FUNCTION pcallback)
{
	HWND hWnd = 0;
	RECT rc = { 0 };
	POINT ptOld = { 0 };
	STRINGVOIDPTRMAP svm;
	AUTODATAVECTORIT itIdxAdv;
	AUTODATAVECTORIT itEndAdv;
	OPERATORPOSITIONVECTORIT itIdxOpv;
	OPERATORPOSITIONVECTORIT itEndOpv;

	itIdxAdv = pAdv->begin();
	itEndAdv = pAdv->end();
	for (; itIdxAdv != itEndAdv; itIdxAdv++)
	{
		//if (hWnd)
		//{
			//EXECUTE_MINIMIZE(hWnd);
			//Sleep(GetDoubleClickTime());
		//}
		hWnd = STRING_WINDOW_HANDLE(svm, itIdxAdv->wcname);
		EXECUTE_TOPSTICK(hWnd);
		EXECUTE_MAXIMIZE(hWnd);
		//Sleep(GetDoubleClickTime());

		//获取窗口全局坐标位置
		memset(&rc, 0, sizeof(rc));
		::GetWindowRect(hWnd, &rc);

		//保存当前鼠标指针
		//取得当前鼠标位置
		memset(&ptOld, 0, sizeof(ptOld));
		::GetCursorPos(&ptOld);

		itIdxOpv = itIdxAdv->opvector.begin();
		itEndOpv = itIdxAdv->opvector.end();
		for (; itIdxOpv != itEndOpv; itIdxOpv++)
		{
			//设置鼠标指针位置(x, y)
			::SetCursorPos(rc.left + itIdxOpv->sX, rc.top + itIdxOpv->sY);

			//抬起
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			//按下
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			//抬起
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			//还原
			Sleep(GetDoubleClickTime());
		}

		//还原坐标位置
		//SetCursorPos(ptOld.x, ptOld.y);

		//休眠等待
		pcallback(itIdxAdv->sdelay);
	}	
}

#define TAGS_WECHAT_WINDOW		"WeChatMainWndForPC"
#define TAGS_WECHATWEB_WINDOW	"CefWebViewWnd"

#define CONTACT_FILENAME		"contact.txt"
#define PUBLIC_FILENAME			"public.txt"
#define POSITION_FILENAME		"position.txt"

static HWND G_hWechatWnd = 0;
static HWND G_hWechatWebWnd = 0;

int read_contact_point(POINT &pt, char * pFileName)
{
	int result = 0;
	char szData[MAX_LINE_SIZE] = "";
	FILE * pFile = fopen(pFileName, "rb");
	if (pFile)
	{
		if (fgets(szData, sizeof(szData) / sizeof(*szData), pFile))
		{
			result = sscanf(szData, "[(%ld,%ld)]", &pt.x, &pt.y);
		}
		fclose(pFile);
	}
	return result;
}

int read_public_point(POINT &pt, char * pFileName)
{
	int result = 0;
	char szData[MAX_LINE_SIZE] = "";
	FILE * pFile = fopen(pFileName, "rb");
	if (pFile)
	{
		if(fgets(szData, sizeof(szData) / sizeof(*szData), pFile))
		{
			result = sscanf(szData, "[(%ld,%ld)]", &pt.x, &pt.y);
		}
		fclose(pFile);
	}
	return result;
}

int read_position_point(std::vector<POINT> &rcVector, char * pFileName)
{
	int result = 0;
	char szData[MAX_LINE_SIZE] = "";
	FILE * pFile = fopen(pFileName, "rb");
	if (pFile)
	{
		while (fgets(szData, sizeof(szData) / sizeof(*szData), pFile))
		{
			POINT ptA = { 0 }; POINT ptB = { 0 };
			if (4 == sscanf(szData, "[(%ld,%ld),(%ld,%ld)]", &ptA.x, &ptA.y, &ptB.x, &ptB.y))
			{
				rcVector.push_back(ptA); rcVector.push_back(ptB);
			}
		}
		fclose(pFile);
	}
	return result;
}


HWND GetWindowHandle(HWND &hWnd, _TCHAR * tWindowClassName)
{
	if (!hWnd || !IsWindow(hWnd))
	{
		hWnd = ::FindWindowEx(NULL, NULL, tWindowClassName, NULL);
	}
	return hWnd;
}

void mouse_click(HWND hWnd, POINT ptNew)
{
	RECT rc = { 0 };
	POINT ptOld = { 0 };

	//获取窗口全局坐标位置
	::GetWindowRect(hWnd, &rc);

	//保存当前鼠标指针
	//取得当前鼠标位置
	::GetCursorPos(&ptOld);

	//设置鼠标指针位置(x, y)
	::SetCursorPos(rc.left + ptNew.x, rc.top + ptNew.y);

	//抬起
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	//按下
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	//抬起
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	//还原
	Sleep(GetDoubleClickTime());

	//还原坐标位置
	SetCursorPos(ptOld.x, ptOld.y);
}

void mouse_group_click(HWND hWnd, std::vector<POINT> * ptNewGroupVector)
{
	RECT rc = { 0 };
	POINT ptOld = { 0 };

	//获取窗口全局坐标位置
	::GetWindowRect(hWnd, &rc);

	//保存当前鼠标指针
	//取得当前鼠标位置
	::GetCursorPos(&ptOld);

	for (std::vector<POINT>::iterator it = ptNewGroupVector->begin();
		it != ptNewGroupVector->end();
		it++)
	{
		//设置鼠标指针位置(x, y)
		::SetCursorPos(rc.left + it->x, rc.top + it->y);

		//抬起
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		//按下
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		//抬起
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		//还原
		Sleep(GetDoubleClickTime());
	}

	//还原坐标位置
	SetCursorPos(ptOld.x, ptOld.y);
}

void __stdcall click_sleep()
{
	Sleep(3000);
	G_hWechatWebWnd = GetWindowHandle(G_hWechatWebWnd, _T(TAGS_WECHATWEB_WINDOW));
	if (G_hWechatWebWnd != NULL)
	{
		//最小化
		EXECUTE_MINIMIZE(G_hWechatWebWnd);
	}
	G_hWechatWnd = GetWindowHandle(G_hWechatWnd, _T(TAGS_WECHAT_WINDOW));
	if (G_hWechatWnd != NULL)
	{
		//最大化
		EXECUTE_MAXIMIZE(G_hWechatWnd);
	}
}
void mouse_group_click(HWND hWnd, std::vector<POINT> * ptNewGroupVector, P_CALLBACK_FUNCTION pcallback, const int nCounter)
{
	RECT rc = { 0 };
	POINT ptOld = { 0 };
	int nNumberCounter = 0;

	//获取窗口全局坐标位置
	::GetWindowRect(hWnd, &rc);

	//保存当前鼠标指针
	//取得当前鼠标位置
	::GetCursorPos(&ptOld);

	nNumberCounter = 1;

	for (std::vector<POINT>::iterator it = ptNewGroupVector->begin();
		it != ptNewGroupVector->end();
		it++)
	{
		//设置鼠标指针位置(x, y)
		::SetCursorPos(rc.left + it->x, rc.top + it->y);

		//抬起
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		//按下
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		//抬起
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		//还原
		Sleep(GetDoubleClickTime());

		printf("%d\n", nNumberCounter);
		if (!(nNumberCounter++ % nCounter))
		{
			printf("==%d\n", nNumberCounter);
			pcallback(0);
			nNumberCounter = 1;
		}		
	}

	//还原坐标位置
	SetCursorPos(ptOld.x, ptOld.y);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
//int main(int argc, char ** argv)
{
	int result = 0;
	POINT ptContact = { 0 };
	POINT ptPublic = { 0 };
	std::vector<POINT> ptNewGroupVector;

	AUTODATAVECTOR adv;
	InitRead(adv, DEFAULT_CONFIG);
	mouse_group_click(&adv, click_sleep);
	return 0;

	G_hWechatWnd = GetWindowHandle(G_hWechatWnd, _T(TAGS_WECHAT_WINDOW));
	if (G_hWechatWnd != NULL)
	{
		//最大化
		EXECUTE_MAXIMIZE(G_hWechatWnd);

		//点击通讯录
		read_contact_point(ptContact, CONTACT_FILENAME);
		mouse_click(G_hWechatWnd, ptContact);
		//点击公众号
		read_contact_point(ptPublic, PUBLIC_FILENAME);
		mouse_click(G_hWechatWnd, ptPublic);

		//开始遍历点击公众号
		ptNewGroupVector.clear();
		read_position_point(ptNewGroupVector, POSITION_FILENAME);
		mouse_group_click(G_hWechatWnd, &ptNewGroupVector, click_sleep, 2);
		//最小化
		//SendMessage(G_hWechatWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		//关闭窗口
		//SendMessage(G_hWechatWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
		//还原窗口
		//SendMessage(G_hWechatWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		//滚动到最后
		//SendMessage(G_hWechatWnd, WM_VSCROLL, SB_BOTTOM, 0);
	}
    
	return result;
}

