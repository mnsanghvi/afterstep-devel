/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#define LOCAL_DEBUG

#include "../../../../configure.h"
#include "../../../../libAfterStep/asapp.h"
#include "../../../../libAfterImage/afterimage.h"
#include "../../../../libAfterStep/screen.h"
#include "../../../../libAfterStep/colorscheme.h"
#include "../../../../libAfterStep/module.h"
#include "../../../../libASGTK/asgtk.h"

#include <unistd.h>		   

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)


void
gtk_image_browser_destroy(GtkWidget *widget, gpointer user_data)
{
	if(WallpaperState.filechooser != NULL)
		WallpaperState.filechooser = NULL ; 
	gtk_widget_set_sensitive( GTK_WIDGET(WallpaperState.list_browse_button), TRUE );
}

void
on_list_del_clicked(GtkButton *button, gpointer user_data)
{
	ASGtkImageDir *id = ASGTK_IMAGE_DIR(user_data);
	ASImageListEntry *entry = asgtk_image_dir_get_selection( id );
	if( entry ) 
	{	
		if( asgtk_yes_no_question1( WallpaperState.main_window, "Do you really want to delete private background file \"%s\" ???", entry->name ) )
		{
			if( id->mini_extension ) 
			{	
				char *mini_filename, *mini_fullfilename ;
				asgtk_image_dir_make_mini_names( id, entry, &mini_filename, &mini_fullfilename ); 
				
				if( CheckFile( mini_fullfilename ) == 0 ) 
				{
					if( asgtk_yes_no_question1( WallpaperState.main_window, "It appears that there is a minipixmap for deleted background with the name \"%s\". Would you like to delete it as well ?", mini_filename ) )
					{
						unlink( mini_fullfilename );
					}	 				   
				}	
				free( mini_fullfilename );
				free( mini_filename );
			}			
			unlink( entry->fullfilename );
			asgtk_image_dir_refresh( id );	 
		}	 
		unref_asimage_list_entry( entry );
	}
}

void
on_list_apply_clicked(GtkButton *button, gpointer user_data)
{
	ASGtkImageDir *id = ASGTK_IMAGE_DIR(user_data);
	ASImageListEntry *entry = asgtk_image_dir_get_selection( id );
	if( entry ) 
	{	
		SendTextCommand ( F_CHANGE_BACKGROUND, NULL, entry->fullfilename, 0);
		unref_asimage_list_entry( entry );
	}
}

void
on_make_xml_clicked(GtkButton *button, gpointer user_data)
{

}

void gtk_xml_editor_destroy( GtkWidget *widget, gpointer user_data ) 
{
	WallpaperState.xml_editor = NULL ;
	gtk_widget_set_sensitive( GTK_WIDGET(WallpaperState.edit_xml_button), TRUE );
}	 

void
on_edit_xml_clicked(GtkButton *button, gpointer user_data)
{
	ASGtkImageDir *id = ASGTK_IMAGE_DIR(user_data);
	ASImageListEntry *entry = asgtk_image_dir_get_selection( id );
	if( WallpaperState.xml_editor == NULL ) 
	{	
		WallpaperState.xml_editor = asgtk_xml_editor_new();
		g_signal_connect (G_OBJECT (WallpaperState.xml_editor), "destroy", G_CALLBACK (gtk_xml_editor_destroy), NULL);
	}

	gtk_widget_show( WallpaperState.xml_editor );
	asgtk_xml_editor_set_entry( ASGTK_XML_EDITOR(WallpaperState.xml_editor), entry );
	unref_asimage_list_entry( entry );

	gtk_widget_set_sensitive( GTK_WIDGET(WallpaperState.edit_xml_button), FALSE );
}

