
#include "stdafx.h"

#include <stdio.h>
#include <wchar.h>
#include "NcrAddIn.h"
#include <string>
#include <fstream>

#define TIME_LEN 34

#define BASE_ERRNO     7

/*
��������:
  ������
  ���������
  �����������������������
  ������������������������������
  ������
  ����������
  ���������������
  ����
  ��������
  ��������
  ��������
  ���������
  ������������������
  ��������������������
  ������������������
  ������������������������
  �����������������������
  ������������
  �����������
  ���������������
  ������������
  ���������

������:
  ������������������()
  �����������������()
  �����������(�����, �����, �����, �����, �����, �����)
  ��������()
  ����������������(�����, �����, ������, �����)
  �������������()
  �����������()
*/

static Aliases g_PropNames[] = {
    {L"������",                         L"Version"},
    {L"���������",                      L"Result"},
    {L"�����������������������",        L"CurrentDeviceNumber"},
    {L"������������������������������", L"CurrentDeviceName"},
    {L"������",                         L"Model"},
    {L"����������",                     L"DataBits"},
    {L"���������������",                L"LoadFonts"},
    {L"����",                           L"Port"},
    {L"��������",                       L"Speed"},
    {L"��������",                       L"StopBits"},
    {L"��������",                       L"Parity"},
    {L"���������",                      L"CodePage"},
    {L"������������������",             L"ResultDescription"},
    {L"��������������������",           L"DeviceColumnCount"},
    {L"������������������",             L"DeviceTurnedOn"},
    {L"������������������������",       L"MarqueeRepeatDelay"},
    {L"�����������������������",        L"MarqueeShowDelay"},
    {L"������������",                   L"MarqueeType"},
    {L"�����������",                    L"CurrentWindow"},
    {L"���������������",                L"MarqueeFormat"},
    {L"���������",                      L"WindowCount"},
    {{0}, {0}}
};

static Aliases g_MethodNames[] = {
    {L"������������������",       L"AddDevice"},
    {L"�����������������",        L"DeleteDevice"},
    {L"�����������",              L"CreateWindow"},
    {L"��������",                 L"Clear"},
    {L"����������������",         L"ShowTextPos"},
    {L"�������������",            L"ClearText"},
    {L"�����������",              L"DeleteWindow"},
	{L"�������������",            L"SendByte"},
    {{0}, {0}}
};

static const wchar_t g_kClassNames[] = L"CAddInNCR5976"; //"|OtherClass1|OtherClass2";

uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len = 0);
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
uint32_t getLenShortWcharStr(const WCHAR_T* Source);

//++ dmpas::debug
namespace Debug {
    static bool first = true;
    static bool enabled = true;

    static void nolog(bool disable = true) { enabled = !disable; }

    void log(const char *text)
    {
        if (!enabled)
            return;

        std::ofstream out;
        out.open("c:/temp/ncr.txt", std::ios_base::app);
        if (first)
            out << "----" << std::endl;

        out << text << std::endl;
        first = false;
    }
    void log(const WCHAR_T *text)
    {
        if (!enabled)
            return;

        wchar_t *sh = 0;

        nolog();
        ::convFromShortWchar(&sh, text, 0);
        nolog(false);

        std::wofstream out;
        out.open("c:/temp/ncr.txt", std::ios_base::app);
        if (first)
            out << L"----" << std::endl;

        out << text << std::endl;
        first = false;
    }
}
//-- dmpas::debug

//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
    Debug::log("GetClassObject");
    Debug::log(wsName);
    if(!*pInterface)
    {
        *pInterface= new CAddInNCR5976;
        if (*pInterface) {
            Debug::log("GetClassObject.Done:OK");
            return 1;
        }
    }
    Debug::log("GetClassObject.Done:FAIL");
    return 0;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
    Debug::log("DestroyObject");
    if(!*pIntf)
        return -1;

    delete *pIntf;
    *pIntf = 0;
    return 0;
}
//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
    Debug::log("GetClassNames");
    static WCHAR_T* names = 0;
    if (!names)
        ::convToShortWchar(&names, g_kClassNames);
    return names;
}

