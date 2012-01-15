/**
 *******************************************************************************
 * @file BufferViewer.cpp
 * @brief Editeur De N'ours : main textViewer diplayer
 * @author Edouard DUPIN
 * @date 04/12/2010
 * @par Project
 * Edn
 *
 * @par Copyright
 * Copyright 2010 Edouard DUPIN, all right reserved
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY.
 *
 * Licence summary : 
 *    You can modify and redistribute the sources code and binaries.
 *    You can send me the bug-fix
 *    You can not earn money with this Software (if the source extract from Edn
 *        represent less than 50% of original Sources)
 * Term of the licence in in the file licence.txt.
 *
 *******************************************************************************
 */

#include <tools_debug.h>
#include <tools_globals.h>
#include <Display.h>
#include <BufferView.h>
#include <BufferManager.h>
#include <ColorizeManager.h>
#include <MainWindows.h>

#undef __class__
#define __class__	"BufferView"

BufferView::BufferView(void) : MsgBroadcast("Buffer View", EDN_CAT_GUI)
{
	m_shawableAreaX = 0;
	m_shawableAreaY = 0;
	// Init link with the buffer Manager
	//m_bufferManager = BufferManager::Get();
	//m_colorManager = ColorizeManager::Get();
#if 0
	m_widget = gtk_drawing_area_new();
	gtk_widget_set_size_request( m_widget, 250, 100);

	gtk_widget_add_events(	m_widget, 
								GDK_KEY_PRESS_MASK
							|	GDK_BUTTON_PRESS_MASK
							|	GDK_BUTTON_RELEASE_MASK
							|	GDK_POINTER_MOTION_MASK
							|	GDK_POINTER_MOTION_HINT_MASK);

#	ifdef USE_GTK_VERSION_3_0
	g_object_set(m_widget,"can-focus", TRUE, NULL);
#	elif defined( USE_GTK_VERSION_2_0 )
	GTK_WIDGET_SET_FLAGS(m_widget, GTK_CAN_FOCUS);
#	endif
	// Focus Event
	g_signal_connect(		G_OBJECT(m_widget), "focus_in_event",		G_CALLBACK(CB_focusGet),         this);
	g_signal_connect(		G_OBJECT(m_widget), "focus_out_event",		G_CALLBACK(CB_focusLost),        this);
	// Keyboard Event
	g_signal_connect_after(	G_OBJECT(m_widget), "key_press_event",		G_CALLBACK(CB_keyboardEvent),    this);
	g_signal_connect_after(	G_OBJECT(m_widget), "key_release_event",	G_CALLBACK(CB_keyboardEvent),    this);
	// Mouse Event
	g_signal_connect(		G_OBJECT(m_widget), "button_press_event",	G_CALLBACK(CB_mouseButtonEvent), this);
	g_signal_connect(		G_OBJECT(m_widget), "button_release_event",	G_CALLBACK(CB_mouseButtonEvent), this);
	g_signal_connect(		G_OBJECT(m_widget), "motion_notify_event",	G_CALLBACK(CB_mouseMotionEvent), this);
	// Display Event
	g_signal_connect(		G_OBJECT(m_widget), "realize",				G_CALLBACK(CB_displayInit),      this);
#	ifdef USE_GTK_VERSION_3_0
	g_signal_connect(		G_OBJECT(m_widget), "draw",					G_CALLBACK(CB_displayDraw),		this);
#	elif defined( USE_GTK_VERSION_2_0 )
	g_signal_connect(		G_OBJECT(m_widget), "expose_event",			G_CALLBACK(CB_displayDraw),		this);
#	endif
#endif
	m_selectedID = -1;

}

BufferView::~BufferView(void)
{

}
/*
GtkWidget * BufferView::GetMainWidget(void)
{
	return m_widget;
}
*/

void BufferView::OnMessage(int32_t id, int32_t dataID)
{
	switch (id)
	{
		case EDN_MSG__BUFFER_CHANGE_CURRENT:
			m_selectedID = dataID;
		case EDN_MSG__BUFFER_CHANGE_STATE:
		case EDN_MSG__BUFFER_CHANGE_NAME:
		case EDN_MSG__BUFFER_CHANGE_MODIFY:
			// change Title :
			//gtk_widget_queue_draw(m_widget);
			break;
		case EDN_MSG__USER_DISPLAY_CHANGE:
			// Redraw all the display ...
			//gtk_widget_queue_draw(m_widget);
			break;
	}
}
#if 0

gboolean BufferView::CB_displayDraw( GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	BufferView * self = reinterpret_cast<BufferView*>(data);

#	ifdef USE_GTK_VERSION_3_0
	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);
	bool needRedrawAll = false;
	if (self->m_shawableAreaX != allocation.width) {
		needRedrawAll = true;
		self->m_shawableAreaX = allocation.width;
	}
	if (self->m_shawableAreaY != allocation.height) {
		needRedrawAll = true;
		self->m_shawableAreaY = allocation.height;
	}