void
on_list_add_clicked(GtkButton *button, gpointer user_data)
{
	ASGtkImageDir *id = ASGTK_IMAGE_DIR(user_data);
	ASImageListEntry *entry = asgtk_image_dir_get_selection( id );
	if( entry ) 
	{	
		ASGtkImageDir *backs_list = ASGTK_IMAGE_DIR(WallpaperState.backs_list);
		char *new_filename = make_file_name( backs_list->fulldirname, entry->name );
		if( CheckFile( new_filename ) == 0 ) 
		{
			if( !asgtk_yes_no_question1( WallpaperState.main_window, "Private background with the name \"%s\" already exists. Would you like to replace it ???", entry->name ) )
			{
				free( new_filename );
				return;
			}	 				   
		}	
		copy_file (entry->fullfilename, new_filename);
		free( new_filename );
		if( backs_list->mini_extension != NULL  && entry->preview != NULL ) 
		{
			char *mini_filename, *mini_fullfilename ;
			Bool do_mini = True ;
			asgtk_image_dir_make_mini_names( backs_list, entry, &mini_filename, &mini_fullfilename );
			if( CheckFile( mini_fullfilename ) == 0 ) 
			{
				if( !asgtk_yes_no_question1( WallpaperState.main_window, "Overwrite minipixmap \"%s\" with the new one ?", mini_filename ) )
					do_mini = False ;
			}	

			if( do_mini ) 
			{
				ASImage *thumbnail = scale_asimage( get_screen_visual(NULL), entry->preview, 24, 24, ASA_ASImage, 0, ASIMAGE_QUALITY_DEFAULT );
				if( thumbnail ) 
				{	
					save_asimage_to_file(mini_fullfilename, thumbnail, "png", "9", NULL, 0, True);
					destroy_asimage( &thumbnail );					
				}
			}	 

			free( mini_fullfilename );
			free( mini_filename );
		}	 
		unref_asimage_list_entry( entry );
		asgtk_image_dir_refresh( backs_list );	 
	}
}

void
on_update_as_menu_clicked(GtkButton *button, gpointer user_data)
{
	SendTextCommand ( F_QUICKRESTART, NULL,  "startmenu", 0);
}

void
on_browse_clicked(GtkButton *button, gpointer user_data)
{
	if( WallpaperState.filechooser == NULL ) 
	{	
		GtkWidget *add_button, *apply_button ; 
		WallpaperState.filechooser = asgtk_image_browser_new();
	
		/* close_button = asgtk_image_browser_add_main_button (ASGTK_IMAGE_BROWSER(WallpaperState.filechooser), "gtk-close", G_CALLBACK(on_filechooser_close_clicked), NULL); */
		add_button = asgtk_image_browser_add_selection_button (ASGTK_IMAGE_BROWSER(WallpaperState.filechooser), GTK_STOCK_ADD, G_CALLBACK(on_list_add_clicked)); 
		apply_button = asgtk_image_browser_add_selection_button (ASGTK_IMAGE_BROWSER(WallpaperState.filechooser), GTK_STOCK_APPLY, G_CALLBACK(on_list_apply_clicked));

		g_signal_connect (G_OBJECT (WallpaperState.filechooser), "destroy", G_CALLBACK (gtk_image_browser_destroy), NULL);  		   
		/*gtk_widget_set_size_request (GTK_WIDGET(close_button), 150, -1);*/
	}
	
	gtk_widget_show (GTK_WIDGET(WallpaperState.filechooser));
	gtk_widget_set_sensitive( GTK_WIDGET(WallpaperState.list_browse_button), FALSE );
}

