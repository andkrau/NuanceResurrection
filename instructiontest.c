/*
 * NUON instruction-set test harness.
 *
 * The actual tests live in nuontest.s; this file is just the C wrapper
 * that runs them against both the interpreter and the JIT compiler and
 * reports pass/fail on screen.  Add new tests to nuontest.s (extend the
 * SetTestNumber sequence) - no change should be needed here.
 *
 * =====================================================================
 * BUILD STEPS (Windows / git-bash):
 *
 *   # 1. Set up the NUON SDK env (in SDKFolder).
 *   #    env.sh is for Linux; on Windows in git-bash export the win32
 *   #    paths directly:
 *   export VMLABS="SDKFolder/vmlabs"
 *   export VMBLESSDIR="SDKFolder/bless"
 *   export BUILDHOST=WINDOWS_NT
 *   export PATH="$PATH:/SDKFolder/bin/win32:/SDKFolder/vmlabs/bin/win32"
 *
 *   # 2. (Optional) clean stale outputs so gmake actually rebuilds:
 *   rm -f nuontest.o instructiontest.cof cd_app.cof NUON.CD
 *
 *   # 3. Build.  The Makefile uses gmake (GNU Make 3.79.1 shipped with
 *   #    the SDK at vmlabs/bin/win32/gmake) - plain "make" won't be on
 *   #    PATH on Windows.
 *   gmake
 *
 * Outputs:
 *   nuontest.o          - assembled NUON test packets
 *   instructiontest.cof - linked test executable
 *   cd_app.cof          - coffpack-consolidated COFF
 *   NUON.CD             - bootable disc image (what the emulator loads)
 *
 * Run by loading NUON.CD in Release/Nuance.exe.  The on-screen status
 * line reports either "All instruction set tests passed" or the failing
 * test number.  Tests run twice (interpreter, then 5000 JIT iterations);
 * a mismatch between the two paths indicates either an interpreter bug
 * or a JIT/constant-propagation bug.
 *
 * If you only changed nuontest.s, gmake's mtime check is enough; you do
 * not need to wipe artifacts.  CreateNuonCD always re-runs, which is harmless.
 * =====================================================================
 */

#include <nuon/dma.h>
#include <nuon/bios.h>
#include <nuon/mml2d.h>
#include <nuon/mutil.h>
#include <nuon/msprintf.h>
#include <stdio.h>

#define YSTART			(25)	// Y-pos of top of color square

#define X_TEXT1			(55)	// X-pos of "Y" value display
#define Y_TEXT1_TOP		(25)	// Y-pos of top of "Y" value display area
#define Y_TEXT1_BOTTOM	(70)	// Y-pos of bottom of "Y" value display area

#define SCREENWIDTH		(720)
#define SCREENHEIGHT	(480)

mmlGC				gl_gc;
mmlSysResources 	gl_sysRes;
mmlDisplayPixmap	gl_screen;

unsigned int nuontest(unsigned int *scratchBuffer);
extern const char nuontest_version[];

int DoInstructionTest(unsigned int *scratchBuffer)
{
  return nuontest(scratchBuffer);
}

void text_background(mmlGC *gcP, mmlDisplayPixmap *screen)
{
	mmlColor clr;
	int yy, y;

	clr = mmlColorFromRGB( 40, 40, 40 );
	for( y = Y_TEXT1_BOTTOM; y >= YSTART; y-- )
	{
		yy = (y * 255) / SCREENHEIGHT;

		clr = mmlColorFromRGB( yy, yy, yy );
		_raw_plotpixel( screen->dmaFlags, screen->memP, X_TEXT1|(40<<16), y|(1<<16), clr );
	}
}

void fill_background(mmlGC *gcP, mmlDisplayPixmap *screen, unsigned int r, unsigned int g, unsigned int b)
{
  mmlColor clr;
  int x, y, w;

  clr = mmlColorFromRGB( r, g, b );

  for( y = SCREENHEIGHT; y >= 0; y-- )
  {
    for( x = 0; x < SCREENWIDTH; x+= 60 )
    {
      w = (SCREENWIDTH - x) > 60 ? 60 : (SCREENWIDTH - x);
      _raw_plotpixel( screen->dmaFlags, screen->memP, x|(w<<16), y|(1<<16), clr );
    }
  }
}