#	elif defined( USE_GTK_VERSION_2_0)
	bool needRedrawAll = false;
	if (self->m_shawableAreaX != widget->allocation.width) {
		needRedrawAll = true;
		self->m_shawableAreaX = widget->allocation.width;
	}
	if (self->m_shawableAreaY != widget->allocation.height) {
		needRedrawAll = true;
		self->m_shawableAreaY = widget->allocation.height;
	}
#endif
	if (true == needRedrawAll) {
		//myBuffer->ForceReDraw(true);
	}
	EDN_INFO("==========================================================================");
	EDN_INFO("Request a display of : " << self->m_shawableAreaX << "px * "<<  self->m_shawableAreaY<<"px");
	DrawerManager monDrawer(widget, self->m_shawableAreaX, self->m_shawableAreaY);
	
	// clean buffer
	monDrawer.Clean(self->m_colorManager->Get(COLOR_LIST_BG_1));
	
	// get the number of buffer open
	int32_t nbBufferOpen = self->m_bufferManager->Size();
	int32_t i;
	uint32_t lineID = 0;
	uint32_t fontHeight = Display::GetFontHeight();
	basicColor_te selectFG = COLOR_LIST_TEXT_NORMAL;
	basicColor_te selectBG = COLOR_LIST_BG_1;
	for (i=0; i < nbBufferOpen; i++) {
		Edn::File name;
		bool isModify;
		if (self->m_bufferManager->Exist(i)) {
			isModify = self->m_bufferManager->Get(i)->IsModify();
			name = self->m_bufferManager->Get(i)->GetFileName();
			char *tmpModify = (char*)" ";
			if (true == isModify) {
				tmpModify = (char*)"M";
			}
			char name2[1024] = "";
			sprintf(name2, "[%2d](%s) %s", i, tmpModify, name.GetShortFilename().c_str() );
			
			if (true == isModify) {
				selectFG = COLOR_LIST_TEXT_MODIFY;
			} else {
				selectFG = COLOR_LIST_TEXT_NORMAL;
			}
			if (lineID%2==0) {
				selectBG = COLOR_LIST_BG_1;
			} else {
				selectBG = COLOR_LIST_BG_2;
			}
			if (self->m_selectedID == i) {
				selectBG = COLOR_LIST_BG_SELECTED;
			}
			EDN_INFO("color fg=" << selectFG << " bg="<<  selectBG);
			monDrawer.Rectangle(self->m_colorManager->Get(selectBG), 0, lineID*fontHeight, self->m_shawableAreaX, Display::GetFontHeight());
			
			monDrawer.Text(self->m_colorManager->Get(selectFG), 2, lineID*fontHeight, name2);
			monDrawer.Flush();
			lineID ++;
		}
		
	}
	return TRUE;

}




// sur : �mis lors du premier affichage de la GtkDrawingArea
gboolean BufferView::CB_displayInit( GtkWidget *widget, gpointer data)
{
	BufferView * self = reinterpret_cast<BufferView*>(data);
	
#	ifdef USE_GTK_VERSION_3_0
	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);
	int32_t size_x = allocation.width;
	int32_t size_y = allocation.height;
	
	self->m_shawableAreaX = allocation.width;
	self->m_shawableAreaY = allocation.height;
#	elif defined( USE_GTK_VERSION_2_0)
	int32_t size_x = widget->allocation.width;
	int32_t size_y = widget->allocation.height;
	
	self->m_shawableAreaX = widget->allocation.width;
	self->m_shawableAreaY = widget->allocation.height;
#	endif
	EDN_INFO("Request a diplay of : " << size_x << "px * " << size_y << "px");

	//Display::InitDisplayParam(self->m_displayParameters, widget, 700, 1200);

	gtk_widget_queue_draw( widget );
	return TRUE;
}

gint BufferView::CB_focusGet(	GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	//BufferView * self = reinterpret_cast<BufferView*>(data);
	
#	ifdef USE_GTK_VERSION_2_0
	GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_FOCUS);
#	endif
	EDN_INFO("Focus - In");
	gtk_widget_queue_draw( widget );
	return FALSE;
}

gint BufferView::CB_focusLost(	GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	//BufferView * self = reinterpret_cast<BufferView*>(data);
	
#	ifdef USE_GTK_VERSION_2_0
	GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_FOCUS);
#	endif
	EDN_INFO("Focus - out");
	return FALSE;
}

gint BufferView::CB_keyboardEvent(	GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	//BufferView * self = reinterpret_cast<BufferView*>(data);
	
	if(event->type == GDK_KEY_PRESS) {
		gtk_widget_queue_draw( widget );
	}
	return true;
}


void BufferView::OnPopupEventShow(GtkWidget *menuitem, gpointer data)
{
	BufferView * self = reinterpret_cast<BufferView*>(data);
	self->SendMessage(EDN_MSG__CURRENT_CHANGE_BUFFER_ID, self->m_contectMenuSelectID);
	self->m_contectMenuSelectID = -1;
}

