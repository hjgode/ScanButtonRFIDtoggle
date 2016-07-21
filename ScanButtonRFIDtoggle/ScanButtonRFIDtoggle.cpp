// ScanButtonRFIDtoggle.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

/// set scan button to RFID or SCAN using the datacollection2 API
void set_RFID_OR_BARCODE(bool bswitchToRFID){
	INT32 pHandle=NULL;
	TCHAR* pszDeviceName = L"All";
	DWORD dwDataBufferSize=512;
	HRESULT hRes = ITCSCAN_Open(&pHandle, pszDeviceName, dwDataBufferSize);
	if (hRes!=ERROR_SUCCESS){
		return;
	}
	ITCSCAN_BUTTON_ID eButtonID=CENTER;
	ITCSCAN_BUTTON_ACTION_TYPE peActionType;
	hRes = ITCSCAN_GetDeviceButtonAction(eButtonID, &peActionType);
	if (hRes!=ERROR_SUCCESS){
		goto exit;
	}
	//change needed?
	if ( (peActionType==SCAN && !bswitchToRFID) || (peActionType==RFID_WEDGE && bswitchToRFID) )
		goto exit;

	if(bswitchToRFID)
		hRes=ITCSCAN_SetDeviceButtonAction(eButtonID, RFID_WEDGE);
	else
		hRes=ITCSCAN_SetDeviceButtonAction(eButtonID, SCAN);

	if(hRes==ERROR_SUCCESS)
		DEBUGMSG(1, (L"ITCSCAN_SetDeviceButtonAction OK for %s\n", bswitchToRFID?L"RFID_WEDGE":L"SCAN"));
	else
		DEBUGMSG(1, (L"ITCSCAN_SetDeviceButtonAction FAILED with %i for %s\n", GetLastError(), bswitchToRFID?L"RFID_WEDGE":L"SCAN"));

exit:
	ITCSCAN_Close(pHandle);
}

/// toggle scan button to RFID or SCAN using the datacollection2 API
void toggleScanRFID(){
	INT32 pHandle=NULL;
	TCHAR* pszDeviceName = L"All";
	DWORD dwDataBufferSize=512;
	HRESULT hRes = ITCSCAN_Open(&pHandle, pszDeviceName, dwDataBufferSize);
	if (hRes!=ERROR_SUCCESS){
		return;
	}
	ITCSCAN_BUTTON_ID eButtonID=CENTER;
	ITCSCAN_BUTTON_ACTION_TYPE peActionType;
	hRes = ITCSCAN_GetDeviceButtonAction(eButtonID, &peActionType);
	if (hRes!=ERROR_SUCCESS){
		goto exit;
	}
	if(peActionType==SCAN){
		hRes=ITCSCAN_SetDeviceButtonAction(eButtonID, RFID_WEDGE);
		if(hRes==ERROR_SUCCESS)
			DEBUGMSG(1, (L"ITCSCAN_SetDeviceButtonAction OK for %s\n", L"RFID_WEDGE"));
		else
			DEBUGMSG(1, (L"ITCSCAN_SetDeviceButtonAction FAILED with %i for %s\n", GetLastError(), L"RFID_WEDGE"));

	}
	else{
		hRes=ITCSCAN_SetDeviceButtonAction(eButtonID, SCAN);
		if(hRes==ERROR_SUCCESS)
			DEBUGMSG(1, (L"ITCSCAN_SetDeviceButtonAction OK for %s\n", L"SCAN"));
		else
			DEBUGMSG(1, (L"ITCSCAN_SetDeviceButtonAction FAILED with %i for %s\n", GetLastError(), L"SCAN"));
	}
exit:
	ITCSCAN_Close(pHandle);
}

