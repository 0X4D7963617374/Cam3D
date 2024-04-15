#include <Kinect.h>
#include <algorithm>

#include "pch.h"
#include "stdafx.h"
#include "dllmain.h"

CDepthBasics* kinectInstance = nullptr;
bool m_bRGBMode = true;

void CDepthBasics::Cleanup()
{
    SafeRelease(m_pDepthFrameReader);
    SafeRelease(m_pKinectSensor);
}

extern "C" __declspec(dllexport) void Connect()
{
    if (kinectInstance == nullptr)
    {
        
        kinectInstance = new CDepthBasics();
        kinectInstance->InitializeDefaultSensor();
        kinectInstance->m_bUpdateThreadRunning = true;
        kinectInstance->m_updateThread = std::thread(&CDepthBasics::UpdateLoop, kinectInstance);
    }
}

extern "C" __declspec(dllexport) void Disconnect()
{
    if (kinectInstance != nullptr)
    {
        kinectInstance->m_bUpdateThreadRunning = false;
        kinectInstance->m_updateThread.join();
        kinectInstance->Cleanup();
        delete kinectInstance;
        kinectInstance = nullptr;
    }
}

extern "C" __declspec(dllexport) void ReceiveArray(int* array, int length)
{
    if (kinectInstance != nullptr)
    {
        kinectInstance->CopyDepthDataTo(array, length);
    }
}

void CDepthBasics::SetOptions(int Arc, int Dist, bool RGBMode)
{
    this->Zone_Arc = Arc;
    this->distance_Max = Dist;
    this->m_bRGBMode = RGBMode;
}

extern "C" __declspec(dllexport) void SetOptions(int Arc, int Dist, bool RGBMode)
{
    if (kinectInstance != nullptr)
    {
        kinectInstance->SetOptions(Arc, Dist, RGBMode);
    }
}

void CDepthBasics::CopyDepthDataTo(int* pDepthArray, int nArraySize)
{
    if (m_depthRGBArray != nullptr && pDepthArray != nullptr)
    {
        int maxSize = cDepthWidth * cDepthHeight;
        int copySize = (nArraySize < maxSize) ? nArraySize : maxSize;
        memcpy(pDepthArray, m_depthRGBArray, sizeof(int) * copySize);
    }
}

CDepthBasics::CDepthBasics()
{
    m_depthData = new int[cDepthWidth * cDepthHeight];
    m_depthRGBArray = new int[cDepthWidth * cDepthHeight];
    m_pDepthRGBX = new RGBQUAD[cDepthWidth * cDepthHeight];
}

CDepthBasics::~CDepthBasics()
{
    delete[] m_depthData;
    delete[] m_depthRGBArray;
    delete[] m_pDepthRGBX;
}

void CDepthBasics::UpdateLoop()
{
    while (m_bUpdateThreadRunning)
    {
        Update();
    }
}

void CDepthBasics::Update()
{
    IDepthFrame* pDepthFrame = nullptr;
    HRESULT hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);

    if (SUCCEEDED(hr))
    {
        INT64 nTime = 0;
        hr = pDepthFrame->get_RelativeTime(&nTime);

        IFrameDescription* pFrameDescription = nullptr;
        int nWidth = 0;
        int nHeight = 0;
        USHORT nDepthMinReliableDistance = 0;
        USHORT nDepthMaxDistance = 0;
        UINT nBufferSize = 0;
        UINT16* pBuffer = nullptr;

        if (SUCCEEDED(hr))
            hr = pDepthFrame->get_FrameDescription(&pFrameDescription);
        if (SUCCEEDED(hr))
            hr = pFrameDescription->get_Width(&nWidth);
        if (SUCCEEDED(hr))
            hr = pFrameDescription->get_Height(&nHeight);
        if (SUCCEEDED(hr))
            hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
        if (SUCCEEDED(hr))
            nDepthMaxDistance = USHRT_MAX;
        if (SUCCEEDED(hr))
            hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
        if (SUCCEEDED(hr))
            ProcessDepth(nTime, pBuffer, nWidth, nHeight, nDepthMinReliableDistance, nDepthMaxDistance);

        SafeRelease(pFrameDescription);
    }

    SafeRelease(pDepthFrame);
}