int main( )
{
	mmlAppPixmap source, clutSource;
	char buf[800];
	static unsigned int testResults[4*8];
	unsigned int i;
	int tellPos;
	FILE *testFile;
  int clrWhite = mmlColorFromRGB( 255, 255, 255 );
  int clrGreen = mmlColorFromRGB( 0, 255, 0 );
  int clrRed = mmlColorFromRGB( 255, 0, 0 );
  unsigned int testResult;
  
// Initialize the system resources and graphics context to a default state.
	mmlPowerUpGraphics( &gl_sysRes );
	mmlInitGC( &gl_gc, &gl_sysRes );

	// Initialize a single display pixmap as a framebuffer
	// 720 pixels wide by 480 lines tall, using 32 bit YCC-alpha pixels.
	mmlInitDisplayPixmaps( &gl_screen, &gl_sysRes, 720, 480, e888Alpha, 1, 0L );

	// show the sample pixmap
	mmlSimpleVideoSetup(&gl_screen, &gl_sysRes, eTwoTapVideoFilter);

	_DeviceDetect(1);

	while(1)
	{   
	  // Set all the pixels in the display pixmap
		fill_background(&gl_gc, &gl_screen, 0, 0, 0);

	msprintf(buf, "%s\n", nuontest_version);
	DebugWS(gl_screen.dmaFlags, gl_screen.memP, X_TEXT1 + 5, Y_TEXT1_TOP, clrWhite, buf );

	msprintf(buf, "Testing instruction set (interpreter)\n");
	  DebugWS(gl_screen.dmaFlags, gl_screen.memP, X_TEXT1 + 5, Y_TEXT1_TOP + 10, clrWhite, buf );

	  if((testResult = DoInstructionTest(testResults)) != 0)
	  {			
	    msprintf(buf, "Instruction set test %d FAILED! (interpreter)\n", testResult);
		  DebugWS(gl_screen.dmaFlags, gl_screen.memP, X_TEXT1 + 5, Y_TEXT1_TOP + 20, clrRed, buf );
	    while(1);
    }
    
    msprintf(buf, "All instruction set tests passed (interpreter)\n");
    DebugWS(gl_screen.dmaFlags, gl_screen.memP, X_TEXT1 + 5, Y_TEXT1_TOP + 30, clrWhite, buf );   
    msprintf(buf, "Testing instruction set (compiler)\n");
    DebugWS(gl_screen.dmaFlags, gl_screen.memP, X_TEXT1 + 5, Y_TEXT1_TOP + 40, clrWhite, buf );
    for(i = 0; i < 5000; i++)
    {
	  	if((testResult = DoInstructionTest(testResults)) != 0)
      {
        msprintf(buf, "Instruction set test #%d FAILED! (compiler)\n", testResult);                  
	    	DebugWS(gl_screen.dmaFlags, gl_screen.memP, X_TEXT1 + 5, Y_TEXT1_TOP + 60, clrRed, buf );
        while(1);
      }
      else
      {
		    msprintf(buf, "Instruction set tests passed (compiler iteration i)\n");
		    DebugWS(gl_screen.dmaFlags, gl_screen.memP, X_TEXT1 + 5, Y_TEXT1_TOP + 50, clrGreen, buf );      
		  }
    }
    msprintf(buf, "All compiler instruction test iterations passed!\n");
    DebugWS(gl_screen.dmaFlags, gl_screen.memP, X_TEXT1 + 5, Y_TEXT1_TOP + 70, clrGreen, buf );
		// Endless loop!
		while(1);
	}


	// Release allocated memory

	mmlReleasePixmaps( (mmlPixmap*)&gl_screen, &gl_sysRes, 1 );
	mmlReleasePixmaps( (mmlPixmap*)&source, &gl_sysRes, 1 );
	mmlReleasePixmaps( (mmlPixmap*)&clutSource, &gl_sysRes, 1 );

	return 0;
}
