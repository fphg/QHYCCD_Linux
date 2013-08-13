#include   <string.h> 
#include   <pthread.h> 
#include   <unistd.h> 
#include   <stdio.h> 
#include   <stdlib.h> 
#include   <errno.h> 
#include   <termios.h> 
#include   <sys/types.h> 
#include   <sys/time.h> 
#include   <sys/stat.h> 
#include   <fcntl.h> 
#include   <sys/ioctl.h> 
#include   <signal.h> 
#include   <opencv/cv.h>
#include   <opencv/highgui.h>
#include   <time.h>

int port_handle = 0;


int   open_port() 
{ 
    int port_handle = 0;   
    struct termios options; 
    int status; 

    char *portname = "/dev/ttyS0";

    port_handle = open(portname,O_RDWR | O_NOCTTY | O_NDELAY);
     
    if(port_handle == -1)
    { 
        perror( "open serial failed\n"); 
        exit(1); 
    }
    else
    { 
        /*restore   async   mode,   use   block   mode   */ 
        fcntl(port_handle,F_SETFL,0); 

        /*configure this serial port 9600,N,8,1*/ 
        tcgetattr(port_handle,&options); 
        cfsetispeed(&options,B9600); 
        cfsetospeed(&options,B9600); 

        /*start the read and local mode*/ 
        options.c_cflag |= (CLOCAL | CREAD); 

        /*set parity and other options*/ 
        options.c_cflag &= ~PARENB; 
        options.c_iflag &= ~INPCK;   
        options.c_cflag &= ~CSTOPB; 
        options.c_cflag &= ~CSIZE; 
        options.c_cflag |= CS8; 

        /*close hardware flow control*/ 
        options.c_cflag &= ~CRTSCTS; 

        /*close software flow control*/ 
        options.c_iflag &= ~(IXON | IXOFF | IXANY); 
        options.c_iflag &= ~ICRNL; 
        options.c_cc[VMIN] = 0;   
        options.c_cc[VTIME] = 10;   

        /*select raw input mode*/ 
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 

        /*select raw output mode*/ 
        options.c_oflag &= ~OPOST; 

        tcsetattr(port_handle,TCSANOW,&options); 
    } 
    return port_handle; 
} 


void send2speaker()
{
    unsigned char buf[7];
    buf[0] = '#';
    buf[1] = '#';
    buf[2] = 'A';
    buf[5] = '&';
    buf[6] = '&';
    write(port_handle,buf,7);
}

void send2oled(int width,int height,int bpp,int channels,unsigned char *showdata)
{
    unsigned char data[132 * 32];
    memset(data,0,132 * 32);

    //this is final image to oled
    IplImage *img = cvCreateImage(cvSize(132,32),8,1);

    //this is a temp image to load source image
    IplImage *srcimg = cvCreateImage(cvSize(width,height),bpp,1);

    //ensure image is a mono data
    if(channels == 3)
    {
        IplImage *colorimg = cvCreateImage(cvSize(width,height),bpp,3);
        colorimg->imageData = (char *)showdata;
        cvCvtColor(colorimg,srcimg,CV_BGR2GRAY); 
        cvReleaseImage(&colorimg);       
    }
    else if(channels == 1)
    {
        srcimg->imageData = (char *)showdata;
    }
  
    //first to resize the source image to 132*32
    cvResize(srcimg,img,CV_INTER_NN);
    
    //just flip the image to fit oled
    cvFlip(img,0,-1);

    unsigned char data8[533];

    int headlen = 3;
    int e = 0;
	int s = 0;
    int k = 0;
    int i;
	CvScalar pix;
    memset(data8,0,533);

    data8[0] = '#';
    data8[1] = '#';
    data8[2] = 'B';
    e = 0;
    s = headlen;
    for(k = 0;k < 132;k++)
    {
        while(e < 8)
        {
            pix = cvGet2D(img,e,k);

            if(pix.val[0] > 128)
            {
                data8[s] = data8[s] | (1 << e);
            }
            e++;
         }
         e = 0;
         s++;
    }

    e = 8;
    s = 132 + headlen;
    for(k = 0;k < 132;k++)
    {
        while(e < 16)
	{
            pix = cvGet2D(img,e,k);

	    if(pix.val[0] > 128)
	    {
                data8[s] = data8[s] | (1 << (e%8));
	    }
            e++;
         }
	 e = 8;
         s++;
    }

    e = 16;
    s = 264 + headlen;
    for(k = 0;k < 132;k++)
    {
        while(e < 24)
	{
            pix = cvGet2D(img,e,k);

	    if(pix.val[0] > 128)
	    {
                data8[s] = data8[s] | (1 << (e%16));
	    }
	    e++;
        }
        e = 16;
        s++;
    }
    e = 24;
    s = 396 + headlen;
    for(k = 0;k < 132;k++)
    {
        while(e < 32)
	{
            pix = cvGet2D(img,e,k);

	    if(pix.val[0] > 128)
	    {
                data8[s] = data8[s] | (1 << (e % 24));
	    }
	    e++;
        }
        e = 24;
        s++;
    }
    data8[531] = '&';
    data8[532] = '&';
    write(port_handle,data8,533); 
    cvReleaseImage(&srcimg);
    cvReleaseImage(&img);
}

int   main(int   argc,   char   *   argv[])   
{ 
    char read_buf[256];
    port_handle = open_port();

    IplImage *img = cvLoadImage("pad.jpg",0);
    send2oled(img->width,img->height,img->depth,img->nChannels,img->imageData);
    cvReleaseImage(&img);
    send2speaker();
        
    while(1)
    {
        bzero(read_buf, sizeof(read_buf));

        read(port_handle, read_buf, sizeof(read_buf));
        printf("%d\n",read_buf[0]); 
#if 0
        if(read_buf[0] == '0')
        {
            send2oled("0");
            send2oled("0");
        }
        else if(read_buf[0] == '1')
        {
            send2speaker();
        }
        else if(read_buf[0] == '2')
        {
            send2oled("2");
            send2oled("2");
        }
        else if(read_buf[0] == '3')
        {
            send2oled("3");
            send2oled("3");
        }
        else if(read_buf[0] == '4')
        {
            send2oled("4");
            send2oled("4");
        }
 #endif
        usleep(100);
    }
    close(port_handle); 

    return   0   ; 
}