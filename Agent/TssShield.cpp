#include <iostream>
#include <windows.h>
#include <fltuser.h>
#include <fstream>
#include <thread>
#include <vector>
#include <Psapi.h>
#include "include/json.hpp"
#include "../Common/TssShield.h"

#pragma comment(lib, "fltlib.lib")

#define SERVICE_NAME "TssShield"

using json = nlohmann::json;

SERVICE_STATUS g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_ServiceStatusHandle = NULL;
HANDLE g_hServiceStopEvent = NULL;
HANDLE g_hDriverConnection = NULL;
std::thread g_DriverMessageThread;

void WINAPI ServiceMain(DWORD argc, LPSTR* argv);
void WINAPI ServiceCtrlHandler(DWORD CtrlCode);
void ServiceWorkerThread();
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);

void ShowUsage()
{
    std::cout << "Usage: TssShield.exe [--install | --uninstall | --console]" << std::endl;
}

void LogToFile(const std::string& message)
{
    std::ofstream logfile("C:\\TssShield.log", std::ios_base::app);
    if (logfile.is_open())
    {
        logfile << message << std::endl;
    }
}

std::string GetProcessNameFromPid(DWORD pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess == NULL)
    {
        return "";
    }

    char processName[MAX_PATH];
    if (GetModuleBaseNameA(hProcess, NULL, processName, sizeof(processName)) > 0)
    {
        CloseHandle(hProcess);
        return std::string(processName);
    }

    CloseHandle(hProcess);
    return "";
}

void DriverMessageThread(HANDLE hPort, json config)
{
    std::vector<BYTE> buffer(sizeof(FILTER_MESSAGE_HEADER) + sizeof(TSS_SHIELD_REQUEST));
    FILTER_MESSAGE_HEADER* messageHeader = (FILTER_MESSAGE_HEADER*)buffer.data();

    while (WaitForSingleObject(g_hServiceStopEvent, 0) != WAIT_OBJECT_0)
    {
        HRESULT hr = FilterGetMessage(hPort, messageHeader, (DWORD)buffer.size(), NULL);
        if (IS_ERROR(hr))
        {
            if (hr == HRESULT_FROM_WIN32(ERROR_IO_PENDING))
            {
                continue;
            }
            LogToFile("FilterGetMessage failed.");
            break;
        }

        TSS_SHIELD_REQUEST* request = (TSS_SHIELD_REQUEST*)((PBYTE)messageHeader + sizeof(FILTER_MESSAGE_HEADER));
        std::string processName = GetProcessNameFromPid((DWORD)(ULONG_PTR)request->ProcessId);

        TSS_SHIELD_REPLY reply;
        reply.Allow = FALSE;

        auto& whitelist = config["white_list"];
        for (const auto& allowed_process : whitelist)
        {
            if (_stricmp(processName.c_str(), allowed_process.get<std::string>().c_str()) == 0)
            {
                reply.Allow = TRUE;
                break;
            }
        }

        std::wstring wFilePath(request->FilePath);
        std::string sFilePath(wFilePath.begin(), wFilePath.end());
        std::string logMessage = (reply.Allow ? "Allowed" : "Denied") + std::string(" access to ") + sFilePath + " for process " + processName;
        LogToFile(logMessage);

        FILTER_REPLY_HEADER replyHeader;
        replyHeader.Status = STATUS_SUCCESS;
        replyHeader.MessageId = messageHeader->MessageId;

        hr = FilterReplyMessage(hPort, &replyHeader, &reply, sizeof(reply));
        if (IS_ERROR(hr))
        {
            LogToFile("FilterReplyMessage failed.");
        }
    }
}

bool ConnectDriver(HANDLE* pPort)
{
    HRESULT hr = FilterConnectCommunicationPort(TSS_SHIELD_PORT_NAME, 0, NULL, 0, NULL, pPort);
    if (IS_ERROR(hr))
    {
        LogToFile("Could not connect to the driver.");
        return false;
    }
    return true;
}

void SendWatchDirectories(HANDLE hPort, const json& config)
{
    if (config.find("dirs") == config.end() || !config["dirs"].is_array())
    {
        LogToFile("Invalid configuration: 'dirs' not found or not an array.");
        return;
    }

    for (const auto& dir : config["dirs"])
    {
        std::string s_dir = dir.get<std::string>();
        std::wstring ws_dir(s_dir.begin(), s_dir.end());

        DWORD bytesReturned;
        HRESULT hr = FilterSendMessage(hPort, (LPVOID)ws_dir.c_str(), (DWORD)(ws_dir.size() * sizeof(wchar_t)), NULL, 0, &bytesReturned);
        if (IS_ERROR(hr))
        {
            LogToFile("Failed to send directory to driver.");
        }
    }
}