void BufferView::OnPopupEventClose(GtkWidget *menuitem, gpointer data)
{
	BufferView * self = reinterpret_cast<BufferView*>(data);
	self->SendMessage(EDN_MSG__BUFF_ID_CLOSE, self->m_contectMenuSelectID);
	self->m_contectMenuSelectID = -1;
}

void BufferView::OnPopupEventSave(GtkWidget *menuitem, gpointer data)
{
	BufferView * self = reinterpret_cast<BufferView*>(data);
	self->SendMessage(EDN_MSG__BUFF_ID_SAVE, self->m_contectMenuSelectID);
	self->m_contectMenuSelectID = -1;
}

void BufferView::OnPopupEventSaveAs(GtkWidget *menuitem, gpointer data)
{
	BufferView * self = reinterpret_cast<BufferView*>(data);
	self->SendMessage(EDN_MSG__GUI_SHOW_SAVE_AS, self->m_contectMenuSelectID);
	self->m_contectMenuSelectID = -1;
}


void BufferView::ViewPopupMenu(GtkWidget *parrent, GdkEventButton *event, int32_t BufferID)
{
	// Save the slected buffer
	m_contectMenuSelectID = BufferID;
	if (m_bufferManager->Exist(m_contectMenuSelectID)) {
		GtkWidget *menu, *menuitem;
		menu = gtk_menu_new();
		menuitem = gtk_menu_item_new_with_label("Show");
		g_signal_connect(    G_OBJECT(menuitem),     "activate",     G_CALLBACK(OnPopupEventShow),     this);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		if (true == m_bufferManager->Get(m_contectMenuSelectID)->HaveName()) {
			if (true == m_bufferManager->Get(m_contectMenuSelectID)->IsModify()) {
				menuitem = gtk_menu_item_new_with_label("Save");
			} else {
				menuitem = gtk_menu_item_new_with_label("Force Save");
			}
			g_signal_connect(G_OBJECT(menuitem),     "activate",     G_CALLBACK(OnPopupEventSave),     this);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		}
		menuitem = gtk_menu_item_new_with_label("Save As ...");
		g_signal_connect(    G_OBJECT(menuitem),     "activate",     G_CALLBACK(OnPopupEventSaveAs),   this);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		menuitem = gtk_menu_item_new_with_label("Close");
		g_signal_connect(    G_OBJECT(menuitem),     "activate",     G_CALLBACK(OnPopupEventClose),    this);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		gtk_widget_show_all(menu);
		// Note: event can be NULL here when called from view_onPopupMenu; 
		// gdk_event_get_time() accepts a NULL argument
		gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
		               (event != NULL) ? event->button : 0,
		               gdk_event_get_time((GdkEvent*)event));
	} else {
		EDN_ERROR("Buffer does not Exist !!! " << m_contectMenuSelectID);
	}
}




gint BufferView::CB_mouseButtonEvent(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	BufferView * self = reinterpret_cast<BufferView*>(data);
	// get focus on the widget
	gtk_widget_grab_focus(widget);
	if (event->button == 1) {
		if (event->type == GDK_BUTTON_PRESS) {
			EDN_INFO("mouse-event BT1  ==> One Clicked");
		}else if (event->type == GDK_2BUTTON_PRESS) {
			//EDN_INFO("mouse-event BT1  ==> Double Clicked %d, %d", (uint32_t)event->x, (uint32_t)event->y);
			uint32_t fontHeight = Display::GetFontHeight();
			int32_t selectBuf = self->m_bufferManager->WitchBuffer((event->y / fontHeight) + 1);
			//EDN_INFO(" plop %d / %d = %d ==> %d", (uint32_t)event->y, fontHeight, ((uint32_t)event->y / fontHeight), selectBuf);
			if ( 0 <= selectBuf) {
				self->SendMessage(EDN_MSG__CURRENT_CHANGE_BUFFER_ID, selectBuf);
			}
		}
	} else if (event->button == 3) {
		if (event->type == GDK_BUTTON_PRESS) {
			EDN_INFO("mouse-event BT3 PRESS");
			uint32_t fontHeight = Display::GetFontHeight();
			int32_t selectBuf = self->m_bufferManager->WitchBuffer((event->y / fontHeight) + 1);
			if ( 0 <= selectBuf) {
				self->ViewPopupMenu(widget, event, selectBuf);
			}
		}
	} else {
		// not usefull to redraw
		return true;
	}
	gtk_widget_queue_draw( widget );
	return true;
}



gint BufferView::CB_mouseMotionEvent( GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	//BufferView * self = reinterpret_cast<BufferView*>(data);
	
/*
	if (true == ButtunOneSelected) {
		int x, y;
		GdkModifierType state;

		if (event->is_hint) {
			gdk_window_get_pointer(event->window, &x, &y, &state);
		} else {
			x = event->x;
			y = event->y;
			state = (GdkModifierType)event->state;
		}
		EDN_INFO("mouse-motion BT1 %d, %d", x, y);
	}
*/
	return true;
}
#endif