HRESULT CDepthBasics::InitializeDefaultSensor()
{
    HRESULT hr = GetDefaultKinectSensor(&m_pKinectSensor);

    if (SUCCEEDED(hr))
    {
        hr = m_pKinectSensor->Open();

        IDepthFrameSource* pDepthFrameSource = nullptr;
        if (SUCCEEDED(hr))
            hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
        if (SUCCEEDED(hr))
            hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);

        SafeRelease(pDepthFrameSource);
    }

    if (!m_pKinectSensor || FAILED(hr))
        return E_FAIL;

    return hr;
}

void CDepthBasics::ProcessDepth(INT64 nTime, const UINT16* pBuffer, int nHeight, int nWidth, USHORT nMinDepth, USHORT nMaxDepth)
{
    if (m_pDepthRGBX == nullptr || m_depthRGBArray == nullptr)
    {
        return;
    }

    RGBQUAD* pRGBX = m_pDepthRGBX;
    const UINT16* pBufferEnd = pBuffer + (nWidth * nHeight);

    while (pBuffer < pBufferEnd)
    {
        USHORT depth = *pBuffer;

        if ((depth >= nMinDepth) && (depth <= nMaxDepth))
        {
            if (m_bRGBMode)
            {
                float depthScale = ((float)depth - nMinDepth) / (nMaxDepth - nMinDepth);
                if (depth > distance_Max)
                {
                    pRGBX->rgbRed = 0;
                    pRGBX->rgbGreen = 0;
                    pRGBX->rgbBlue = 0;
                }
                else
                {
                    float h = Zone_Arc * depthScale;
                    float s = 1.0f;
                    float v = 1.0f;
                    int i = static_cast<int>(h / 60) % 6;
                    float f = h / 60 - i;
                    float p = v * (1 - s);
                    float q = v * (1 - s * f);
                    float t = v * (1 - s * (1 - f));

                    switch (i)
                    {
                    case 0:
                        pRGBX->rgbBlue = static_cast<BYTE>(v * 255);
                        pRGBX->rgbGreen = static_cast<BYTE>(t * 255);
                        pRGBX->rgbRed = static_cast<BYTE>(p * 255);
                        break;
                    case 1:
                        pRGBX->rgbBlue = static_cast<BYTE>(q * 255);
                        pRGBX->rgbGreen = static_cast<BYTE>(v * 255);
                        pRGBX->rgbRed = static_cast<BYTE>(p * 255);
                        break;
                    case 2:
                        pRGBX->rgbBlue = static_cast<BYTE>(p * 255);
                        pRGBX->rgbGreen = static_cast<BYTE>(v * 255);
                        pRGBX->rgbRed = static_cast<BYTE>(t * 255);
                        break;
                    case 3:
                        pRGBX->rgbBlue = static_cast<BYTE>(p * 255);
                        pRGBX->rgbGreen = static_cast<BYTE>(q * 255);
                        pRGBX->rgbRed = static_cast<BYTE>(v * 255);
                        break;
                    case 4:
                        pRGBX->rgbBlue = static_cast<BYTE>(t * 255);
                        pRGBX->rgbGreen = static_cast<BYTE>(p * 255);
                        pRGBX->rgbRed = static_cast<BYTE>(v * 255);
                        break;
                    case 5:
                        pRGBX->rgbBlue = static_cast<BYTE>(v * 255);
                        pRGBX->rgbGreen = static_cast<BYTE>(p * 255);
                        pRGBX->rgbRed = static_cast<BYTE>(q * 255);
                        break;
                    }
                }
            }
            else
            {
                if (depth > distance_Max)
                {
                    pRGBX->rgbRed = 0;
                    pRGBX->rgbGreen = 0;
                    pRGBX->rgbBlue = 0;
                }
                else
                {
                    float normalizedDepth = ((float)depth - 0) / (10000 - 0); 
                    normalizedDepth *= Zone_Arc;
                    BYTE intensity = static_cast<BYTE>(255 * (1.0f - normalizedDepth)); 
                    pRGBX->rgbRed = intensity;
                    pRGBX->rgbGreen = intensity;
                    pRGBX->rgbBlue = intensity;
                }
            }
        }
        else
        {
            pRGBX->rgbRed = 0;
            pRGBX->rgbGreen = 0;
            pRGBX->rgbBlue = 0;
        }

        ++pRGBX;
        ++pBuffer;
    }

    for (int i = 0; i < nWidth * nHeight; i++)
    {
        m_depthRGBArray[i] = (m_pDepthRGBX[i].rgbRed << 16) | (m_pDepthRGBX[i].rgbGreen << 8) | m_pDepthRGBX[i].rgbBlue;
    }
}

