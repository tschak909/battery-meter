/**
 * @brief A Battery meter
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

char win_title[128] = "Battery Meter";

Display *display;
int screen;
Window win;
GC gc, clipgc;
Colormap colormap;
XEvent event;
bool done=false;
unsigned long black, white;
Atom wm_delete_window;

uint32_t battery_energy_now(void)
{
  FILE *fp = fopen("/sys/class/power_supply/BAT0/energy_now","r");
  char temp[16];
  
  if (!fp)
    {
      perror("battery_energy_now");
      exit(1);
    }

  fgets(temp,sizeof(temp),fp);
  
  fclose(fp);

  return atol(temp);
}

uint32_t battery_energy_full(void)
{
  FILE *fp = fopen("/sys/class/power_supply/BAT0/energy_full","r");
  char temp[16];
  
  if (!fp)
    {
      perror("battery_energy_full");
      exit(1);
    }
  
  fgets(temp,sizeof(temp),fp);

  fclose(fp);

  return atol(temp);
}

int battery_level(int w)
{
  FILE *fp;
  uint32_t ef, en;
  float p;
  char temp[16];
  
  en = battery_energy_now();
  ef = battery_energy_full();
  p = ((float)en / (float)ef) * (float)w;
  
  return (int)p;
}

void handle_expose(void)
{
  int x,y,w,h,bw,de;
  Window *winRoot;
  
  XGetGeometry(display, win, winRoot, &x, &y, &w, &h, &bw, &de);
  XClearWindow(display,win);
  XFillRectangle(display, win, gc, 0, 0, battery_level(w), h);
  XFlush(display);
}

void handle_client_message(void)
{
  printf("handle_client_message\n");
  done=true;
}

int main(int argc, char *argv[])
{
  int x,y,w,h;
  
  if (argc < 5)
    {
      printf("%s <xpos> <ypos> <width> <height>\n",argv[0]);
      return 1;
    }

  // Get parameters
  x = atoi(argv[1]);
  y = atoi(argv[2]);
  w = atoi(argv[3]);
  h = atoi(argv[4]);
  
  // Set up window.
  display=XOpenDisplay((char *)0);
  screen=DefaultScreen(display);
  black=BlackPixel(display,screen);
  white=WhitePixel(display,screen);
  win=XCreateSimpleWindow(display,DefaultRootWindow(display),x,y,w,h,5,white,black);
  XSetStandardProperties(display,win,win_title,win_title,None,NULL,0,NULL);
  XSelectInput(display,win,ExposureMask);
  gc=XCreateGC(display,win,0,0);
  colormap = DefaultColormap(display, DefaultScreen(display));
  XClearWindow(display,win);
  XMapRaised(display,win);	
  XSetBackground(display,gc,black);
  XSetForeground(display,gc,white);
  XMoveWindow(display,win,x,y);
  XSync(display,false);

  wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", false);

  XSetWMProtocols(display, win, &wm_delete_window, 1);
  
  while (!done)
    {
      if (!XPending(display))
	{
	  handle_expose();
	  usleep(100000);
	}
      else
	{
	  XNextEvent(display,&event);
	  
	  switch (event.type)
	    {
	    case ClientMessage:
	      if ((Atom)event.xclient.data.l[0] == wm_delete_window)
		handle_client_message();
	      break;
	    case Expose:
	      handle_expose();
	      break;
	    }
	  usleep(10000);
	}
    }
  
  XCloseDisplay(display);
  return 0;
}