static void
backs_list_sel_handler(ASGtkImageDir *id, gpointer user_data)
{
	ASGtkImageView *iv = ASGTK_IMAGE_VIEW(user_data);
	ASImageListEntry *le;
	g_return_if_fail (ASGTK_IS_IMAGE_DIR (id));
	
	le = asgtk_image_dir_get_selection( id ); 
	if( WallpaperState.xml_editor && le->type == ASIT_XMLScript )
		asgtk_xml_editor_set_entry( ASGTK_XML_EDITOR( WallpaperState.xml_editor), le );
	
	if( iv ) 
	{	
		asgtk_image_view_set_entry ( iv, le);
		
		if( le->type == ASIT_XMLScript ) 
		{	
			gtk_widget_show(WallpaperState.edit_xml_button);
			gtk_widget_hide(WallpaperState.make_xml_button);
		}else
		{
			gtk_widget_hide(WallpaperState.edit_xml_button);
			gtk_widget_show(WallpaperState.make_xml_button);
		}		   
	}
	if( le )
		unref_asimage_list_entry( le );
}


void
create_main_window (void)
{
    GtkWidget *main_vbox;

	GtkWidget *buttons_hbox;

  	WallpaperState.main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  	gtk_window_set_title (GTK_WINDOW (WallpaperState.main_window), _("AfterStep Wallpaper Manager"));

 	colorize_gtk_window( WallpaperState.main_window ); 	
		
	main_vbox = gtk_vbox_new (FALSE, 0);
  	gtk_widget_show (main_vbox);
	gtk_container_add (GTK_CONTAINER (WallpaperState.main_window), main_vbox);

  	WallpaperState.list_hbox = gtk_hbox_new (FALSE, 0);
  	gtk_widget_show (WallpaperState.list_hbox);
  	gtk_box_pack_start (GTK_BOX (main_vbox), WallpaperState.list_hbox, TRUE, TRUE, 5);

  	buttons_hbox = gtk_hbutton_box_new ();
  	gtk_hbutton_box_set_layout_default(GTK_BUTTONBOX_SPREAD);
  	gtk_widget_show (buttons_hbox);
  	gtk_box_pack_end (GTK_BOX (main_vbox), buttons_hbox, FALSE, FALSE, 5);
    /* separator really goes above the buttons box, so it is added second from the end ! */

  	/* Store pointers to all widgets, for use by lookup_widget(). */
  	GLADE_HOOKUP_OBJECT_NO_REF (WallpaperState.main_window, WallpaperState.main_window, "main_window");
}

GtkWidget*
create_list_button( GtkWidget *buttons_hbox, const char *stock, GCallback func )
{
	GtkWidget *btn = gtk_button_new_from_stock (stock);
  	gtk_widget_show (btn);
  	gtk_box_pack_start (GTK_BOX (buttons_hbox), btn, FALSE, FALSE, 0);
  	g_signal_connect ((gpointer) btn, "clicked", G_CALLBACK (func), NULL);
	colorize_gtk_widget( GTK_WIDGET(btn), get_colorschemed_style_button());
	return btn;	
}	   

void
create_backs_list()
{
	GtkWidget *vbox ;
  	
	vbox = gtk_vbox_new (FALSE, 0);
  	gtk_widget_show (vbox);
  	gtk_box_pack_start (GTK_BOX (WallpaperState.list_hbox), vbox, FALSE, FALSE, 5);

  	WallpaperState.backs_list = asgtk_image_dir_new();
	gtk_box_pack_start (GTK_BOX (vbox), WallpaperState.backs_list, TRUE, TRUE, 0);
	gtk_widget_set_size_request (WallpaperState.backs_list, 200, INITIAL_PREVIEW_HEIGHT);
	gtk_widget_show (WallpaperState.backs_list);

	/* creating the list widget itself */
	asgtk_image_dir_set_title(ASGTK_IMAGE_DIR(WallpaperState.backs_list),"Images in your private backgrounds folder:");
	asgtk_image_dir_set_mini (ASGTK_IMAGE_DIR(WallpaperState.backs_list), ".mini" );
		
	colorize_gtk_widget( WallpaperState.backs_list, get_colorschemed_style_button());
	gtk_widget_set_style( WallpaperState.backs_list, get_colorschemed_style_normal());
	
	/* adding list manipulation buttons : */

	WallpaperState.list_update_as_button = asgtk_add_button_to_box( NULL, GTK_STOCK_REFRESH, "Update AfterStep Menu", G_CALLBACK(on_update_as_menu_clicked), NULL );
  	gtk_box_pack_end (GTK_BOX (vbox), WallpaperState.list_update_as_button, FALSE, FALSE, 0);
	WallpaperState.list_browse_button = asgtk_add_button_to_box( NULL, GTK_STOCK_ADD, "Browse for more", G_CALLBACK(on_browse_clicked), NULL );
  	gtk_box_pack_end (GTK_BOX (vbox), WallpaperState.list_browse_button, FALSE, FALSE, 5);

}

