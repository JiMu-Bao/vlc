/*****************************************************************************
 * familiar_callbacks.c : Callbacks for the Familiar Linux Gtk+ plugin.
 *****************************************************************************
 * Copyright (C) 2000, 2001 VideoLAN
 * $Id: familiar_callbacks.c,v 1.6.2.6 2002/10/02 20:12:02 jpsaman Exp $
 *
 * Authors: Jean-Paul Saman <jpsaman@wxs.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

/*****************************************************************************
 * Preamble
 *****************************************************************************/
#include <sys/types.h>                                              /* off_t */
#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <videolan/vlc.h>

#include "stream_control.h"
#include "input_ext-intf.h"

#include "interface.h"
#include "intf_playlist.h"

#include "video.h"
#include "video_output.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "familiar_callbacks.h"
#include "familiar_interface.h"
#include "familiar_support.h"
#include "familiar.h"

#include "netutils.h"

static void MediaURLOpenChanged( GtkWidget *widget, gchar *psz_url );
static char* get_file_perm(const char *path);

/*****************************************************************************
 * Useful function to retrieve p_intf
 ****************************************************************************/
void * __GtkGetIntf( GtkWidget * widget )
{
    void *p_data;

    if( GTK_IS_MENU_ITEM( widget ) )
    {
        /* Look for a GTK_MENU */
        while( widget->parent && !GTK_IS_MENU( widget ) )
        {
            widget = widget->parent;
        }

        /* Maybe this one has the data */
        p_data = gtk_object_get_data( GTK_OBJECT( widget ), "p_intf" );
        if( p_data )
        {
            return p_data;
        }

        /* Otherwise, the parent widget has it */
        widget = gtk_menu_get_attach_widget( GTK_MENU( widget ) );
    }

    /* We look for the top widget */
    widget = gtk_widget_get_toplevel( GTK_WIDGET( widget ) );

    p_data = gtk_object_get_data( GTK_OBJECT( widget ), "p_intf" );

    return p_data;
}

/*****************************************************************************
 * Helper functions for URL changes in Media and Preferences notebook pages.
 ****************************************************************************/
static void MediaURLOpenChanged( GtkWidget *widget, gchar *psz_url )
{
    intf_thread_t *p_intf = GtkGetIntf( widget );
    int            i_end = p_main->p_playlist->i_size;

    // Add p_url to playlist .... but how ?
    if( p_main->p_playlist )
    {
       intf_PlaylistAdd( p_main->p_playlist, PLAYLIST_END, (char*)psz_url );
    }

    if (p_intf->p_sys->b_autoplayfile)
    {
       intf_ErrMsg("autoplay selected");
       /* end current item, select added item  */
       if( p_input_bank->pp_input[0] != NULL )
       {
           p_input_bank->pp_input[0]->b_eof = 1;
       }
       intf_PlaylistJumpto( p_main->p_playlist, i_end - 1 );

       if( p_input_bank->pp_input[0] != NULL )
       {
           input_SetStatus( p_input_bank->pp_input[0], INPUT_STATUS_PLAY );
           p_main->p_playlist->b_stopped = 0;
       }
    }
}

/*****************************************************************
 * Read directory helper function.
 ****************************************************************/
void ReadDirectory( GtkCList *clist, char *psz_dir )
{
    intf_thread_t *p_intf = GtkGetIntf( clist );
    struct dirent **namelist;
    int n=-1;
    int status=-1;

    if (psz_dir)
    {
       status = chdir(psz_dir);
    }
    if (status<0)
      intf_ErrMsg("File is not a directory.");
    else
      n = scandir(".", &namelist, 0, NULL);

    if (n<0)
        perror("scandir");
    else
    {
        gchar *ppsz_text[2];
        int i;

        gtk_clist_freeze( p_intf->p_sys->p_clist );
        gtk_clist_clear( p_intf->p_sys->p_clist );
        for (i=0; i<n; i++)
        {
            /* This is a list of strings. */
            ppsz_text[0] = namelist[i]->d_name;
            ppsz_text[1] = get_file_perm(namelist[i]->d_name);
            if (strcmp(ppsz_text[1],"") == 0)
                intf_ErrMsg("File system error unknown filetype encountered.");
            gtk_clist_insert( p_intf->p_sys->p_clist, i, ppsz_text );
            free(namelist[i]);
        }
        gtk_clist_thaw( p_intf->p_sys->p_clist );
        free(namelist);
    }
}

