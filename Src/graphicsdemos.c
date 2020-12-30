#include <PalmOS.h>
#include <System/SystemPublic.h>
#include <UI/UIPublic.h>
#include "graphicsdemos.h"

#define WIDTH 160
#define HEIGHT 120
#define YOFFSET 15
#define NUM_BUFFERS 8
#define IMG_SIZE 16
#define PADDING 30

#define MANDEL_MAX_ITERS 128

/*
 * Should probably have:
 * - infinite sprites
 * - Mandelbrot set
 * - Barnsley fern
 * - Rotating cube / Menger sponge
 * - Raytracing
 */

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	UInt16 err;
	EventType e;
	FormType *pfrm;
	Boolean running = true;
	UInt16 currentFormID = 0;
	
	RectangleType sourceBounds;
	WinHandle* buffers;
	WinHandle oldDrawWinH;
	UInt16 bufferIndex;
	RGBColorType blue = {0, 0, 0, 255};
	RGBColorType cyan = {0, 0, 255, 255};
	
	int x = WIDTH / 2;
	int y = HEIGHT / 2;
	int dx = 16;
	int dy = 16;

	void fakeLissajous() {
		x += dx / 4;
		y += dy / 4;
		
		if (x < PADDING) {
			dx++;
		}
		
		if (x > WIDTH - PADDING - IMG_SIZE) {
			dx--;
		}
		
		if (y < PADDING) {
			dy++;
		}
		
		if (y > HEIGHT - PADDING - IMG_SIZE) {
			dy--;
		}
		
		MemHandle handle = DmGetResource(bitmapRsc, BitmapCat);
		
		BitmapType* bmp = MemHandleLock(handle);
		
		WinDrawBitmap(bmp,x,y);
		
		MemHandleUnlock(handle);
		DmReleaseResource(handle);
	}

	/**
	 * create off screen buffers
	 */
	void createScreenBuffers() {
		UInt16 error;
		WinHandle oldDrawWinH;
		oldDrawWinH = WinGetDrawWindow();
		sourceBounds.topLeft.x = sourceBounds.topLeft.y = 0;
		sourceBounds.extent.x = WIDTH;
		sourceBounds.extent.y = HEIGHT;
		
		buffers = MemPtrNew(sizeof(oldDrawWinH) * NUM_BUFFERS);
		
		for (int i = 0; i < NUM_BUFFERS; i++) {
			buffers[i] = WinCreateOffscreenWindow
				(WIDTH, HEIGHT, screenFormat, &error);
			WinSetDrawWindow(buffers[i]);
			WinEraseRectangle(&sourceBounds, 0);
		}
		WinSetDrawWindow(oldDrawWinH);
	}

	void startDrawOffscreen() {
		oldDrawWinH = WinGetDrawWindow();
		WinSetDrawWindow(buffers[bufferIndex]);
	}
	
	void destroyScreenBuffers() {
		MemPtrFree(buffers);
	}

	void endDrawOffscreen() {
		WinSetDrawWindow(oldDrawWinH);
	}

	void flipDisplay() {
		WinCopyRectangle
			(buffers[bufferIndex], 0, &sourceBounds, 0, YOFFSET, winPaint);
		bufferIndex = (bufferIndex + 1) % NUM_BUFFERS;
	}

	if (cmd == sysAppLaunchCmdNormalLaunch)			// Make sure only react to NormalLaunch, not Reset, Beam, Find, GoTo...
	{
		FrmGotoForm(InfSprForm);
		
		createScreenBuffers();

		while(running) 
		{
			EvtGetEvent(&e, 3);
			if (SysHandleEvent(&e)) 
				continue;
			if (MenuHandleEvent((void *)0, &e, &err)) 
				continue;
	
			switch (e.eType) 
			{
				case ctlSelectEvent:
					switch (e.data.ctlSelect.controlID) {
						case ButtonNext:
							if (currentFormID < LastForm) {
								FrmGotoForm(currentFormID + 1);
							} else {
								FrmGotoForm(FirstForm);
							}
							break;
						case ButtonPrev:
							if (currentFormID > FirstForm) {
								FrmGotoForm(currentFormID - 1);
							} else {
								FrmGotoForm(LastForm);
							}
							break;
					}
					break;

				case frmLoadEvent:
					FrmSetActiveForm(FrmInitForm(e.data.frmLoad.formID));
					currentFormID = e.data.frmLoad.formID;
					break;

				case frmOpenEvent:
					pfrm = FrmGetActiveForm();
					endDrawOffscreen();
					FrmDrawForm(pfrm);
					if (currentFormID == MandelbrotForm) {
						double foo = 1.23456789;
						double bar = 4.567890;
						
						FlpCompDouble fooComp;
						FlpCompDouble barComp;
						
						fooComp.d = foo;
						barComp.d = bar;
						
						if (_d_cmp(fooComp.fd, barComp.fd) == flpLess) {
							WinDrawChars("foo < bar", 9, 0, 20);
						} else {
							WinDrawChars("bar < foo", 9, 0, 20);
						}
					}
					break;

				case menuEvent:
					break;

				case appStopEvent:
					running = false;
					break;

				default:
					if (FrmGetActiveForm())
						FrmHandleEvent(FrmGetActiveForm(), &e);
			}
			if (currentFormID == InfSprForm) {
				startDrawOffscreen();
				fakeLissajous();
				endDrawOffscreen();
				flipDisplay();
			}
		}
		destroyScreenBuffers();
		FrmCloseAllForms();
	}

	return 0;
}

UInt32 __attribute__((section(".vectors"))) __Startup__(void)
{
	SysAppInfoPtr appInfoP;
	void *prevGlobalsP;
	void *globalsP;
	UInt32 ret;
	
	SysAppStartup(&appInfoP, &prevGlobalsP, &globalsP);
	ret = PilotMain(appInfoP->cmd, appInfoP->cmdPBP, appInfoP->launchFlags);
	SysAppExit(appInfoP, prevGlobalsP, globalsP);
	
	return ret;
}
