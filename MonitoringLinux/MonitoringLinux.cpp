#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

int main() {
    HRESULT hres;

    // Инициализация COM
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        std::cerr << "Failed to initialize COM library." << std::endl;
        return 1;
    }

    // Инициализация безопасности
    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );
    if (FAILED(hres)) {
        std::cerr << "Failed to initialize security." << std::endl;
        CoUninitialize();
        return 1;
    }

    // Получение указателя на интерфейс сервиса WMI
    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc
    );
    if (FAILED(hres)) {
        std::cerr << "Failed to create IWbemLocator object." << std::endl;
        CoUninitialize();
        return 1;
    }

    // Подключение к сервису WMI
    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc
    );
    if (FAILED(hres)) {
        std::cerr << "Failed to connect to WMI service." << std::endl;
        pLoc->Release();
        CoUninitialize();
        return 1;
    }

    // Установка контекста безопасности для вызова методов
    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );
    if (FAILED(hres)) {
        std::cerr << "Failed to set proxy blanket." << std::endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;
    }

    // Получение нагрузки на процессор
    while (true) {
        IEnumWbemClassObject* pEnumerator = NULL;
        hres = pSvc->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT LoadPercentage FROM Win32_Processor"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pEnumerator
        );
        if (FAILED(hres)) {
            std::cerr << "Failed to execute WMI query." << std::endl;
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return 1;
        }

        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;

        while (pEnumerator) {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if (0 == uReturn)
                break;

            VARIANT vtProp;
            hr = pclsObj->Get(L"LoadPercentage", 0, &vtProp, 0, 0);
            if (SUCCEEDED(hr) && vtProp.vt == VT_UINT) {
                std::cout << "Total CPU Load: " << vtProp.uintVal << "%" << std::endl;
            }
            else {
                std::cerr << "Failed to get CPU load value." << std::endl;
            }

            VariantClear(&vtProp);
            pclsObj->Release();
        }

        pEnumerator->Release();

        Sleep(1000); // Ожидание в миллисекундах между обновлениями информации о загрузке
    }

    // Освобождение ресурсов
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    return 0;
}
