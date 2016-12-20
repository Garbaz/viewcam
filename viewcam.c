#include <stdlib.h>
#include <stdio.h>
#include <cv.h>
#include <highgui.h>
#include <signal.h>
#include <unistd.h>

#define KEY_ESC 0x10001b
#define KEY_NONE 0xffffffff
#define KEY_CAM 0x1018ff8f
#define TEXTBUFFERSIZE 256

#define SBLUE(s)  s.val[0]
#define SGREEN(s) s.val[1]
#define SRED(s)   s.val[2]

#define DBLUE(x,y)  (imgdata + y * imgstep)[x * imgchannels + 0]
#define DGREEN(x,y) (imgdata + y * imgstep)[x * imgchannels + 1]
#define DRED(x,y)   (imgdata + y * imgstep)[x * imgchannels + 2]

#define resetPrint() currTextY = 20

void mouseHandler(int event, int x, int y, int flags, void *params);
void printNextLine(char *str);
void printCross(int x, int y, int len, int red, int green, int blue, int thickness);

void getExtremePoint(int *x, int *y, int (*function)(int,int));
int exBrightness(int x, int y);
int exDarkest(int x, int y);

struct sigaction sa;
CvCapture* video;
IplImage* img;
uchar *imgdata;
char textBuffer[TEXTBUFFERSIZE], mouseDown, freeze, showBrightest, showDarkest;
int keycode, selectedPixelX, selectedPixelY, lineWidth, mouseParam, mouseX, mouseY, imgheight, imgwidth, imgstep, imgchannels, currTextY,
    brightestX, brightestY, darkestX, darkestY;

char winTitle[] = "View Cam (Device: ";
//CvScalar s;
CvFont font;
double hScale, vScale;

void interrupt_handler(int sig)
{
	printf("| Cleaning up and exiting...\n");
	cvReleaseImage(&img);
	cvDestroyWindow(winTitle);
	exit(0);
}

