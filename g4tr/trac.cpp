#include "stdafx.h"
#include <string>
#include <iostream>
#include "PDI.h"
#include <vector>
#include <fstream>


CPDIdev		g_pdiDev;
CPDImdat    g_pdiMDat;
CPDIser		g_pdiSer;
DWORD		g_dwFrameSize;
BOOL		g_bCnxReady;
DWORD		g_dwStationMap;

HWND	g_hwnd = NULL;
#define BUFFER_SIZE 0x1FA400 
BYTE	g_pMotionBuf[BUFFER_SIZE];

std::vector<std::vector<float>> vertices;


VOID DisplayFrame(PBYTE pBuf, DWORD dwSize)
{
	
	TCHAR	szFrame[200];
	DWORD		i = 0;

	while (i<dwSize)
	{
		BYTE ucSensor = pBuf[i + 2];
		SHORT shSize = pBuf[i + 6];

		// skip rest of header
		i += 8;

		PDWORD pFC = (PDWORD)(&pBuf[i]);
		PFLOAT pPno = (PFLOAT)(&pBuf[i + 4]);

		_sntprintf(szFrame, _countof(szFrame), _T("%2d   %d  %+011.6f %+011.6f %+011.6f   %+011.6f %+011.6f %+011.6f\r\n"),
			ucSensor, *pFC, pPno[0], pPno[1], pPno[2], pPno[3], pPno[4], pPno[5]);

		std::vector<float> point;
		point.push_back(pPno[3]);
		point.push_back(pPno[4]);
		point.push_back(pPno[5]);
		vertices.push_back(point);


		i += shSize;
	}
}




void ParseG4NativeFrame( PBYTE pBuf, DWORD dwSize )
{
	if ((!pBuf) || (!dwSize))
	{}
	else
	{
		DWORD i= 0;
		LPG4_HUBDATA	pHubFrame;

		while (i < dwSize )
		{
			pHubFrame = (LPG4_HUBDATA)(&pBuf[i]);

			i += sizeof(G4_HUBDATA);

			UINT	nHubID = pHubFrame->nHubID;
			UINT	nFrameNum =  pHubFrame->nFrameCount;
			UINT	nSensorMap = pHubFrame->dwSensorMap;
			UINT	nDigIO = pHubFrame->dwDigIO;

			UINT	nSensMask = 1;



			for (int j=0; j<G4_MAX_SENSORS_PER_HUB; j++)
			{
				if (((nSensMask << j) & nSensorMap) != 0)
				{
					G4_SENSORDATA * pSD = &(pHubFrame->sd[j]);
					
					float x =pSD->pos[0];
					float y =pSD->pos[1];
					float z =pSD->pos[2];

					std::cout<< "x " << x << "y " <<y << "z " << z << "\n";

					std::vector<float> point;
					point.push_back(x);
					point.push_back(y);
					point.push_back(z);
					vertices.push_back(point);
					
				}
			}

		} // end while dwsize
	}
}














int main()
{

	CPDIg4 m_g4;
	BOOL bResult = m_g4.ConnectG4(_T("test.g4c"));


	/*CPDIg4 m_g4(_T("sample.g4c"));
	BOOL bResult = m_g4.ConnectG4();*/

	if (!bResult)
	{
		std::cout << "Can't connect to tracker \n";
	}
	else
	{
	std::cout<< "Successfully connected \n";
	}

	int track=0;
    while(track==0)
	{
		int control;
		std::cout << "Press 1 to collect or 0 to exit \n " ;
		std::cin >> control ;
		
		if(control==1)	
		{
			BOOL bExit = FALSE;

			PBYTE pBuf;
			DWORD dwSize;

			std::cout << "\n";

				if (!(m_g4.ReadSinglePnoBufG4(pBuf, dwSize)))
				{
						//DisplayFrame(pBuf, dwSize);
					//ParseFrame(pBuf, dwSize);
					std::cout<<"No frame found \n";
					bExit = TRUE;
				}
				else if ((pBuf == 0) || (dwSize == 0))
				{
					std::cout<<"empty \n";
				}
				else
				{

					ParseG4NativeFrame( pBuf,dwSize );
				}	
						
		


		}


		else if(control==0) break;


	}
	

	std::ofstream myfile("points.obj");
	if (myfile.is_open())
	{
		for (int k = 0; k < vertices.size(); ++k)
		{
			std::cout << "v " << vertices[k][0] << " " << vertices[k][1] << " " << vertices[k][2] << "\n";
			myfile << "v " << vertices[k][0] << " " << vertices[k][1] << " " << vertices[k][2] << "\n";
		}
		myfile.close();
	}
	else std::cout << "Unable to open file\n";


return 0;
}

