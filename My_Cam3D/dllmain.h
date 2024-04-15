#include "framework.h"
#include <Windows.h>
#include <Kinect.h>
#include <thread>

class CDepthBasics
{
    static const int cDepthWidth = 512;
    static const int cDepthHeight = 424;
    
public:
    CDepthBasics();
    ~CDepthBasics();
    int Zone_Arc = 3100;
    int distance_Max;
    bool m_bRGBMode;
    static LRESULT CALLBACK MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    int Run(HINSTANCE hInstance, int nCmdShow);

    int* m_depthData;
    int* m_depthRGBArray = nullptr;

    IKinectSensor* m_pKinectSensor;
    IDepthFrameReader* m_pDepthFrameReader;
    RGBQUAD* m_pDepthRGBX;

    void CopyDepthDataTo(int* pDepthArray, int nArraySize);
    std::thread m_updateThread;
    bool m_bUpdateThreadRunning = false;

    void CDepthBasics::SetOptions(int Arc, int Dist, bool RGBMode);

    void UpdateLoop();
    void CDepthBasics::Cleanup();
    void Update();
    HRESULT InitializeDefaultSensor();
    void ProcessDepth(INT64 nTime, const UINT16* pBuffer, int nHeight, int nWidth, USHORT nMinDepth, USHORT nMaxDepth);


private:
  
};

void KinectUpdateLoop();