TCHAR* regSubKeyDriverLink=L"Hardware\\DeviceMap\\Keybd";
TCHAR  regSubKeyKeyboard[MAX_PATH];
/// change scan button to RFID or SCAN using the registry
void set_RFID_SCANBUTTON(bool bRFID){
	HKEY hKey;
	// HKEY_LOCAL_MACHINE\Hardware\DeviceMap\Keybd
	ulong uRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regSubKeyDriverLink, NULL, KEY_QUERY_VALUE, &hKey);
	if(uRes!=ERROR_SUCCESS)
		return;

	DWORD cbData=MAX_PATH;
	DWORD dwType=REG_SZ;
	TCHAR szTemp[MAX_PATH];
	//Drivers\HID\ClientDrivers\ITCKeyboard\Layout\A-Numeric\0001
	uRes = RegQueryValueEx(hKey, L"CurrentActiveLayoutKey", NULL, &dwType, (LPBYTE)szTemp, &cbData);
	if(uRes!=ERROR_SUCCESS){
		DEBUGMSG(1, (L"CurrentActiveLayoutKey: %i\n", GetLastError()));
		RegCloseKey(hKey);
		return;
	}
	RegCloseKey(hKey);
	TCHAR regSubDeltaEvent[MAX_PATH];
	TCHAR regSubStateEvent[MAX_PATH];
	wsprintf(regSubKeyKeyboard, L"%s", szTemp);

	//HKEY_LOCAL_MACHINE\Drivers\HID\ClientDrivers\ITCKeyboard\Layout\A-Numeric\0001\Events\Delta
	wsprintf(regSubDeltaEvent, L"%s\\Events\\Delta", regSubKeyKeyboard);

	uRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regSubDeltaEvent, NULL, KEY_QUERY_VALUE, &hKey);
	if(uRes!=ERROR_SUCCESS){
		DEBUGMSG(1, (L"regSubDeltaEvent: %i\n", GetLastError()));
		return;
	}
	dwType=REG_SZ;
	cbData=MAX_PATH;
	uRes = RegQueryValueEx(hKey, L"event1", NULL, &dwType, (LPBYTE)szTemp, &cbData);
	if(uRes!=ERROR_SUCCESS){
		DEBUGMSG(1, (L"RegQueryValueEx Delta event1: %i\n", GetLastError()));
		RegCloseKey(hKey);
		return;
	}
	DEBUGMSG(1, (L"%s:%s\n", regSubDeltaEvent, szTemp));

	wsprintf(regSubStateEvent, L"%s\\Events\\State", regSubKeyKeyboard);
	uRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regSubStateEvent, NULL, KEY_QUERY_VALUE, &hKey);
	if(uRes!=ERROR_SUCCESS){
		DEBUGMSG(1, (L"regSubStateEvent: %i\n", GetLastError()));
		return;
	}
	dwType=REG_SZ;
	cbData=MAX_PATH;
	uRes = RegQueryValueEx(hKey, L"event1", NULL, &dwType, (LPBYTE)szTemp, &cbData);
	if(uRes!=ERROR_SUCCESS){
		DEBUGMSG(1, (L"RegQueryValueEx State event1: %i\n", GetLastError()));
		RegCloseKey(hKey);
		return;
	}
	DEBUGMSG(1, (L"%s:%s\n", regSubStateEvent, szTemp));

	RegCloseKey(hKey);

	//ITC_RFID_TRIGGER_EVENT	DeltaLeftScan
	//ITC_RFID_TRIGGER_STATE	StateLeftScan

	TCHAR newDelta[MAX_PATH];
	TCHAR newState[MAX_PATH];
	if(bRFID){
		wsprintf(newDelta, L"ITC_RFID_TRIGGER_EVENT");
		wsprintf(newState, L"ITC_RFID_TRIGGER_STATE");
	}
	else{
		wsprintf(newDelta, L"DeltaLeftScan");
		wsprintf(newState, L"StateLeftScan");
	}

	uRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regSubDeltaEvent, NULL, KEY_SET_VALUE, &hKey);
	if(uRes!=ERROR_SUCCESS){
		DEBUGMSG(1, (L"regSubDeltaEvent: %i\n", GetLastError()));
		return;
	}
	dwType=REG_SZ;
	cbData=wcslen(newDelta)*sizeof(TCHAR) + 2;
	uRes=RegSetValueEx(hKey, L"event1", NULL, dwType, (LPBYTE)newDelta, cbData);
	if(uRes!=ERROR_SUCCESS){
		DEBUGMSG(1, (L"RegSetValueEx-regSubDeltaEvent: %i\n", GetLastError()));
		RegCloseKey(hKey);
		return;
	}
	RegCloseKey(hKey);

	uRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regSubStateEvent, NULL, KEY_SET_VALUE, &hKey);
	if(uRes!=ERROR_SUCCESS){
		DEBUGMSG(1, (L"regSubStateEvent: %i\n", GetLastError()));
		return;
	}
	dwType=REG_SZ;
	cbData=wcslen(newState)*sizeof(TCHAR) + 2;
	uRes=RegSetValueEx(hKey, L"event1", NULL, dwType, (LPBYTE)newState, cbData);
	if(uRes!=ERROR_SUCCESS){
		DEBUGMSG(1, (L"RegSetValueEx-regSubStateEvent: %i\n", GetLastError()));
		RegCloseKey(hKey);
		return;
	}

	RegCloseKey(hKey);

	PostMessage(HWND_BROADCAST, WM_WININICHANGE, NULL, NULL);
}

int _tmain(int argc, _TCHAR* argv[])
{
	if(argc==2){
		DEBUGMSG(1, (L"cmd=%s\n", argv[1]));
		if(wcsicmp(argv[1], L"RFID")==0){
			//set_RFID_SCANBUTTON(TRUE);	//	uses registry
			set_RFID_OR_BARCODE(TRUE);		//	uses datacollection2 API
		}
		else if(wcsicmp(argv[1], L"SCAN")==0){
			//set_RFID_SCANBUTTON(FALSE);		//	uses registry
			set_RFID_OR_BARCODE(FALSE);	//	uses datacollection2 API
		}
	}
	else{
		DEBUGMSG(1, (L"no cmd, using toggle\n"));
		toggleScanRFID();	//	uses datacollection2 API
	}
	return 0;
}