static char* get_file_perm(const char *path)
{
    struct stat st;
    char *perm;

    perm = (char *) malloc(sizeof(char)*10);
    strncpy( perm, "----------", sizeof("----------"));
    if (lstat(path, &st)==0)
    {
        if (S_ISLNK(st.st_mode))
            perm[0]= 'l';
        else if (S_ISDIR(st.st_mode))
            perm[0]= 'd';
        else if (S_ISCHR(st.st_mode))
            perm[0]= 'c';
        else if (S_ISBLK(st.st_mode))
            perm[0]= 'b';
        else if (S_ISFIFO(st.st_mode))
            perm[0]= 'f';
        else if (S_ISSOCK(st.st_mode))
            perm[0]= 's';
        else if (S_ISREG(st.st_mode))
            perm[0]= '-';
        else /* Unknown type is an error */
            perm[0]= '?';
        /* Get file permissions */
        /* User */
        if (st.st_mode & S_IRUSR)
            perm[1]= 'r';
        if (st.st_mode & S_IWUSR)
            perm[2]= 'w';
        if (st.st_mode & S_IXUSR)
        {
            if (st.st_mode & S_ISUID)
                perm[3] = 's';
            else
                perm[3]= 'x';
        }
        else if (st.st_mode & S_ISUID)
            perm[3] = 'S';
        /* Group */
        if (st.st_mode & S_IRGRP)
            perm[4]= 'r';
        if (st.st_mode & S_IWGRP)
            perm[5]= 'w';
        if (st.st_mode & S_IXGRP)
        {
            if (st.st_mode & S_ISGID)
                perm[6] = 's';
            else
                perm[6]= 'x';
        }
        else if (st.st_mode & S_ISGID)
            perm[6] = 'S';
        /* Other */
        if (st.st_mode & S_IROTH)
            perm[7]= 'r';
        if (st.st_mode & S_IWOTH)
            perm[8]= 'w';
        if (st.st_mode & S_IXOTH)
        {
            // 'sticky' bit
            if (st.st_mode &S_ISVTX)
                perm[9] = 't';
            else
                perm[9]= 'x';
        }
        else if (st.st_mode &S_ISVTX)
            perm[9]= 'T';
    }
    return perm;
}

/*
 * Main interface callbacks
 */

gboolean GtkExit( GtkWidget       *widget,
                  gpointer         user_data )
{
    intf_thread_t *p_intf = GtkGetIntf( widget );

    vlc_mutex_lock( &p_intf->change_lock );
    p_intf->b_die = 1;
    vlc_mutex_unlock( &p_intf->change_lock );

    return TRUE;
}

void
on_toolbar_open_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GtkGetIntf( button );

    if (p_intf->p_sys->p_notebook)
    {
       gtk_widget_show( GTK_WIDGET(p_intf->p_sys->p_notebook) );
       gtk_notebook_set_page(p_intf->p_sys->p_notebook,0);
    }
    gdk_window_raise( p_intf->p_sys->p_window->window );
    if (p_intf->p_sys->p_clist)
    {
       ReadDirectory(p_intf->p_sys->p_clist, ".");
    }
}


void
on_toolbar_preferences_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GtkGetIntf( button );

    if (p_intf->p_sys->p_notebook)
    {
       gtk_widget_show( GTK_WIDGET(p_intf->p_sys->p_notebook) );
       gtk_notebook_set_page(p_intf->p_sys->p_notebook,1);
    }
    gdk_window_raise( p_intf->p_sys->p_window->window );
}


void
on_toolbar_rewind_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
//    intf_thread_t *  p_intf = GtkGetIntf( button );

    if( p_input_bank->pp_input[0] != NULL )
    {
        input_SetStatus( p_input_bank->pp_input[0], INPUT_STATUS_SLOWER );
    }
}


void
on_toolbar_pause_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
//    intf_thread_t *  p_intf = GtkGetIntf( button );

    if( p_input_bank->pp_input[0] != NULL )
    {
        input_SetStatus( p_input_bank->pp_input[0], INPUT_STATUS_PAUSE );
    }
}


void
on_toolbar_play_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *  p_intf = GtkGetIntf( button );


    if( p_input_bank->pp_input[0] != NULL )
    {
        input_SetStatus( p_input_bank->pp_input[0], INPUT_STATUS_PLAY );
        p_main->p_playlist->b_stopped = 0;
        gdk_window_lower( p_intf->p_sys->p_window->window );
    }
    else
    {
        vlc_mutex_lock( &p_main->p_playlist->change_lock );

        if( p_main->p_playlist->b_stopped )
        {
            if( p_main->p_playlist->i_size )
            {
                vlc_mutex_unlock( &p_main->p_playlist->change_lock );
                intf_PlaylistJumpto( p_main->p_playlist,
                                     p_main->p_playlist->i_index );
            }
            else
            {
                vlc_mutex_unlock( &p_main->p_playlist->change_lock );
                /* simulate on open button click */
                on_toolbar_open_clicked(button,user_data);
            }
        }
        else
        {

            vlc_mutex_unlock( &p_main->p_playlist->change_lock );
        }

    }
}


