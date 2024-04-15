// pch.h : Il s'agit d'un fichier d'en-tête précompilé.
// Les fichiers listés ci-dessous sont compilés une seule fois, ce qui améliore les performances de génération des futures builds.
// Cela affecte également les performances d'IntelliSense, notamment la complétion du code et de nombreuses fonctionnalités de navigation du code.
// Toutefois, les fichiers listés ici sont TOUS recompilés si l'un d'entre eux est mis à jour entre les builds.
// N'ajoutez pas de fichiers fréquemment mis à jour ici, car cela annule les gains de performance.



// ajouter les en-têtes à précompiler ici

#include "framework.h"
#include <Windows.h>
#include <Kinect.h>

class CDepthBasics
{
    static const int cDepthWidth = 512;
    static const int cDepthHeight = 424;

public:
    CDepthBasics();
    ~CDepthBasics();

    static LRESULT CALLBACK MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    int Run(HINSTANCE hInstance, int nCmdShow);



    HWND m_hWnd;
    INT64 m_nStartTime;
    INT64 m_nLastCounter;
    double m_fFreq;
    INT64 m_nNextStatusTime;
    DWORD m_nFramesSinceUpdate;
    bool m_bSaveScreenshot;

    IKinectSensor* m_pKinectSensor;
    IDepthFrameReader* m_pDepthFrameReader;
    RGBQUAD* m_pDepthRGBX;

    void Update();
    HRESULT InitializeDefaultSensor();
    void ProcessDepth(INT64 nTime, const UINT16* pBuffer, int nHeight, int nWidth, USHORT nMinDepth, USHORT nMaxDepth);
    bool SetStatusMessage(_In_z_ WCHAR* szMessage, DWORD nShowTimeMsec, bool bForce);
    HRESULT GetScreenshotFileName(_Out_writes_z_(nFilePathSize) LPWSTR lpszFilePath, UINT nFilePathSize);
    HRESULT SaveBitmapToFile(BYTE* pBitmapBits, LONG lWidth, LONG lHeight, WORD wBitsPerPixel, LPCWSTR lpszFilePath);

private:
  
};


