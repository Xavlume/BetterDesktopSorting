#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shlobj.h>
#include <exdisp.h>
#include <shlwapi.h>
#include <atlbase.h>
#include <atlalloc.h>
#include <stdio.h>
#include "wtypes.h"
#include <string>
#include <iostream>
#include <map>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <vector>

//https://devblogs.microsoft.com/oldnewthing/20130318-00/?p=4933
//Huge help ^^

const char SORT_METHOD_ALPHABETICAL = 'A';
const char SORT_METHOD_FILE_TYPE = 'F';
const char SORT_METHOD_FILE_TYPE_DUAL = 'D';

class CCoInitialize {
public:
    CCoInitialize() : m_hr(CoInitialize(NULL)) { }
    ~CCoInitialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
    operator HRESULT() const { return m_hr; }
    HRESULT m_hr;
};

class iconClass {
public:
    long x = 0;
    long y = 0;
    CComHeapPtr<ITEMID_CHILD> id;
    bool isFolder = false;
    bool isLink = false;
};

class desktopSpacingInfo {
public:
    long minX = 0;
    long minY = 0;
    long maxX = 0;
    long maxY = 0;

    long diffX = 10000;
    long diffY = 10000;

    char sortType = SORT_METHOD_FILE_TYPE;

    desktopSpacingInfo(std::vector<long> xs, std::vector<long> ys) {
        minX = *std::min_element(xs.begin(), xs.end());
        maxX = *std::max_element(xs.begin(), xs.end());
        minY = *std::min_element(ys.begin(), ys.end());
        maxY = *std::max_element(ys.begin(), ys.end());

        for (auto i = xs.begin(); i != xs.end(); ++i) {
            long curValue = *i;
            for (auto j = xs.begin(); j != xs.end(); ++j) {
                long otherValue = *j;
                long valueDiff = std::abs(curValue - otherValue);
                if (valueDiff != 0 and valueDiff < diffX and valueDiff > 5) {
                    diffX = valueDiff;
                }
            }
        }

        for (auto i = ys.begin(); i != ys.end(); ++i) {
            long curValue = *i;
            for (auto j = ys.begin(); j != ys.end(); ++j) {
                long otherValue = *j;
                long valueDiff = std::abs(curValue - otherValue);
                if (valueDiff != 0 and valueDiff < diffY and valueDiff > 5) {
                    diffY = valueDiff;
                }
            }
        }

        std::cout << diffX << std::endl;
        std::cout << diffY << std::endl;
    }

    desktopSpacingInfo(std::string path) {
        std::ifstream MyReadFile(path.c_str());

        std::string tempStr;

        getline(MyReadFile, tempStr);
        minX = std::stol(tempStr);

        getline(MyReadFile, tempStr);
        minY = std::stol(tempStr);

        getline(MyReadFile, tempStr);
        maxX = std::stol(tempStr);

        getline(MyReadFile, tempStr);
        maxY = std::stol(tempStr);

        getline(MyReadFile, tempStr);
        diffX = std::stol(tempStr);

        getline(MyReadFile, tempStr);
        diffY = std::stol(tempStr);

        getline(MyReadFile, tempStr);
        sortType = tempStr[0];
    }

    void toFile(std::string path) {
        std::ofstream outFile(path.c_str());

        outFile << minX << std::endl;
        outFile << minY << std::endl;
        outFile << maxX << std::endl;
        outFile << maxY << std::endl;
        outFile << diffX << std::endl;
        outFile << diffY << std::endl;
        outFile << sortType;

        outFile.close();
    }
};


void FindDesktopFolderView(REFIID riid, void** ppv)
{
    CComPtr<IShellWindows> spShellWindows;
    spShellWindows.CoCreateInstance(CLSID_ShellWindows);

    CComVariant vtLoc(CSIDL_DESKTOP);
    CComVariant vtEmpty;
    long lhwnd;
    CComPtr<IDispatch> spdisp;
    spShellWindows->FindWindowSW(
        &vtLoc, &vtEmpty,
        SWC_DESKTOP, &lhwnd, SWFO_NEEDDISPATCH, &spdisp);

    CComPtr<IShellBrowser> spBrowser;
    CComQIPtr<IServiceProvider>(spdisp)->
        QueryService(SID_STopLevelBrowser,
            IID_PPV_ARGS(&spBrowser));

    CComPtr<IShellView> spView;
    spBrowser->QueryActiveShellView(&spView);

    spView->QueryInterface(riid, ppv);
}