void RunAsConsole()
{
    std::cout << "Running in console mode. Press Ctrl+C to stop." << std::endl;
    g_hServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE))
    {
        std::cerr << "Failed to set console control handler" << std::endl;
        return;
    }

    std::thread worker(ServiceWorkerThread);
    worker.join();
}

void InstallService()
{
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager)
    {
        std::cerr << "Failed to open SCManager" << std::endl;
        return;
    }

    SC_HANDLE hService = CreateServiceA(hSCManager, SERVICE_NAME, SERVICE_NAME, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, path, NULL, NULL, NULL, NULL, NULL);
    if (!hService)
    {
        std::cerr << "Failed to create service" << std::endl;
    }
    else
    {
        std::cout << "Service installed successfully" << std::endl;
        CloseServiceHandle(hService);
    }
    CloseServiceHandle(hSCManager);
}

void UninstallService()
{
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCManager)
    {
        std::cerr << "Failed to open SCManager" << std::endl;
        return;
    }

    SC_HANDLE hService = OpenServiceA(hSCManager, SERVICE_NAME, DELETE);
    if (!hService)
    {
        std::cerr << "Failed to open service" << std::endl;
    }
    else
    {
        if (DeleteService(hService))
        {
            std::cout << "Service uninstalled successfully" << std::endl;
        }
        else
        {
            std::cerr << "Failed to uninstall service" << std::endl;
        }
        CloseServiceHandle(hService);
    }
    CloseServiceHandle(hSCManager);
}

void ServiceWorkerThread()
{
    LogToFile("TssShield service started.");

    std::ifstream f("C:\\TssShield.json");
    if (!f.is_open())
    {
        LogToFile("Could not open TssShield.json");
        return;
    }

    json config;
    try
    {
        config = json::parse(f);
    }
    catch (json::parse_error& e)
    {
        LogToFile("JSON parse error: " + std::string(e.what()));
        f.close();
        return;
    }
    f.close();

    if (!ConnectDriver(&g_hDriverConnection))
    {
        return;
    }

    SendWatchDirectories(g_hDriverConnection, config);

    g_DriverMessageThread = std::thread(DriverMessageThread, g_hDriverConnection, config);

    WaitForSingleObject(g_hServiceStopEvent, INFINITE);

    if (g_hDriverConnection != NULL)
    {
        CloseHandle(g_hDriverConnection);
    }

    g_DriverMessageThread.join();
    LogToFile("TssShield service stopped.");
}

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_C_EVENT)
    {
        SetEvent(g_hServiceStopEvent);
        return TRUE;
    }
    return FALSE;
}

void WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
    switch (CtrlCode)
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
            break;

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;
        SetServiceStatus(g_ServiceStatusHandle, &g_ServiceStatus);

        SetEvent(g_hServiceStopEvent);
        break;
    default:
        break;
    }
}

void WINAPI ServiceMain(DWORD argc, LPSTR* argv)
{
    g_ServiceStatusHandle = RegisterServiceCtrlHandlerA(SERVICE_NAME, ServiceCtrlHandler);
    if (g_ServiceStatusHandle == NULL)
    {
        return;
    }

    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;
    SetServiceStatus(g_ServiceStatusHandle, &g_ServiceStatus);

    g_hServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_hServiceStopEvent == NULL)
    {
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(g_ServiceStatusHandle, &g_ServiceStatus);
        return;
    }

    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;
    SetServiceStatus(g_ServiceStatusHandle, &g_ServiceStatus);

    std::thread(ServiceWorkerThread).join();

    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_ServiceStatusHandle, &g_ServiceStatus);
}

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        SERVICE_TABLE_ENTRYA ServiceTable[] = { { (LPSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTIONA)ServiceMain }, { NULL, NULL } };
        if (StartServiceCtrlDispatcherA(ServiceTable) == FALSE)
        {
            return GetLastError();
        }
        return 0;
    }

    if (argc != 2)
    {
        ShowUsage();
        return 1;
    }

    if (strcmp(argv[1], "--console") == 0)
    {
        RunAsConsole();
    }
    else if (strcmp(argv[1], "--install") == 0)
    {
        InstallService();
    }
    else if (strcmp(argv[1], "--uninstall") == 0)
    {
        UninstallService();
    }
    else
    {
        ShowUsage();
        return 1;
    }

    return 0;
}
