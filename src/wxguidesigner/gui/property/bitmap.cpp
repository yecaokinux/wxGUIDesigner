/////////////////////////////////////////////////////////////////////////////
// Name:        wxguidesigner/gui/property/bitmap.cpp
// Purpose:     
// Author:      Andrea Zanellato
// Modified by: 
// Created:     2012-05-08
// RCS-ID:      $Hash$
// Copyleft:    (ɔ) Andrea Zanellato
// Licence:     GNU General Public License Version 3
/////////////////////////////////////////////////////////////////////////////
#include <wx/artprov.h>
#include <wx/filedlg.h>
#include <wx/filefn.h>
#include <wx/propgrid/propgrid.h>

#include <wx/log.h>

#include "wxguidesigner/gui/property/bitmapdialog.h"
#include "wxguidesigner/gui/property/bitmap.h"

namespace wxGD
{
namespace Property
{
    wxString BitmapLastDir     = wxEmptyString;
    int      BitmapFilterIndex = 0;
};
};
//=============================================================================
// wxGD::Property::BitmapDialogAdapter
//=============================================================================
bool wxGD::Property::BitmapDialogAdapter::DoShowDialog( wxPropertyGrid* propGrid,
                                                        wxPGProperty*   property )
{
    Bitmap *bmpProp = wxDynamicCast( property, Bitmap );

    if( bmpProp )
    {
        wxString      source = _("wxArtProvider");
        wxString      value  = bmpProp->GetValueAsString();
        wxArrayString params = wxStringTokenize( value, ";" );
        size_t        count  = params.GetCount();
        int        selection = 0;

        if( count )
        {
            source = params.Item(0);
            if( source == _("File") )
                selection = 1;
        }

        BitmapDialog dlg( propGrid->GetPanel(), selection );
        dlg.SetFilterIndex  ( wxGD::Property::BitmapFilterIndex );
        dlg.SetDirectory    ( wxGD::Property::BitmapLastDir );

        if( dlg.ShowModal() == wxID_OK )
        {
            selection = dlg.GetSelection();

            if( selection )
            {
                wxGD::Property::BitmapFilterIndex   = dlg.GetFilterIndex();
                wxGD::Property::BitmapLastDir       = dlg.GetDirectory();
                wxString    filePath                = dlg.GetPath();
                wxFileName  fileName                = filePath;

                if( fileName.FileExists() )
                {
                    wxBitmap bmp =
                    wxBitmap( fileName.GetFullPath(), wxBITMAP_TYPE_ANY );

                    if( bmp.IsOk() )
                    {
                        bmpProp->m_imgThumb = wxImage( bmp.ConvertToImage() );

                        value = _("File") + ";" + filePath;

                        SetValue( WXVARIANT( value ) );
                        return true;
                    }
                }
            }
            else
            {
                wxString artId  = dlg.GetArtId();
                wxString client = dlg.GetArtClient();
                wxBitmap bmp    = wxBitmap
                (
                    wxArtProvider::GetBitmap( artId, client, wxDefaultSize )
                );

                bmpProp->m_imgThumb = wxImage( bmp.ConvertToImage() );

                // Display the string without the "_C suffix"
                client = client.Truncate( client.length() - 2 );
                value = _("wxArtProvider") + ";" + artId + ";" + client;

                SetValue( WXVARIANT( value ) );
                return true;
            }
        }
    }

    return false;
}
//=============================================================================
// wxGD::Property::Bitmap
//=============================================================================
WX_PG_IMPLEMENT_PROPERTY_CLASS( wxGD::Property::Bitmap, wxPGProperty, wxString,
                                const wxString &, TextCtrlAndButton )

wxGD::Property::Bitmap::Bitmap( const wxString &label,
                                const wxString &name,
                                const wxString &value )
:
wxPGProperty( label, name ),
m_bmpThumb( wxNullBitmap ),
m_imgThumb( wxNullImage )
{
    wxString      newValue = wxEmptyString;
    wxArrayString params   = wxStringTokenize( value, ";" );
    size_t        count    = params.GetCount();

    if( count > 1 )
    {
        wxString source = params.Item(0);

        if( source == _("wxArtProvider") )
        {
            wxString artId  = params.Item(1);
            wxString client = wxART_OTHER;

            if( count > 2 )
                client = params.Item(2) + "_C";

            wxBitmap bmp = wxArtProvider::GetBitmap( artId, client );

            if( bmp.IsOk() )
            {
                newValue   = value;
                m_imgThumb = wxImage( bmp.ConvertToImage() );
            }
            else
            {
                wxLogDebug("Invalid art:%s %s", artId, client);
            }
        }
        else if( source == _("File") )
        {
            wxFileName fileName = params.Item(1);

            if( fileName.FileExists() )
            {
                wxString path = fileName.GetFullPath();
                wxBitmap bmp  = wxBitmap( path, wxBITMAP_TYPE_ANY );

                if( bmp.IsOk() )
                {
                    newValue    = value;
                    m_imgThumb  = wxImage( bmp.ConvertToImage() );
                }
            }
        }
    }
    else
    {
        wxLogDebug("Nothing to load");
    }

    m_value = WXVARIANT( newValue );
}

wxGD::Property::Bitmap::~Bitmap()
{
}

wxString wxGD::Property::Bitmap::ValueToString( wxVariant &value, int ) const
{
    if ( value.GetType() == wxPG_VARIANT_TYPE_STRING )
        return value.GetString();

    return wxEmptyString;
}

wxPGEditorDialogAdapter *wxGD::Property::Bitmap::GetEditorDialog() const
{
    return new BitmapDialogAdapter();
}
//=============================================================================
// Thumbnail drawing
//=============================================================================
wxSize wxGD::Property::Bitmap::OnMeasureImage( int ) const
{
    return wxPG_DEFAULT_IMAGE_SIZE;
}

void wxGD::Property::Bitmap::OnCustomPaint( wxDC &dc, const wxRect &rect,
                                                    wxPGPaintData & )
{
    if( !m_bmpThumb.IsNull() || m_imgThumb.IsOk() )
    {
        if( m_bmpThumb.IsNull() )
        {
            m_imgThumb.Rescale( rect.width, rect.height );
            m_bmpThumb = wxBitmap( m_imgThumb );
        }

        dc.DrawBitmap( m_bmpThumb, rect.x, rect.y, false );
    }
    else
    {
        // No source - just draw a white box
        dc.SetBrush( *wxWHITE_BRUSH );
        dc.DrawRectangle ( rect );
    }
}