void
on_toolbar_stop_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *  p_intf = GtkGetIntf( button );

    if( p_input_bank->pp_input[0] != NULL )
    {
        /* end playing item */
        p_input_bank->pp_input[0]->b_eof = 1;

        /* update playlist */
        vlc_mutex_lock( &p_main->p_playlist->change_lock );

        p_main->p_playlist->i_index--;
        p_main->p_playlist->b_stopped = 1;

        vlc_mutex_unlock( &p_main->p_playlist->change_lock );
        gdk_window_raise( p_intf->p_sys->p_window->window );
    }
}


void
on_toolbar_forward_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
//    intf_thread_t *  p_intf = GtkGetIntf( button );

    if( p_input_bank->pp_input[0] != NULL )
    {
        input_SetStatus( p_input_bank->pp_input[0], INPUT_STATUS_FASTER );
    }
}


void
on_toolbar_about_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GtkGetIntf( button );

    // Toggle notebook
    if (p_intf->p_sys->p_notebook)
    {
        gtk_widget_show( GTK_WIDGET(p_intf->p_sys->p_notebook) );
        gtk_notebook_set_page(p_intf->p_sys->p_notebook,2);
    }
    gdk_window_raise( p_intf->p_sys->p_window->window );
}


void
on_comboURL_entry_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
    intf_thread_t * p_intf = GtkGetIntf( editable );
    gchar *       psz_url;

    if (p_intf->p_sys->b_autoplayfile == 1)
    {
        psz_url = gtk_entry_get_text(GTK_ENTRY(editable));
        MediaURLOpenChanged( GTK_WIDGET(editable), psz_url );
    }
}

void
on_clistmedia_click_column             (GtkCList        *clist,
                                        gint             column,
                                        gpointer         user_data)
{
    static GtkSortType sort_type = GTK_SORT_ASCENDING;

    // Should sort on column
    switch(sort_type)
    {
        case GTK_SORT_ASCENDING:
            sort_type = GTK_SORT_DESCENDING;
            break;
        case GTK_SORT_DESCENDING:
            sort_type = GTK_SORT_ASCENDING;
            break;
    }
    gtk_clist_freeze( clist );
    gtk_clist_set_sort_type( clist, sort_type );
    gtk_clist_sort( clist );
    gtk_clist_thaw( clist );
}


void
on_clistmedia_select_row               (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GtkGetIntf( clist );
    gchar *text[2];
    gint ret;
    struct stat st;

    ret = gtk_clist_get_text (p_intf->p_sys->p_clist, row, 0, text);
    if (ret)
    {
        if (lstat((char*)text[0], &st)==0)
        {
            if (S_ISDIR(st.st_mode))
               ReadDirectory(p_intf->p_sys->p_clist, text[0]);
            else
               MediaURLOpenChanged(GTK_WIDGET(p_intf->p_sys->p_clist), text[0]);
       }
    }
}


void
on_cbautoplay_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    intf_thread_t * p_intf = GtkGetIntf( togglebutton );

    if (p_intf->p_sys->b_autoplayfile == 1)
    {
       p_intf->p_sys->b_autoplayfile = 0;
       intf_ErrMsg("autoplay not selected");
    }
    else
    {
       p_intf->p_sys->b_autoplayfile = 1;
       intf_ErrMsg("autoplay selected");
    }
}


gboolean
on_familiar_delete_event               (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    GtkExit( GTK_WIDGET( widget ), user_data );
    return TRUE;
}


void
on_clistmedia_start_selection          (GtkCList        *clist,
                                        gpointer         user_data)
{
    g_print( ">>>>start_selection\n" );
}


void
on_clistmedia_end_selection            (GtkCList        *clist,
                                        gpointer         user_data)
{
    g_print( ">>>>end_selection\n" );
}


gboolean
on_clistmedia_enter_notify_event       (GtkWidget       *widget,
                                        GdkEventCrossing *event,
                                        gpointer         user_data)
{
  g_print( ">>>>enter_notify_event\n" );
  return FALSE;
}


void
on_buttonSave_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
    on_buttonApply_clicked( button, user_data );
    config_SaveConfigFile( NULL );
}


void
on_buttonApply_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *apply_button;
//    GHashTable *hash_table;
//
//    hash_table = (GHashTable *)gtk_object_get_data( GTK_OBJECT(user_data),
//                                                    "config_hash_table" );
//    g_hash_table_foreach_remove( hash_table, GtkSaveHashValue, NULL );

    /* change the highlight status of the Apply button */
    apply_button = (GtkWidget *)gtk_object_get_data( GTK_OBJECT(user_data),
                                                    "buttonAapply" );
    gtk_widget_set_sensitive( apply_button, FALSE );
}


void
on_buttonCancel_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    //gtk_widget_destroy( gtk_widget_get_toplevel( GTK_WIDGET (button) ) );
}