// CAddInNCR5976
//---------------------------------------------------------------------------//
CAddInNCR5976::CAddInNCR5976()
{
    Debug::log("CAddInNCR5976::CAddInNCR5976");
    m_iMemory = 0;
    m_iConnect = 0;
}
//---------------------------------------------------------------------------//
CAddInNCR5976::~CAddInNCR5976()
{
    Debug::log("CAddInNCR5976::~CAddInNCR5976");
}
//---------------------------------------------------------------------------//
bool CAddInNCR5976::Init(void* pConnection)
{
    Debug::log("CAddInNCR5976::Init");
    m_iConnect = (IAddInDefBase*)pConnection;
    return m_iConnect != NULL;
}
//---------------------------------------------------------------------------//
long CAddInNCR5976::GetInfo()
{
    Debug::log("CAddInNCR5976::GetInfo");
    // Component should put supported component technology version
    // This component supports 2.0 version
    return 2000;
}
//---------------------------------------------------------------------------//
void CAddInNCR5976::Done()
{
    Debug::log("CAddInNCR5976::Done");
}
/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool CAddInNCR5976::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{
    Debug::log("CAddInNCR5976::RegisterExtensionAs");
    //static wchar_t wsExtension[] = L"AddInNativeExtension";
    static wchar_t wsExtension[] = L"NCR5976";
    int iActualSize = ::wcslen(wsExtension) + 1;
    WCHAR_T* dest = 0;

    if (m_iMemory)
    {
        if(m_iMemory->AllocMemory((void**)wsExtensionName, iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(wsExtensionName, wsExtension, iActualSize);
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------//
long CAddInNCR5976::GetNProps()
{
    Debug::log("CAddInNCR5976::GetNProps");
    // You may delete next lines and add your own implementation code here
    return eProp_Last;
}
//---------------------------------------------------------------------------//
long CAddInNCR5976::FindProp(const WCHAR_T* wsPropName)
{
    Debug::log("CAddInNCR5976::FindProp");

    long plPropNum = -1;
    wchar_t* propName = 0;

    ::convFromShortWchar(&propName, wsPropName);
    plPropNum = findName(g_PropNames, propName);

    delete[] propName;

    return plPropNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNCR5976::GetPropName(long lPropNum, long lPropAlias)
{
    Debug::log("CAddInNCR5976::GetPropName");

    if (lPropNum >= eProp_Last)
        return NULL;

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsPropName = NULL;
    int iActualSize = 0;

    switch(lPropAlias)
    {
    case 0: // First language
        wsCurrentName = g_PropNames[lPropNum].ru;
        break;
    case 1: // Second language
        wsCurrentName = g_PropNames[lPropNum].name;
        break;
    default:
        return 0;
    }

    iActualSize = wcslen(wsCurrentName)+1;

    if (m_iMemory && wsCurrentName)
    {
        if (m_iMemory->AllocMemory((void**)&wsPropName, iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(&wsPropName, wsCurrentName, iActualSize);
    }

    return wsPropName;
}
//---------------------------------------------------------------------------//
bool CAddInNCR5976::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{
    Debug::log("CAddInNCR5976::GetPropVal");
    switch(lPropNum)
    {
    case eProp_Version:
        TV_VT(pvarPropVal) = VTYPE_I4;
        pvarPropVal->lVal = NCR_INT_API;
        break;
    case eProp_Result:
    case eProp_CurrentDeviceNumber:
    case eProp_CurrentDeviceName:
    case eProp_Model:
    case eProp_DataBits:
    case eProp_LoadFonts:
    case eProp_Port:
    case eProp_Speed:
    case eProp_StopBits:
    case eProp_Parity:
    case eProp_CodePage:
    case eProp_ResultDescription:
    case eProp_DeviceColumnCount:
    case eProp_DeviceTurnedOn:
    case eProp_MarqueeRepeatDelay:
    case eProp_MarqueeShowDelay:
    case eProp_MarqueeType:
    case eProp_CurrentWindow:
    case eProp_MarqueeFormat:
    case eProp_WindowCount:

    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNCR5976::SetPropVal(const long lPropNum, tVariant *varPropVal)
{
    Debug::log("CAddInNCR5976::SetPropVal");
    switch(lPropNum)
    {
    case eProp_Version:
        return false;

    case eProp_Result:
    case eProp_CurrentDeviceNumber:
    case eProp_CurrentDeviceName:
    case eProp_Model:
    case eProp_DataBits:
    case eProp_LoadFonts:
    case eProp_Port:
    case eProp_Speed:
    case eProp_StopBits:
    case eProp_Parity:
    case eProp_CodePage:
    case eProp_ResultDescription:
    case eProp_DeviceColumnCount:
    case eProp_DeviceTurnedOn:
    case eProp_MarqueeRepeatDelay:
    case eProp_MarqueeShowDelay:
    case eProp_MarqueeType:
    case eProp_CurrentWindow:
    case eProp_MarqueeFormat:
    case eProp_WindowCount:
        return true;

    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNCR5976::IsPropReadable(const long lPropNum)
{
    Debug::log("CAddInNCR5976::IsPropReadable");
    switch(lPropNum)
    {
    case eProp_Version:
    case eProp_Result:
    case eProp_CurrentDeviceNumber:
    case eProp_CurrentDeviceName:
    case eProp_Model:
    case eProp_DataBits:
    case eProp_LoadFonts:
    case eProp_Port:
    case eProp_Speed:
    case eProp_StopBits:
    case eProp_Parity:
    case eProp_CodePage:
    case eProp_ResultDescription:
    case eProp_DeviceColumnCount:
    case eProp_DeviceTurnedOn:
    case eProp_MarqueeRepeatDelay:
    case eProp_MarqueeShowDelay:
    case eProp_MarqueeType:
    case eProp_CurrentWindow:
    case eProp_MarqueeFormat:
    case eProp_WindowCount:
        return true;
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNCR5976::IsPropWritable(const long lPropNum)
{
    Debug::log("CAddInNCR5976::IsPropWritable");
    switch(lPropNum)
    {
    case eProp_Version:
        return false;
    case eProp_Result:
        return false;
    case eProp_CurrentDeviceNumber:
    case eProp_CurrentDeviceName:
    case eProp_Model:
    case eProp_DataBits:
    case eProp_LoadFonts:
    case eProp_Port:
    case eProp_Speed:
    case eProp_StopBits:
    case eProp_Parity:
    case eProp_CodePage:
    case eProp_ResultDescription:
    case eProp_DeviceColumnCount:
    case eProp_DeviceTurnedOn:
    case eProp_MarqueeRepeatDelay:
    case eProp_MarqueeShowDelay:
    case eProp_MarqueeType:
    case eProp_CurrentWindow:
    case eProp_MarqueeFormat:
    case eProp_WindowCount:
        return true;
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
long CAddInNCR5976::GetNMethods()
{
    Debug::log("CAddInNCR5976::GetNMethods");
    return eMeth_Last;
}
//---------------------------------------------------------------------------//
long CAddInNCR5976::FindMethod(const WCHAR_T* wsMethodName)
{
    Debug::log("CAddInNCR5976::FindMethod");
    long plMethodNum = -1;
    wchar_t* name = 0;

    ::convFromShortWchar(&name, wsMethodName);

    plMethodNum = findName(g_MethodNames, name);

    return plMethodNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNCR5976::GetMethodName(const long lMethodNum, const long lMethodAlias)
{
    Debug::log("CAddInNCR5976::GetNethodName");
    if (lMethodNum >= eMeth_Last)
        return NULL;

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsMethodName = NULL;
    int iActualSize = 0;

    switch(lMethodAlias)
    {
    case 0: // First language
        wsCurrentName = g_MethodNames[lMethodNum].name;
        break;
    case 1: // Second language
        wsCurrentName = g_MethodNames[lMethodNum].ru;
        break;
    default:
        return 0;
    }

    iActualSize = wcslen(wsCurrentName)+1;

    if (m_iMemory && wsCurrentName)
    {
        if(m_iMemory->AllocMemory((void**)&wsMethodName, iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
    }

    return wsMethodName;
}
//---------------------------------------------------------------------------//
long CAddInNCR5976::GetNParams(const long lMethodNum)
{
    Debug::log("CAddInNCR5976::GetNParams");
    switch(lMethodNum)
    {
    case eMeth_CreateWindow:
        return 6;

    case eMeth_ShowTextPos:
        return 4;

	case eMeth_SendByte:
		return 1;

    case eMeth_AddDevice:
    case eMeth_DeleteDevice:
    case eMeth_Clear:
    case eMeth_ClearText:
    case eMeth_DeleteWindow:
    default:
        return 0;
    }

    return 0;
}
//---------------------------------------------------------------------------//
bool CAddInNCR5976::GetParamDefValue(const long lMethodNum, const long lParamNum,
                          tVariant *pvarParamDefValue)
{
    Debug::log("CAddInNCR5976::GetParamDefValue");
    TV_VT(pvarParamDefValue)= VTYPE_EMPTY;

    switch(lMethodNum)
    {
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNCR5976::HasRetVal(const long lMethodNum)
{
    Debug::log("CAddInNCR5976::HasRetVal");
    switch(lMethodNum)
    {
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNCR5976::CallAsProc(const long lMethodNum,
                    tVariant* paParams, const long lSizeArray)
{
    Debug::log("CAddInNCR5976::CallAsProc");
    switch(lMethodNum)
    {
	case eMeth_AddDevice:
		m_devices.AddDevice();
		m_devices.Current().Open("COM3");
		break;

	case eMeth_SendByte:
		{
		tVariant *var = paParams;
		char byte = (char)(var->intVal);

		m_devices.Current().SendData(&byte);

		break;
	}
        /*
    case eMethEnable:
        m_boolEnabled = true;
        break;
    case eMethDisable:
        m_boolEnabled = false;
        break;
    case eMethShowInStatusLine:
        if (m_iConnect && lSizeArray)
        {
            tVariant *var = paParams;
            m_iConnect->SetStatusLine(var->pwstrVal);

            Sleep(5000);
        }
        break;
    case eMethStartTimer:
        pAsyncEvent = m_iConnect;
        break;
    case eMethStopTimer:
        if (m_uiTimer != 0)
            ::KillTimer(NULL,m_uiTimer);
        m_uiTimer = 0;
        pAsyncEvent = NULL;
        break;
        */
    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNCR5976::CallAsFunc(const long lMethodNum,
                tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    Debug::log("CAddInNCR5976::CallAsFunc");
    bool ret = false;
    return ret;
}
//---------------------------------------------------------------------------//
void CAddInNCR5976::SetLocale(const WCHAR_T* loc)
{
    Debug::log("CAddInNCR5976::SetLocale");
    _wsetlocale(LC_ALL, loc);
}
/////////////////////////////////////////////////////////////////////////////
// LocaleBase
//---------------------------------------------------------------------------//
bool CAddInNCR5976::setMemManager(void* mem)
{
    Debug::log("CAddInNCR5976::SetMemManager");
    m_iMemory = (IMemoryManager*)mem;
    return m_iMemory != 0;
}
//---------------------------------------------------------------------------//
void CAddInNCR5976::addError(uint32_t wcode, const wchar_t* source,
                        const wchar_t* descriptor, long code)
{
    Debug::log("CAddInNCR5976::AddError");
    if (m_iConnect)
    {
        WCHAR_T *err = 0;
        WCHAR_T *descr = 0;

        ::convToShortWchar(&err, source);
        ::convToShortWchar(&descr, descriptor);

        m_iConnect->AddError(wcode, err, descr, code);
        delete[] err;
        delete[] descr;
    }
}
//---------------------------------------------------------------------------//
long CAddInNCR5976::findName(const Aliases *names, const wchar_t* name) const
{
    Debug::log("CAddInNCR5976::findName");
    int i = 0;
    while (names[i].ru[0] != 0) {
        if (!wcscmp(names[i].name, name))
            return i;
        if (!wcscmp(names[i].ru, name))
            return i;
		++i;
    }
    return -1;
}

//---------------------------------------------------------------------------//
uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len)
{
    Debug::log("convToShortWChar");
    if (!len)
        len = ::wcslen(Source)+1;

    if (!*Dest)
        *Dest = new WCHAR_T[len];

    WCHAR_T* tmpShort = *Dest;
    wchar_t* tmpWChar = (wchar_t*) Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len*sizeof(WCHAR_T));
    do
    {
        *tmpShort++ = (WCHAR_T)*tmpWChar++;
        ++res;
    }
    while (len-- && *tmpWChar);

    return res;
}
//---------------------------------------------------------------------------//
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
{
    Debug::log("convFromShortWChar");
    if (!len)
        len = getLenShortWcharStr(Source)+1;

    if (!*Dest)
        *Dest = new wchar_t[len];

    wchar_t* tmpWChar = *Dest;
    WCHAR_T* tmpShort = (WCHAR_T*)Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len*sizeof(wchar_t));
    do
    {
        *tmpWChar++ = (wchar_t)*tmpShort++;
        ++res;
    }
    while (len-- && *tmpShort);

    return res;
}
//---------------------------------------------------------------------------//
uint32_t getLenShortWcharStr(const WCHAR_T* Source)
{
    Debug::log("getLenShortWcharStr");
    uint32_t res = 0;
    WCHAR_T *tmpShort = (WCHAR_T*)Source;

    while (*tmpShort++)
        ++res;

    return res;
}
//---------------------------------------------------------------------------//