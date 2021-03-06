#include "qhy6.h"
#include "qhycam.h"
#include <stdlib.h>
#include <string.h>

extern QUsb *qhyusb;
QHY6 *qhy6;

void QHY6::CorrectQHY6WH(int *w,int *h)
{
        if(*w <= 400 && *h <= 298)
	{
		*w = 400;
		*h = 298;
		initQHY6_400x298();
	}
	else
	{
		*w = 800;
		*h = 596;
		initQHY6_800x596();
	}
}

void QHY6::initQHY6_regs(void)
{
    qhyusb->ccdreg.devname = (char *)"QHY6-0";
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset = qhyusb->QCam.camOffset;
    qhyusb->ccdreg.Exptime = (unsigned long)qhyusb->QCam.camTime;       //unit: ms
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.SKIP_TOP = 0;
    qhyusb->ccdreg.SKIP_BOTTOM = 0;
    qhyusb->ccdreg.AMPVOLTAGE = 1;
    qhyusb->ccdreg.LiveVideo_BeginLine = 0;
    qhyusb->ccdreg.AnitInterlace = 0;
    qhyusb->ccdreg.MultiFieldBIN = 0;
    qhyusb->ccdreg.TgateMode = 0;
    qhyusb->ccdreg.ShortExposure = 0;
    qhyusb->ccdreg.VSUB = 0;
    qhyusb->ccdreg.TransferBIT = 0;
    qhyusb->ccdreg.TopSkipNull = 30;
    qhyusb->ccdreg.TopSkipPix = 0;
    qhyusb->ccdreg.DownloadCloseTEC = 0;
    qhyusb->ccdreg.SDRAM_MAXSIZE = 100;
    qhyusb->ccdreg.ClockADJ = 0x0000;
}

void QHY6::initQHY6_400x298(void)
{
    qhyusb->ccdreg.HBIN = 2;
    qhyusb->ccdreg.VBIN = 2;
    qhyusb->ccdreg.LineSize = 400;
    qhyusb->ccdreg.VerticalSize = 298;
    qhyusb->QCam.cameraW = 400;
    qhyusb->QCam.cameraH = 298;
    qhyusb->QCam.P_Size = 233*1024;
}

void QHY6::initQHY6_800x596(void)
{
    qhyusb->ccdreg.HBIN = 1;
    qhyusb->ccdreg.VBIN = 1;
    qhyusb->ccdreg.LineSize = 800;
    qhyusb->ccdreg.VerticalSize = 596;
    qhyusb->QCam.cameraW = 800;
    qhyusb->QCam.cameraH = 596;
    qhyusb->QCam.P_Size = 932*1024;

}


void QHY6::ConvertQHY6PRODataBIN12(unsigned char *Data)
{
    qhyusb->SWIFT_MSBLSB(Data,800,298);
}



void QHY6::ConvertQHY6PRODataBIN22(unsigned char *Data)
{
    qhyusb->SWIFT_MSBLSB(Data,400,298);
}

void QHY6::ConvertQHY6PRODataBIN11(unsigned char *Data)
{
    int x=800;
    int y=596;

    unsigned char * Buf = NULL;

    Buf=(unsigned char *)malloc(x*y*2);

    int j=0;
    int k=0;

    while(j < y)
    {
        memcpy(Buf+j*x*2, Data+k*x*2,           x*2);
	j++;
        memcpy(Buf+j*x*2, Data+k*x*2+(y/2)*x*2 ,  x*2);
	j++;
        k++;
    }
    memcpy(Data,Buf,x*y*2);
    free(Buf);

    qhyusb->SWIFT_MSBLSB(Data,x,y);
}

