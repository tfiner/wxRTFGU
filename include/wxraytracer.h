#ifndef _WXRAYTRACER_H_
#define _WXRAYTRACER_H_

/**
 * Ray Tracer skeleton
 *
 * Author : Sverre Kvaale <sverre@kvaale.com>
 * Version: 0.8
 *
 */

#include <wx/wx.h>
#include <IRenderer.h>

#include <vector>
#include <boost/shared_ptr.hpp>

using namespace std;


class wxraytracerFrame;
class RenderCanvas;
class RenderThread;

class RenderPixel;
typedef vector<RenderPixel> RenderPixels;

class World;
typedef boost::shared_ptr<World> WorldPtr;

class RenderThread : public wxThread, public IRenderer {
public:
   RenderThread(RenderCanvas* c, WorldPtr w) : wxThread(wxTHREAD_JOINABLE), world(w), canvas(c){}
   virtual void *Entry();
   virtual void OnExit();
   virtual void render(int x, int y, int red, int green, int blue);

private:
   void NotifyCanvas();

   WorldPtr world;
   RenderCanvas* canvas;

   RenderPixels pixels;
   wxStopWatch* timer;
   long lastUpdateTime;
};

typedef boost::shared_ptr<RenderThread> RenderThreadPtr;


class wxraytracerapp : public wxApp
{
public:
   virtual bool OnInit();
   virtual int OnExit();
   virtual void SetStatusText(const wxString&  text, int number = 0);

private:
   wxraytracerFrame *frame;
   DECLARE_EVENT_TABLE()
};

class wxraytracerFrame : public wxFrame
{
public:
   wxraytracerFrame(const wxPoint& pos, const wxSize& size);

   //event handlers
   void OnQuit( wxCommandEvent& event );
   void OnOpenFile( wxCommandEvent& event );
   void OnSaveFile( wxCommandEvent& event );
   void OnRenderStart( wxCommandEvent& event );
   void OnRenderCompleted( wxCommandEvent& event );
   void OnRenderPause( wxCommandEvent& event );
   void OnRenderResume( wxCommandEvent& event );

private:
   RenderCanvas *canvas; //where the rendering takes place
   wxString currentPath; //for file dialogues
   DECLARE_EVENT_TABLE()

   void renderStart();
};

//IDs for menu items
enum
{
   Menu_File_Quit = 100,
   Menu_File_Open,
   Menu_File_Save,
   Menu_Render_Start3_1,
   Menu_Render_Start3_2,
   Menu_Render_Pause,
   Menu_Render_Resume
};

class RenderCanvas: public wxScrolledWindow {
public:
   RenderCanvas(wxWindow *parent);
   virtual ~RenderCanvas();

   void SetImage(wxImage& image);
   wxImage GetImage();

   virtual void OnDraw(wxDC& dc);
   void renderStart(int id);
   void renderPause();
   void renderResume();
   void OnRenderCompleted( wxCommandEvent& event );
   void OnTimerUpdate( wxTimerEvent& event );
   void OnNewPixel( wxCommandEvent& event );

protected:
   wxBitmap *m_image;
   WorldPtr w;

private:
   RenderThreadPtr thread;
   wxStopWatch* timer;
   long pixelsRendered;
   long pixelsToRender;
   wxTimer updateTimer;

   DECLARE_EVENT_TABLE()
};


class RenderPixel
{
public:
   RenderPixel(int x, int y, int red, int green, int blue);

public:
   int x, y;
   int red, green, blue;
};


DECLARE_EVENT_TYPE(wxEVT_RENDER, -1)
#define ID_RENDER_COMPLETED 100
#define ID_RENDER_NEWPIXEL  101
#define ID_RENDER_UPDATE    102


#endif
