#include "stdafx.h"
#include "ProgressBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

ProgressBar::ProgressBar()
    : m_error( false )
    , m_total( 0 )
    , m_progress( 0 ) 
    , m_progressX( 0 )
{
}


ProgressBar::~ProgressBar()
{
}


BEGIN_MESSAGE_MAP(ProgressBar, CWnd)
	//{{AFX_MSG_MAP(ProgressBar)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void 
ProgressBar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
  paint( dc );
}


// Paint the progress bar in response to a paint message
void 
ProgressBar::paint( CDC &dc )
{
  paintBackground( dc );
  paintStatus( dc );
}


// Paint the background of the progress bar region
void 
ProgressBar::paintBackground( CDC &dc )
{
  CBrush brshBackground;
  CPen penShade( PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW) );
  CPen penLight( PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT) );

  VERIFY( brshBackground.CreateSolidBrush( ::GetSysColor (COLOR_BTNFACE) ) );

  dc.FillRect( m_bounds, &brshBackground );
  
  CPen *pOldPen = dc.SelectObject( &penShade );
  int xRight = m_bounds.left + m_bounds.Width() -1;
  int yBottom = m_bounds.top + m_bounds.Height() -1;
  {
    dc.MoveTo( m_bounds.left, m_bounds.top );
    dc.LineTo( xRight, m_bounds.top );

    dc.MoveTo( m_bounds.left, m_bounds.top );
    dc.LineTo( m_bounds.left, yBottom );
  }

  dc.SelectObject( &penLight );
  {
    dc.MoveTo( xRight, m_bounds.top );
    dc.LineTo( xRight, yBottom );

    dc.MoveTo( m_bounds.left, yBottom );
    dc.LineTo( xRight, yBottom );
  }

  dc.SelectObject( pOldPen );
}


// Paint the actual status of the progress bar
void 
ProgressBar::paintStatus( CDC &dc )
{
  if ( m_progress <= 0 )
    return;

  CBrush brshStatus;
  CRect rect( m_bounds.left, m_bounds.top, 
              m_bounds.left + m_progressX, m_bounds.bottom );

  COLORREF statusColor = getStatusColor();

  VERIFY( brshStatus.CreateSolidBrush( statusColor ) );

  rect.DeflateRect( 1, 1 );
  dc.FillRect( rect, &brshStatus );
}


// Paint the current step
void 
ProgressBar::paintStep( int startX, 
                        int endX )
{
  CRect redrawBounds( m_bounds.left + startX-1, m_bounds.top, 
                      m_bounds.left + endX, m_bounds.bottom );
  RedrawWindow( redrawBounds );
}


// Setup the progress bar for execution over a total number of steps
void 
ProgressBar::start( int total )
{
  m_total = total;
  reset ();
}


// Take one step, indicating whether it was a successful step
void 
ProgressBar::step( bool successful )
{
  m_progress++;

  int x = m_progressX;

  m_progressX = scale (m_progress);

  if ( !m_error  &&  !successful )
  {
    m_error = true;
    x = 1;
  }

  paintStep( x, m_progressX );
}


// Map from steps to display units
int 
ProgressBar::scale( int value )
{
  if ( m_total > 0 )
      return max( 1, value * (m_bounds.Width() - 1) / m_total );

  return value;
}


// Reset the progress bar
void 
ProgressBar::reset()
{
  m_progressX = 1;
  m_progress = 0;
  m_error = false;

  RedrawWindow( m_bounds );
  UpdateWindow( );
}


void 
ProgressBar::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

  GetClientRect( &m_bounds );
  m_progressX = scale (m_progress);
  Invalidate();
}


BOOL 
ProgressBar::OnEraseBkgnd( CDC *pDC )
{
  return FALSE;
}