int main(int argc, char *argv[])
{
	
	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s VIDEO_DEVICE_NUMBER\n", argv[0]);
		return 1;
	}
	
	sa.sa_handler = interrupt_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction");
		return 2;
	}
	
	hScale = 0.5;
	vScale = 0.5;
	lineWidth = 1;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth, 8);
	
	video = cvCaptureFromCAM(0);
	img = 0;
	
	strcat(winTitle, argv[1]);
	strcat(winTitle, ")");
	cvNamedWindow(winTitle, CV_WINDOW_AUTOSIZE); 
	cvMoveWindow(winTitle, 0,0);
	
	mouseParam=5;
	cvSetMouseCallback(winTitle,mouseHandler,&mouseParam);
	
	if(!cvGrabFrame(video)){
		printf("Could not load frame\n");
		return 1;
	}
	img = cvRetrieveFrame(video,strtol(argv[0],NULL,10));
	
	imgheight	= img->height;
	imgwidth		= img->width;
	imgstep		= img->widthStep;
	imgchannels	= img->nChannels;
	imgdata = (uchar*) img->imageData;
	
	printf("Usage:\n");
	printf("+---------+-------------------+\n");
	printf("| [KEY]   | [Action]          |\n");
	printf("+---------+-------------------+\n");
	printf("| ESC     | Quit              |\n");
	printf("| 'Q'     | Quit              |\n");
	printf("| LMOUSE  | Choose point      |\n");
	printf("| 'C'     | Clear point       |\n");
	printf("| 'F'     | (Un-)Freeze image |\n");
	printf("| CAMKEY  | Freeze image      |\n");
	printf("| 'B'     | Show brightest Pt |\n");
	printf("| 'N'     | Show darkest Pt   |\n");
	printf("+---------+-------------------+\n");
	while(1)
	{
		if(!freeze)
		{
			if(!cvGrabFrame(video)){
				printf("Could not load frame\n");
				return 1;
			}
		}
		
		img = cvRetrieveFrame(video,0);
		
		if(mouseDown)
		{
			selectedPixelX = mouseX;
			selectedPixelY = mouseY;
		}
		
		if(showBrightest)
		{
			getExtremePoint(&brightestX, &brightestY, &exBrightness);
		}
		
		if(showDarkest)
		{
			getExtremePoint(&darkestX, &darkestY, &exDarkest);
		}
		
		if(selectedPixelX | selectedPixelY != 0)
		{
			resetPrint();
			
			sprintf(textBuffer, "PIXEL (%i, %i):", selectedPixelX, selectedPixelY);
			printNextLine(textBuffer);
			
			sprintf(textBuffer, "Red (c2): %i", DRED(selectedPixelX, selectedPixelY));
			printNextLine(textBuffer);
			sprintf(textBuffer, "Green (c1): %i", DGREEN(selectedPixelX, selectedPixelY));
			printNextLine(textBuffer);
			sprintf(textBuffer, "Blue (c0): %i", DBLUE(selectedPixelX, selectedPixelY));
			printNextLine(textBuffer);
			
			sprintf(textBuffer, "Brightness (c0+c1+c2): %i", DRED(selectedPixelX,selectedPixelY) + DGREEN(selectedPixelX,selectedPixelY) + DBLUE(selectedPixelX,selectedPixelY));
			printNextLine(textBuffer);
			sprintf(textBuffer, "avg. Brightness (Brightness/3): %i", (DRED(selectedPixelX,selectedPixelY) + DGREEN(selectedPixelX,selectedPixelY) + DBLUE(selectedPixelX,selectedPixelY))/3);
			printNextLine(textBuffer);
			
			printCross(selectedPixelX, selectedPixelY, 3, 0, 0, 0, 1);
		}
		
		if(showBrightest)
		{
			printCross(brightestX, brightestY, 4, 0, 0, 255, 1);
		}
		if(showDarkest)
		{
			printCross(darkestX, darkestY, 4, 255, 0, 0, 1);
		}
		
		cvShowImage(winTitle, img);
		
		keycode = cvWaitKey(20);
		if(keycode == KEY_ESC || (char) keycode == 'q')
		{
			break;
		}
		else if((char)keycode == 'c')
		{
			selectedPixelX = selectedPixelY = 0;
		}
		else if((char)keycode == 'f' || keycode == KEY_CAM)
		{
			freeze = !freeze;
		}
		else if((char)keycode == 'b')
		{
			showBrightest = !showBrightest;
		}
		else if((char)keycode == 'n')
		{
			showDarkest = !showDarkest;
		}
		else if(keycode != KEY_NONE)
		{
			printf("Keypress: hex=0x%x, dec=%i, char=%c\n", keycode, keycode, (char) keycode);
		}
	}
	
	cvDestroyWindow(winTitle);
	cvReleaseImage(&img);
	return 0;
}

void mouseHandler(int event, int x, int y, int flags, void* params)
{
	mouseX = x;
	mouseY = y;
	switch(event)
	{
		case CV_EVENT_LBUTTONDOWN:
			mouseDown = 1;
			break;
		case CV_EVENT_LBUTTONUP:
			mouseDown = 0;
			selectedPixelX = mouseX;
			selectedPixelY = mouseY;
			break;
	}
}

void printNextLine(char *str)
{
	cvPutText(img,str,cvPoint(10,currTextY), &font, cvScalar(0,0,0,0));
	currTextY += 20;
}

void getExtremePoint(int *x, int *y, int (*function)(int,int))
{
	int exV, exX, exY;
	int currVal;
	for(int i=0;i<imgwidth;i++)
	{
		for(int j=0;j<imgheight;j++)
		{
			currVal = function(i, j);
			if(currVal > exV)
			{
				exX = i;
				exY = j;
				exV = currVal;
			}
		}
	}
	*x = exX;
	*y = exY;
}

int exBrightness(int x, int y)
{
	return DRED(x,y) + DGREEN(x,y) + DBLUE(x,y);
}

int exDarkest(int x, int y)
{
	return (255-DRED(x,y)) + (255-DGREEN(x,y)) + (255-DBLUE(x,y));
}

void printCross(int x, int y, int len, int red, int green, int blue, int thickness)
{
	cvLine(img, cvPoint(x-len,y), cvPoint(x+len,y), cvScalar(blue,green,red,0), thickness, 8, 0);
	cvLine(img, cvPoint(x,y-len), cvPoint(x,y+len), cvScalar(blue,green,red,0), thickness, 8, 0);
}