std::map<std::wstring, iconClass> getIcons(bool log) {
    CCoInitialize init;
    CComPtr<IFolderView> spView;
    FindDesktopFolderView(IID_PPV_ARGS(&spView));
    CComPtr<IShellFolder> spFolder;
    spView->GetFolder(IID_PPV_ARGS(&spFolder));

    CComPtr<IEnumIDList> spEnum;
    spView->Items(SVGIO_ALLVIEW, IID_PPV_ARGS(&spEnum));

    std::map<std::wstring, iconClass> icons;

    for (CComHeapPtr<ITEMID_CHILD> spidl;
        spEnum->Next(1, &spidl, nullptr) == S_OK;
        spidl.Free())
    {
        STRRET str;
        spFolder->GetDisplayNameOf(spidl, SHGDN_NORMAL, &str);
        CComHeapPtr<wchar_t> spszName;
        StrRetToStr(&str, spidl, &spszName);

        POINT pt;
        spView->GetItemPosition(spidl, &pt);

        iconClass tempObj;
        tempObj.x = pt.x;
        tempObj.y = pt.y;
        tempObj.id = spidl;

        PCITEMID_CHILD apidl[1] = { tempObj.id };

        SFGAOF flags1 = SFGAO_LINK | SFGAO_FOLDER;
        spFolder->GetAttributesOf(1, apidl, &flags1);
        tempObj.isFolder = SFGAO_FOLDER & flags1;

        SFGAOF flags2 = SFGAO_LINK | SFGAO_FOLDER;
        spFolder->GetAttributesOf(1, apidl, &flags2);
        tempObj.isLink = SFGAO_LINK & flags2;

        std::wstring windowname(spszName.m_pData);
        std::transform(
            windowname.begin(), windowname.end(),
            windowname.begin(),
            towlower);
        icons[windowname] = tempObj;

        if (log)
            wprintf(L"At %4d,%4d is %ls\n", pt.x, pt.y, spszName);
    }

    return icons;
}

void moveRecycle(desktopSpacingInfo info, PCITEMID_CHILD* apidl, CComPtr<IFolderView> spView) {

    POINT pt;
    pt.x = info.minX;
    pt.y = info.minY;

    spView->SelectAndPositionItems(
        1, apidl, &pt, SVSI_POSITIONITEM);
}

void moveIcon(long &x, long &y, desktopSpacingInfo info, PCITEMID_CHILD *apidl, CComPtr<IFolderView> spView, bool right = false) {

    POINT pt;
    pt.x = x;
    pt.y = y;

    y += info.diffY;
    if (y > info.maxY) {
        if (right)
            x -= info.diffX;
        else
            x += info.diffX;
        y = info.minY;
    }

    spView->SelectAndPositionItems(
        1, apidl, &pt, SVSI_POSITIONITEM);
}

void calibrate(std::string path) {
    std::map<std::wstring, iconClass> icons = getIcons(true);

    std::vector<long> xs;
    std::vector<long> ys;

    for (const auto& icon : icons) {
        xs.push_back(icon.second.x);
        ys.push_back(icon.second.y);
    }

    desktopSpacingInfo info(xs, ys);

    info.toFile(path);
}

void calibrated(std::string path) {
    std::map<std::wstring, iconClass> icons = getIcons(false);

    desktopSpacingInfo info(path);

    CCoInitialize init;
    CComPtr<IFolderView> spView;
    FindDesktopFolderView(IID_PPV_ARGS(&spView));

    CComPtr<IShellFolder> spFolder;
    spView->GetFolder(IID_PPV_ARGS(&spFolder));

    long leftX = info.minX;
    long leftY = info.minY + info.diffY; //Space above reserved for the recycle bin
    long rightX = info.maxX;
    long rightY = info.minY;

    if (info.sortType == SORT_METHOD_ALPHABETICAL) {

        for (const auto& icon : icons) {
            PCITEMID_CHILD apidl[1] = { icon.second.id };

            if (icon.first == L"recycle bin")
                moveRecycle(info, apidl, spView);
            else
                moveIcon(leftX, leftY, info, apidl, spView);

        }
    }

    else {
        for (int iteration = 1; iteration <= 3; iteration++) {
            for (const auto& icon : icons) {
                PCITEMID_CHILD apidl[1] = { icon.second.id };

                if (icon.first == L"recycle bin") {
                    moveRecycle(info, apidl, spView);
                }
                else {
                    if (iteration == 1) {
                        if (icon.second.isFolder)
                            moveIcon(leftX, leftY, info, apidl, spView);
                    }
                    else if (iteration == 2) {
                        if (icon.second.isLink)
                            if (info.sortType == SORT_METHOD_FILE_TYPE_DUAL)
                                moveIcon(rightX, rightY, info, apidl, spView, true);
                            else
                                moveIcon(leftX, leftY, info, apidl, spView);
                        }
                    else {
                        if (!icon.second.isFolder and !icon.second.isLink)
                            moveIcon(leftX, leftY, info, apidl, spView);
                    }
                }
            }
        }
    }
}

int __cdecl wmain(int argc, wchar_t** argv)
{

    char* buf = nullptr;
    size_t sz = 0;
    _dupenv_s(&buf, &sz, "APPDATA");
    std::string path(buf);
    path += "\\BetterDesktopSorting";

    if (!std::filesystem::exists(std::filesystem::status(path)))
        std::filesystem::create_directory(path);

    path += "\\Config.txt";

    if (!std::filesystem::exists(std::filesystem::status(path)))
        calibrate(path);
    else
        calibrated(path);

    return 0;
}