void 
create_list_preview()
{
	int preview_width ; 

	WallpaperState.list_preview = asgtk_image_view_new();
	preview_width = (INITIAL_PREVIEW_HEIGHT *Scr.MyDisplayWidth)/Scr.MyDisplayHeight ;
	gtk_widget_set_size_request (WallpaperState.list_preview, preview_width, INITIAL_PREVIEW_HEIGHT);
	asgtk_image_view_set_aspect (ASGTK_IMAGE_VIEW(WallpaperState.list_preview), Scr.MyDisplayWidth, Scr.MyDisplayHeight );
  	gtk_box_pack_end (GTK_BOX (WallpaperState.list_hbox), WallpaperState.list_preview, TRUE, TRUE, 0);
	gtk_widget_show (WallpaperState.list_preview);

}

void
reload_private_backs_list()
{	
	char *private_back_dir = PutHome("~/.afterstep/backgrounds");
	asgtk_image_dir_set_path(ASGTK_IMAGE_DIR(WallpaperState.backs_list), private_back_dir);
	free( private_back_dir );
}

void 
init_ASWallpaper()
{
	memset( &WallpaperState, 0x00, sizeof(ASWallpaperState));
	
	create_main_window(); 
	create_backs_list();
	create_list_preview();
	
	WallpaperState.sel_apply_button = asgtk_add_button_to_box( NULL, GTK_STOCK_APPLY, NULL, G_CALLBACK(on_list_apply_clicked), WallpaperState.backs_list );
	WallpaperState.make_xml_button = asgtk_add_button_to_box( NULL, GTK_STOCK_PROPERTIES, "Make XML", G_CALLBACK(on_make_xml_clicked), WallpaperState.backs_list );
	WallpaperState.edit_xml_button = asgtk_add_button_to_box( NULL, GTK_STOCK_PROPERTIES, "Edit XML", G_CALLBACK(on_edit_xml_clicked), WallpaperState.backs_list );
	WallpaperState.sel_del_button = asgtk_add_button_to_box( NULL, GTK_STOCK_DELETE, NULL, G_CALLBACK(on_list_del_clicked), WallpaperState.backs_list );

	gtk_widget_hide(WallpaperState.edit_xml_button);

	asgtk_image_view_add_tool( ASGTK_IMAGE_VIEW(WallpaperState.list_preview), WallpaperState.sel_apply_button, 0 );
	asgtk_image_view_add_tool( ASGTK_IMAGE_VIEW(WallpaperState.list_preview), WallpaperState.make_xml_button, 5 );
	asgtk_image_view_add_tool( ASGTK_IMAGE_VIEW(WallpaperState.list_preview), WallpaperState.edit_xml_button, 5 );
	asgtk_image_view_add_tool( ASGTK_IMAGE_VIEW(WallpaperState.list_preview), WallpaperState.sel_del_button, 5 );
	
	asgtk_image_dir_set_sel_handler( ASGTK_IMAGE_DIR(WallpaperState.backs_list), backs_list_sel_handler, WallpaperState.list_preview);

	reload_private_backs_list();

	g_signal_connect (G_OBJECT (WallpaperState.main_window), "destroy", G_CALLBACK (gtk_main_quit), NULL);
  	gtk_widget_show (WallpaperState.main_window);
}	 

