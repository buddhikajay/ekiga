
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         addressbook_window.cpp  -  description
 *                         ---------------------------------------
 *   begin                : Wed Feb 28 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : This file contains functions to build the 
 *                          addressbook window.
 *
 */

#include <gdk/gdkkeysyms.h>

#include "../../config.h"

#include "addressbook.h"
#include "main.h"
#include "chat.h"
#include "callbacks.h"
#include "ekiga.h"
#include "urlhandler.h"
#include "misc.h"
#include "statusicon.h"

#include "gmstockicons.h"
#include "gmcontacts.h"
#include "gmdialog.h"
#include "gmconf.h"
#include "gmmenuaddon.h"

#include "toolbox/toolbox.h"



struct GmAddressbookWindow_ {

  GtkWidget *aw_menu;		/* The main menu of the window */
  GtkWidget *aw_tree_view;      /* The GtkTreeView that contains the address 
				   books list */
  GtkWidget *aw_notebook;       /* The GtkNotebook that contains the different
				   listings for each of the address books */
};


struct GmAddressbookWindowPage_ {

  GtkWidget *awp_tree_view; 	/* The GtkTreeView that contains 
				   the users list */
  GtkWidget *awp_option_menu;    /* The option menu for the search */
  GtkWidget *awp_search_entry;   /* The search entry */ 
  GtkWidget *awp_statusbar;	/* The status bar */
};


typedef struct GmAddressbookWindow_ GmAddressbookWindow;
typedef struct GmAddressbookWindowPage_ GmAddressbookWindowPage;


#define GM_ADDRESSBOOK_WINDOW(x) (GmAddressbookWindow *) (x)
#define GM_ADDRESSBOOK_WINDOW_PAGE(x) (GmAddressbookWindowPage *) (x)


/* The different cell renderers for the different contacts sections (servers
   or groups */
enum {

  COLUMN_PIXBUF,
  COLUMN_AID,
  COLUMN_NAME,
  COLUMN_NOTEBOOK_PAGE,
  COLUMN_PIXBUF_VISIBLE,
  COLUMN_WEIGHT,
  COLUMN_URL,
  COLUMN_CALL_ATTRIBUTE,
  NUM_COLUMNS_CONTACTS
};

enum {

  COLUMN_STATUS,
  COLUMN_FULLNAME,
  COLUMN_UURL,
  COLUMN_CATEGORIES,
  COLUMN_SPEED_DIAL,
  COLUMN_LOCATION,
  COLUMN_COMMENT,
  COLUMN_SOFTWARE,
  COLUMN_EMAIL,
  COLUMN_UUID,
  COLUMN_USER_WEIGHT,
  NUM_COLUMNS_GROUPS
};


/* Declarations */


/* GUI functions */

/* DESCRIPTION  : / 
 * BEHAVIOR     : Frees a GmAddressbookWindowPage and its content.
 * PRE          : A non-NULL pointer to a GmAddressbookWindowPage.
 */
static void gm_awp_destroy (gpointer awp);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Frees a GmAddressbookWindow and its content.
 * PRE          : A non-NULL pointer to a GmAddressbookWindow.
 */
static void gm_aw_destroy (gpointer aw);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to the private GmAddressbookWindow
 * 		  used by the address book GMObject.
 * PRE          : The given GtkWidget pointer must be an address book GMObject.
 */
static GmAddressbookWindow *gm_aw_get_aw (GtkWidget *addressbook_window);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to the private GmAddressbookWindowPage
 * 		  used by any page of the internal GtkNotebook of the 
 * 		  address book GMObject.
 * PRE          : The given GtkWidget pointer must point to a page
 * 		  of the internal GtkNotebook of the address book GMObject.
 */
static GmAddressbookWindowPage *gm_aw_get_awp (GtkWidget *page);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to the private GmAddressbookWindowPage
 * 		  used by the current page of the internal GtkNotebook of the 
 * 		  address book GMObject.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject.
 */
static GmAddressbookWindowPage *gm_awp_get_current_awp (GtkWidget *adressbook_window);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to a newly allocated GmContact with
 * 		  all the info for the contact currently being selected
 * 		  in the address book window given as argument. NULL if none
 * 		  is selected.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject.
 */
static GmContact *gm_aw_get_selected_contact (GtkWidget *addressbook);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a pointer to a newly allocated GmAddressbook with
 * 		  all the info for the address book currently being selected
 * 		  in the address book window given as argument. NULL if none
 * 		  is selected.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject.
 */
static GmAddressbook *gm_aw_get_selected_addressbook (GtkWidget *addressbook);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Fills in the parameters with the current search filter type
 * 		  (search on all contacts, fullname, url, category) and
 * 		  the field to search for in the currently selected page.
 * 		  If the filter is empty, then NULL is returned.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject.
 */
static void gm_aw_get_search_filter (GtkWidget *addressbook_window,
				     int &type,
				     char * &filter);

/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns TRUE if there is a collision when adding a new
 * 		  contact or editing an old one. Returns FALSE if no collision
 * 		  is detected.
 * 		  Notice that when a collision occurs, the user is presented
 * 		  with a dialog allowing him to force adding the user if the
 * 		  collision didn't occur on a speed dial. If the user decides
 * 		  to force adding, then FALSE is returned as if there was no
 * 		  collision. The last argument is the parent window.
 * PRE          : The contact to add or modify, and its old version, if any.
 */
static gboolean gm_aw_check_contact_collision (GtkWidget *adressbook_window,
					       GmContact *new_contact,
					       GmContact *old_contact,
					       GtkWidget *parent_window);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Adds the given GmAddressbook to the address book window
 * 		  GMObject and updates it if it is a local address book.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. Non-NULL pointer to a GmAddressbook.
 */
static void gm_aw_add_addressbook (GtkWidget *addressbook_window,
				   GmAddressbook *addressbook);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Deletes the given GmAddressbook to the address book window
 * 		  GMObject.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. Non-NULL pointer to a GmAddressbook.
 */
static void gm_aw_delete_addressbook (GtkWidget *addressbook_window,
				      GmAddressbook *addressbook);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Modifies the given GmAddressbook to the address book window
 * 		  GMObject.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. Non-NULL pointer to a GmAddressbook.
 */

static void gm_aw_modify_addressbook (GtkWidget *addressbook_window,
				      GmAddressbook *addressbook);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Updates the content of the given GmAddressbook in the 
 * 		  address book window GMObject with the given GSList of 
 * 		  contacts. We only support 3 basic states for now (Available,
 * 		  Do not disturb/in a call, offline)
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. Non-NULL pointer to a GmAddressbook. Non-NULL 
 * 		  pointer to a GSList of GmContacts. Possibly NULL pointer
 * 		  to a message to display in that address book status bar.
 */
static void gm_aw_update_addressbook (GtkWidget *addressbook_window,
				      GmAddressbook *addressbook,
				      GSList *contacts,
				      gchar *msg);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Updates the content of the given GmAddressbook in the 
 * 		  address book window GMObject.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. 
 * 		  The first boolean indicates if the address book props
 * 		  are editable.
 * 		  The second boolean must indicate if a local 
 * 		  addressbook is selected, the third one if a remote 
 * 		  addressbook is selected, both may not be true at the same 
 * 		  time. All other situations are possible.
 */
static void gm_aw_update_menu_sensitivity (GtkWidget *addressbook_window,
					   gboolean is_editable,
					   gboolean is_local_selected,
					   gboolean is_remote_selected);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns the GtkNotebook page containing the content of the
 * 		  given GmAddressbook or -1 if no such page.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. The second argument must point to a valid 
 * 		  GmAddressbook. Both should be non-NULL.
 */
static gint gm_aw_get_notebook_page (GtkWidget *addressbook_window,
				     GmAddressbook *addressbook);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a popup menu to be displayed when the contacts pane 
 * 		  is clicked. The menu content depends on what is selected.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. The second argument must point to a valid 
 * 		  selected GmAddressbook (if any). The last argument points
 * 		  to the selected contact (if any). The 1st should be non-NULL.
 */
static GtkWidget *gm_aw_contact_menu_new (GtkWidget *addressbook_window,
					  GmAddressbook *addressbook,
					  GmContact *contact);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns a popup menu to be displayed when an addressbook is 
 * 		  clicked.
 * PRE          : The given GtkWidget pointer must point to the address book
 * 		  GMObject. 
 */
static GtkWidget *gm_aw_addressbook_menu_new (GtkWidget *addressbook_window);


/* Callbacks */

/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when something is selected in the 
 * 		  aw_tree_view of the GmAddressbookWindow. It returns TRUE or
 * 		  FALSE following the selected item is an address book or a
 * 		  category of address book.
 * PRE          : /
 */
static gboolean aw_tree_selection_function_cb (GtkTreeSelection *selection,
					       GtkTreeModel *model,
					       GtkTreePath *path,
					       gboolean path_currently_selected,
					       gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when a contact is double-clicked
 * 		  in the address book GMObject. He is called.
 * PRE          : The data must point to the address book window GmOject.  
 */
static void call_contact1_cb (GtkWidget *widget,
			      gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when a contact is clicked
 * 		  in the address book GMObject to send an message.
 * PRE          : The data must point to the chat window GmOject.  
 */
static void show_chat_window_cb (GtkWidget *widget,
				 gpointer data);

/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when a contact is double-clicked
 * 		  in the address book GMObject. He is called using the above
 * 		  callback.
 * PRE          : The data must point to the address book window GmOject.  
 */
static void call_contact2_cb (GtkTreeView *tree_view,
			      GtkTreePath *arg1,
			      GtkTreeViewColumn *arg2,
			      gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to add
 * 		  a contact. The address book dialog permitting to add or
 * 		  edit a contact is presented with empty fields.
 * PRE          : The gpointer must point to the address book window. 
 */
static void new_contact_cb (GtkWidget *unused,
			    gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to add
 * 		  an addressbook. The address book edition dialog is presented
 * 		  to the user.
 * PRE          : The gpointer must point to the address book window. 
 */
static void new_addressbook_cb (GtkWidget *unused,
				gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to delete
 * 		  something. If a contact is selected, presents the dialog
 * 		  to delete a contact and delete it if required. If no contact
 * 		  is selected but an address book, present the dialog to delete
 * 		  an addressbook, and delete it if required.
 * PRE          : The gpointer must point to the address book window. 
 */
static void delete_cb (GtkWidget *unused,
		       gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to edit the
 * 		  properties of something. If a contact is selected, 
 * 		  presents the dialog to edit a contact. If no contact
 * 		  is selected but an address book, present the dialog to edit 
 * 		  an addressbook, and delete it if required.
 * PRE          : The gpointer must point to the address book window. 
 */
static void properties_cb (GtkWidget *unused,
			   gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user chooses to search
 * 		  the content of an address book. It launches the search
 * 		  for the selected fields and updates the content of the GUI
 * 		  once the search is over.
 * PRE          : The gpointer must point to the address book window. 
 */
static void search_addressbook1_cb (GtkWidget *unused,
				    gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user double-clicks on 
 * 		  an addressbook.
 * 		  It launches the search for the selected fields and 
 * 		  updates the content of the GUI once the search is over using
 * 		  a separate thread launched by the above callback.
 * PRE          : The gpointer must point to the address book window. 
 */
static void search_addressbook2_cb (GtkTreeView *tree_view,
				    GtkTreePath *arg1,
				    GtkTreeViewColumn *arg2,
				    gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user clicks on a contact.
 * 		  Displays a popup menu.
 * PRE          : /
 */
static gint contact_clicked_cb (GtkWidget *unused,
				GdkEventButton *event,
				gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user clicks on an
 * 		  address book.
 * 		  Displays a popup menu.
 * PRE          : /
 */
static gint addressbook_clicked_cb (GtkWidget *unused,
				    GdkEventButton *event,
				    gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user clicks on a contact.
 * 		  It updates the menu sensitivity of the GUI following what
 * 		  is selected.
 * PRE          : /
 */
static void contact_selected_cb (GtkTreeSelection *selection,
				 gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user clicks on an address
 * 		  book. It unselects all contacts and updates the menu 
 * 		  sensitivity of the GUI accordingly.
 * PRE          : /
 */
static void addressbook_selected_cb (GtkTreeSelection *selection,
				     gpointer data);


/* DESCRIPTION  : / 
 * BEHAVIOR     : This callback is called when the user changes the type of
 * 		  the address book in the edit address book window.
 * 		  It will show/hide required options.
 * PRE          : The container to show/hide.
 */
static void edit_addressbook_type_menu_changed_cb (GtkComboBox *menu,
						   gpointer data);


/* DESCRIPTION  :  This callback is called when the user chooses to copy
 *                 a contact URL to the clipboard.
 * BEHAVIOR     :  Copy the URL for the selected contact into the clipboard.
 * PRE          :  The Address book window GmObject.
 */
static void copy_url_to_clipboard_cb (GtkWidget *unused,
				      gpointer data);


/* DESCRIPTION  :  This callback is called when the user chooses to write
 *                 to a contacts eMail address.
 * BEHAVIOR     :  Call the gnomemeeting URI handler gm_open_uri() with the mail address preceeded by "mailto:".
 * PRE          :  The Address book window GmObject.
 */
static void write_email_with_uricall_cb (GtkWidget *unused,
					 gpointer data);



/* DESCRIPTION  : This function is called when a user drags a contact above the
 *                window, to know if dropping is allowed.
 * BEHAVIOR     : TRUE if allowed, else FALSE.
 * PRE          : Assumes the widget is the server list pane.
 */
static gboolean dnd_allow_drop_cb (GtkWidget *widget,
				   gint x,
				   gint y,
				   gpointer unused);


/* DESCRIPTION  : This function is called when a drop occurs in the server
 *                list.
 * BEHAVIOR     : Adds the dropped GmContact to the addressbook.
 * PRE          : Assumes the gpointer is an addressbook window (widget),
 *                and the widget the server list pane.
 */
static void dnd_add_contact_server_cb (GtkWidget *widget,
				       GmContact *contact,
				       gint x,
				       gint y,
				       gpointer data);


/* DESCRIPTION  : This function is called when a drop occurs in the contact
 *                list of a server.
 * BEHAVIOR     : Adds the dropped GmContact to the addressbook.
 * PRE          : Assumes the gpointer is an addressbook window (widget).
 */
static void dnd_add_contact_contactlist_cb (GtkWidget *widget,
					    GmContact *contact,
					    gint x,
					    gint y,
					    gpointer data);


/* DESCRIPTION  : This function is called when a contact dragged from the
 *                window occurs.
 * BEHAVIOR     : Returns the dragged contact.
 * PRE          : /
 */
static GmContact *dnd_get_contact_cb (GtkWidget *widget,
				      gpointer unused);


/* Implementation */
static void
gm_awp_destroy (gpointer awp)
{
  g_return_if_fail (awp != NULL);

  delete ((GmAddressbookWindowPage *) awp);
}


static void
gm_aw_destroy (gpointer aw)
{
  g_return_if_fail (aw != NULL);

  delete ((GmAddressbookWindow *) aw);
}


static GmAddressbookWindow *
gm_aw_get_aw (GtkWidget *addressbook_window)
{
  g_return_val_if_fail (addressbook_window != NULL, NULL);

  return GM_ADDRESSBOOK_WINDOW (g_object_get_data (G_OBJECT (addressbook_window), "GMObject"));
}


static GmAddressbookWindowPage *
gm_aw_get_awp (GtkWidget *page)
{
  g_return_val_if_fail (page != NULL, NULL);

  return GM_ADDRESSBOOK_WINDOW_PAGE (g_object_get_data (G_OBJECT (page),
							"GMObject"));
}


static GmAddressbookWindowPage *
gm_awp_get_current_awp (GtkWidget *addressbook_window)
{
  GmAddressbookWindow *aw = NULL;
  
  int page_num = 0;
  GtkWidget *page = NULL;
  
  g_return_val_if_fail (addressbook_window != NULL, NULL);

  /* Get the required data from the GtkNotebook page */
  aw = gm_aw_get_aw (addressbook_window);

  g_return_val_if_fail (aw != NULL, NULL);

  g_return_val_if_fail (aw->aw_notebook != NULL, NULL);

  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (aw->aw_notebook));
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (aw->aw_notebook), page_num);

  /* in startup phase there is no page yet */
  if (!page) return NULL;

  return GM_ADDRESSBOOK_WINDOW_PAGE (g_object_get_data (G_OBJECT (page), 
							"GMObject"));
}


static GmContact *
gm_aw_get_selected_contact (GtkWidget *addressbook)
{
  GmAddressbookWindow *aw = NULL;
  GmAddressbookWindowPage *awp = NULL;

  GmContact *contact = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  g_return_val_if_fail (addressbook != NULL, NULL);

  /* Get the required data from the GtkNotebook page */
  aw = gm_aw_get_aw (addressbook);

  g_return_val_if_fail (aw != NULL, NULL);

  awp = gm_awp_get_current_awp (addressbook);
  if (!awp)
    return NULL;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (awp->awp_tree_view));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (awp->awp_tree_view));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    contact = gmcontact_new ();

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			COLUMN_FULLNAME, &contact->fullname, 
			COLUMN_SPEED_DIAL, &contact->speeddial,
			COLUMN_EMAIL, &contact->email,
			COLUMN_CATEGORIES, &contact->categories,
			COLUMN_UURL, &contact->url,
			COLUMN_UUID, &contact->uid,
			-1);
  }


  return contact;
}


static GmAddressbook *
gm_aw_get_selected_addressbook (GtkWidget *addressbook)
{
  GmAddressbookWindow *aw = NULL;
  GmAddressbook *abook = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  
  g_return_val_if_fail (addressbook != NULL, NULL);
  
  /* Get the required data from the GtkNotebook page */
  aw = gm_aw_get_aw (addressbook);

  g_return_val_if_fail (aw != NULL, NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (aw->aw_tree_view));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->aw_tree_view));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    abook = gm_addressbook_new ();
    if (abook->aid)
      g_free (abook->aid);
    if (abook->url)
      g_free (abook->url);

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			COLUMN_NAME, &abook->name, 
			COLUMN_URL, &abook->url,
			COLUMN_AID, &abook->aid,
			COLUMN_CALL_ATTRIBUTE, &abook->call_attribute,
			-1); 
  }

  return abook;
}


static void
gm_aw_get_search_filter (GtkWidget *addressbook_window,
			 int & type,
			 char * & filter)
{
  GmAddressbookWindowPage *awp = NULL;

  const char *entry_text = NULL;
  
  g_return_if_fail (addressbook_window);
  
  awp = gm_awp_get_current_awp (addressbook_window);
  
  filter = NULL;
  
  type = gtk_combo_box_get_active (GTK_COMBO_BOX (awp->awp_option_menu));
  entry_text = gtk_entry_get_text (GTK_ENTRY (awp->awp_search_entry));

  if (strcmp (entry_text, ""))
    filter = g_strdup (entry_text);
}


static gboolean
gm_aw_check_contact_collision (GtkWidget *addressbook_window,
			       GmContact *new_contact, 
			       GmContact *old_contact,
			       GtkWidget *parent_window) 
{
  GSList *contacts = NULL;
  
  GmContact *ctct = NULL;

  GtkWidget *dialog = NULL;
  
  gchar *dialog_text = NULL;
  gchar *primary_text = NULL;
  gchar *secondary_text = NULL;
  
  int cpt = 0;
  int nbr = 0;

  gboolean to_return = FALSE;
  gboolean check_fullname = FALSE;
  gboolean check_url = FALSE;
  gboolean check_speeddial = FALSE;
  
  g_return_val_if_fail (new_contact != NULL, TRUE);


  /* Check the full name if we are adding a contact or if we are editing
   * a contact and added a full name, or changed the full name
   */
  if (new_contact->fullname && strcmp (new_contact->fullname, ""))
    check_fullname = (!old_contact
		      || (new_contact->fullname && !old_contact->fullname)
		      || (old_contact->fullname && new_contact->fullname
			  && strcmp (old_contact->fullname, 
				     new_contact->fullname)));

  /* Check the full url if we are adding a contact or if we are editing
   * a contact and added an url, or changed the url
   */
  if (new_contact->url && strcmp (new_contact->url, ""))
    check_url = (!old_contact
		 || (new_contact->url && !old_contact->url)
		 || (old_contact->url && new_contact->url
		     && strcmp (old_contact->url, 
				new_contact->url)));

  /* Check the speed dial if we are adding a contact or if we are editing
   * a contact and added a speed dial, or changed the speed dial
   */
  if (new_contact->speeddial && strcmp (new_contact->speeddial, ""))
    check_speeddial = (!old_contact
		       || (new_contact->speeddial && !old_contact->speeddial)
		       || (old_contact->speeddial && new_contact->speeddial
			   && strcmp (old_contact->speeddial, 
				      new_contact->speeddial)));

  /* First do a search on the fields, then on the speed dials. Not clean, 
   * but E-D-S doesn't permit to do better for now...
   */
  while (cpt < 2) {

    if (cpt == 0) {

      /* Is there any user with the same speed dial ? */
      if (check_speeddial)
	contacts = 
	  gnomemeeting_addressbook_get_contacts (NULL,
						 nbr,
						 FALSE,
						 NULL,
						 NULL,
						 NULL,
						 NULL,
						 check_speeddial ?
						 new_contact->speeddial:
						 NULL);
    }
    else if (check_fullname || check_url)
      contacts = 
	gnomemeeting_addressbook_get_contacts (NULL,
					       nbr,
					       FALSE,
					       check_fullname ?
					       new_contact->fullname:
					       NULL,
					       check_url?
					       new_contact->url:
					       NULL,
					       NULL,
					       NULL,
					       NULL);

    if (contacts && contacts->data) {

      ctct = GM_CONTACT (contacts->data);

      primary_text = g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>", _("Contact collision"));
      if (cpt == 0)
	secondary_text = g_strdup_printf (_("Another contact with the same speed dial already exists in your address book:\n\n<b>Name</b>: %s\n<b>URL</b>: %s\n<b>Speed Dial</b>: %s\n"), ctct->fullname?ctct->fullname:_("None"), ctct->url?ctct->url:_("None"), ctct->speeddial?ctct->speeddial:_("None"));
      else
	secondary_text = g_strdup_printf (_("Another contact with similar information already exists in your address book:\n\n<b>Name</b>: %s\n<b>URL</b>: %s\n<b>Speed Dial</b>: %s\n\nDo you still want to add the contact?"), ctct->fullname?ctct->fullname:_("None"), ctct->url?ctct->url:_("None"), ctct->speeddial?ctct->speeddial:_("None"));


      dialog_text =
	g_strdup_printf ("%s\n\n%s", primary_text, secondary_text);

      dialog =
	gtk_message_dialog_new (parent_window ? GTK_WINDOW (parent_window) : GTK_WINDOW (addressbook_window),
				GTK_DIALOG_MODAL,
				(cpt == 0) ? 
				GTK_MESSAGE_ERROR
				:
				GTK_MESSAGE_WARNING,
				(cpt == 0) ? 
				GTK_BUTTONS_OK
				:
				GTK_BUTTONS_YES_NO, 
				NULL);

      gtk_window_set_title (GTK_WINDOW (dialog), "");
      gtk_label_set_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label),
			    dialog_text);

      switch (gtk_dialog_run (GTK_DIALOG (dialog)))
	{
	case GTK_RESPONSE_YES:
	  to_return = FALSE;
	  break;

	default:
	  to_return = TRUE;

	}

      gtk_widget_destroy (dialog);

      g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
      g_slist_free (contacts);

      g_free (primary_text);
      g_free (secondary_text);
      g_free (dialog_text);

      break;
    }

    cpt++;
  }

  return to_return;
}


static void
gm_aw_add_addressbook (GtkWidget *addressbook_window,
		       GmAddressbook *addressbook)
{
  GmAddressbookWindow *aw = NULL;
  GmAddressbookWindowPage *awp = NULL;

  GtkWidget *page = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *scroll = NULL;

  GtkWidget *find_button = NULL;

  GdkPixbuf *contact_icon = NULL;

  GtkTreeViewColumn *column = NULL;
  GtkListStore *list_store = NULL;
  GtkCellRenderer *renderer = NULL;

  GtkTreeModel *aw_tree_model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter, child_iter;

  GSList *contacts = NULL;

  gboolean is_local = FALSE;
  int pos = 0;
  int nbr = 0;

  g_return_if_fail (addressbook_window != NULL);
  g_return_if_fail (addressbook != NULL);


  /* Get the Data */
  aw = gm_aw_get_aw (addressbook_window);
  awp = new GmAddressbookWindowPage ();

  if (gnomemeeting_addressbook_is_local (addressbook))
    is_local = TRUE;

  
  /* Add the given address book in the aw_tree_view GtkTreeView listing
   * all address books */
  contact_icon = 
    gtk_widget_render_icon (aw->aw_tree_view, 
			    is_local?
			    GM_STOCK_LOCAL_CONTACT
			    :
			    GM_STOCK_REMOTE_CONTACT,
			    GTK_ICON_SIZE_MENU, NULL);

  aw_tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->aw_tree_view));
  pos = gtk_notebook_get_n_pages (GTK_NOTEBOOK (aw->aw_notebook));

  if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (aw_tree_model), 
					   &iter, is_local ? "1" : "0")) {

    gtk_tree_store_append (GTK_TREE_STORE (aw_tree_model), &child_iter, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (aw_tree_model),
			&child_iter, 
			COLUMN_PIXBUF, contact_icon,
			COLUMN_NAME, addressbook->name,
			COLUMN_NOTEBOOK_PAGE, pos, 
			COLUMN_PIXBUF_VISIBLE, TRUE,
			COLUMN_WEIGHT, PANGO_WEIGHT_NORMAL, 
			COLUMN_URL, addressbook->url, 
			COLUMN_AID, addressbook->aid, 
			COLUMN_CALL_ATTRIBUTE, addressbook->call_attribute, 
			-1);
  }

  gtk_tree_view_expand_all (GTK_TREE_VIEW (aw->aw_tree_view));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (aw->aw_tree_view));
  if (!gtk_tree_selection_get_selected (selection, NULL, NULL))
    gtk_tree_selection_select_iter (selection, &child_iter);


  /* Add the given address book in the aw_notebook GtkNotebook containing
   * the content of all address books */
  list_store = 
    gtk_list_store_new (NUM_COLUMNS_GROUPS, 
			GDK_TYPE_PIXBUF,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING, 
			G_TYPE_STRING, 
			G_TYPE_INT);

  vbox = gtk_vbox_new (FALSE, 0);
  scroll = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  gtk_notebook_append_page (GTK_NOTEBOOK (aw->aw_notebook), vbox, NULL);
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (aw->aw_notebook), pos);
  g_object_set_data_full (G_OBJECT (page), "GMObject", 
			  awp, (GDestroyNotify) gm_awp_destroy);

  awp->awp_tree_view = 
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (awp->awp_tree_view), TRUE);
  gmcontacts_dnd_set_source (GTK_WIDGET (awp->awp_tree_view),
			      dnd_get_contact_cb, NULL);


  renderer = gtk_cell_renderer_pixbuf_new ();
  /* Translators: This is "S" as in "Status" */
  column = gtk_tree_view_column_new_with_attributes (_("S"),
						     renderer,
						     "pixbuf", 
						     COLUMN_STATUS,
						     NULL);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 150);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  if (!gnomemeeting_addressbook_has_state (addressbook))
    g_object_set (G_OBJECT (column), "visible", false, NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Name"),
						     renderer,
						     "text", 
						     COLUMN_FULLNAME,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_FULLNAME);
  gtk_tree_view_column_set_min_width (GTK_TREE_VIEW_COLUMN (column), 125);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
				   GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (column), true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  gtk_tree_view_column_add_attribute (column, renderer, "weight", 
				      COLUMN_USER_WEIGHT);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Comment"),
						     renderer,
						     "text", 
						     COLUMN_COMMENT,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_COMMENT);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
				   GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  if (!gnomemeeting_addressbook_has_comment (addressbook))
    g_object_set (G_OBJECT (column), "visible", false, NULL);
  g_object_set (G_OBJECT (renderer), "style", PANGO_STYLE_ITALIC, NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Software"),
						     renderer,
						     "text", 
						     COLUMN_SOFTWARE,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_SOFTWARE);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
				   GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  if (!gnomemeeting_addressbook_has_software (addressbook))
    g_object_set (G_OBJECT (column), "visible", false, NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("VoIP URL"),
						     renderer,
						     "text", 
						     COLUMN_UURL,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_UURL);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
				   GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  if (!gnomemeeting_addressbook_has_url (addressbook))
    g_object_set (G_OBJECT (column), "visible", false, NULL);
  g_object_set (G_OBJECT (renderer), "foreground", "blue",
		"underline", TRUE, NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("E-Mail"),
						     renderer,
						     "text", 
						     COLUMN_EMAIL,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_EMAIL);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
				   GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  if (!gnomemeeting_addressbook_has_email (addressbook))
    g_object_set (G_OBJECT (column), "visible", false, NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Location"),
						     renderer,
						     "text", 
						     COLUMN_LOCATION,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_LOCATION);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
				   GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  if (!gnomemeeting_addressbook_has_location (addressbook))
    g_object_set (G_OBJECT (column), "visible", false, NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Categories"),
						     renderer,
						     "text", 
						     COLUMN_CATEGORIES,
						     NULL);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
				   GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  if (!gnomemeeting_addressbook_has_categories (addressbook))
    g_object_set (G_OBJECT (column), "visible", false, NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Speed Dial"),
						     renderer,
						     "text", 
						     COLUMN_SPEED_DIAL,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_SPEED_DIAL);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
				   GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (awp->awp_tree_view), column);
  if (!gnomemeeting_addressbook_has_speeddial (addressbook))
    g_object_set (G_OBJECT (column), "visible", false, NULL);


  /* Add the tree view */
  gtk_container_add (GTK_CONTAINER (scroll), awp->awp_tree_view);
  gtk_container_set_border_width (GTK_CONTAINER (awp->awp_tree_view), 0);
  gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
  gtk_widget_show_all (page);

  
  /* The search entry */
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  
  /* The option menu */
    
  awp->awp_option_menu = gtk_combo_box_new_text ();

  gtk_combo_box_append_text (GTK_COMBO_BOX (awp->awp_option_menu), _("Name contains"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (awp->awp_option_menu), _("URL contains"));
  if (gnomemeeting_addressbook_is_local (addressbook))
    gtk_combo_box_append_text (GTK_COMBO_BOX (awp->awp_option_menu), _("Belongs to category"));
  else
    gtk_combo_box_append_text (GTK_COMBO_BOX (awp->awp_option_menu), _("Location contains"));

  gtk_combo_box_set_active (GTK_COMBO_BOX (awp->awp_option_menu), 0);

  gtk_box_pack_start (GTK_BOX (hbox), awp->awp_option_menu, FALSE, FALSE, 2);

  /* The entry */
  awp->awp_search_entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), awp->awp_search_entry, TRUE, TRUE, 2);

  
  /* The Find button */
  find_button = gtk_button_new_from_stock (GTK_STOCK_FIND);
  gtk_box_pack_start (GTK_BOX (hbox), find_button, FALSE, FALSE, 2);
  gtk_widget_show_all (hbox);

  
  /* The statusbar */
  awp->awp_statusbar = gtk_statusbar_new ();
  gtk_box_pack_start (GTK_BOX (vbox), awp->awp_statusbar, FALSE, FALSE, 0);
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (awp->awp_statusbar), TRUE);
  gtk_widget_show_all (awp->awp_statusbar);
  

  /* Connect the signals */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (awp->awp_tree_view));
  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK (contact_selected_cb), 
		    addressbook_window);
  
  g_signal_connect (G_OBJECT (find_button), "clicked",
		    G_CALLBACK (search_addressbook1_cb),
		    addressbook_window);

  g_signal_connect (G_OBJECT (awp->awp_search_entry), "activate",
		    G_CALLBACK (search_addressbook1_cb),
		    addressbook_window);


  g_signal_connect (G_OBJECT (awp->awp_tree_view), "event_after",
		    G_CALLBACK (contact_clicked_cb), 
		    addressbook_window);

  g_signal_connect (G_OBJECT (awp->awp_tree_view), "row-activated",
		    G_CALLBACK (call_contact2_cb), 
		    addressbook_window); 


  /* Update the address book content in the GUI */
  if (gnomemeeting_addressbook_is_local (addressbook)) {
   
    contacts =
      gnomemeeting_addressbook_get_contacts (addressbook, nbr, FALSE, 
					     NULL, NULL, NULL, NULL, NULL);
    gm_aw_update_addressbook (addressbook_window,
			      addressbook,
			      contacts,
			      NULL);
    g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
    g_slist_free (contacts);
  }
}


static void
gm_aw_delete_addressbook (GtkWidget *addressbook_window,
			  GmAddressbook *addressbook)
{
  GmAddressbookWindow *aw = NULL;

  GtkTreeSelection *selection = NULL;
  GtkTreePath *path = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gchar *test = NULL;

  int p = -1;

  g_return_if_fail (addressbook_window != NULL);

  aw = gm_aw_get_aw (addressbook_window);

  g_return_if_fail (addressbook != NULL && aw != NULL);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->aw_tree_view));

  for (int i = 0 ; i < 2 ; i++) {

    if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), 
					     &iter, (i == 0) ? "0:0" : "1:0")) {

      do {

	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			    COLUMN_AID, &test, 
			    COLUMN_NOTEBOOK_PAGE, &p,
			    -1);

	if (test && addressbook->aid && !strcmp (test, addressbook->aid)) {

	  gtk_notebook_remove_page (GTK_NOTEBOOK (aw->aw_notebook), p);
	  gtk_tree_store_remove (GTK_TREE_STORE (model), &iter);
	  g_free (test);
	  break;
	}
	g_free (test);

      } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
    }
  }

  
  gtk_tree_view_expand_all (GTK_TREE_VIEW (aw->aw_tree_view));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (aw->aw_tree_view));
  if (!gtk_tree_selection_get_selected (selection, NULL, NULL)) {
    
    path = gtk_tree_path_new_from_string (gnomemeeting_addressbook_is_local (addressbook)?"1:0":"0:0");
    gtk_tree_selection_select_path (selection, path);
    gtk_tree_path_free (path);
  }
}


static void
gm_aw_modify_addressbook (GtkWidget *addressbook_window, 
			  GmAddressbook *addb) 
{
  GmAddressbookWindow *aw = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gchar *test = NULL;

  int p = -1;

  g_return_if_fail (addressbook_window != NULL);

  aw = gm_aw_get_aw (addressbook_window);

  g_return_if_fail (addb != NULL && aw != NULL);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->aw_tree_view));

  for (int i = 0 ; i < 2 ; i++) {

    if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), 
					     &iter,
					     (i == 0) ? "0:0" : "1:0")) {

      do {

	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			    COLUMN_AID, &test, 
			    COLUMN_NOTEBOOK_PAGE, &p,
			    -1);

	if (test && addb->aid && !strcmp (test, addb->aid)) {

	  gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
			      COLUMN_AID, addb->aid,
			      COLUMN_URL, addb->url,
			      COLUMN_NAME, addb->name,
			      COLUMN_CALL_ATTRIBUTE, addb->call_attribute,
			      COLUMN_NOTEBOOK_PAGE, p,
			      -1);
	  g_free (test);
	  break;
	}
	g_free (test);

      } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
    }
  }

}


static void
gm_aw_update_addressbook (GtkWidget *addressbook_window,
			  GmAddressbook *addressbook,
			  GSList *contacts,
			  gchar *msg)
{
  GmAddressbookWindow *aw = NULL;
  GmAddressbookWindowPage *awp = NULL;

  GSList *l = NULL;

  GmContact *contact = NULL;

  GtkWidget *page = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  GdkPixbuf *status_icon = NULL;

  int page_num = -1;

  g_return_if_fail (addressbook_window != NULL && addressbook != NULL);

  page_num = 
    gm_aw_get_notebook_page (addressbook_window,
			     addressbook);

  if (page_num == -1)
    return;

  aw = gm_aw_get_aw (addressbook_window);

  g_return_if_fail (aw != NULL);

  page =
    gtk_notebook_get_nth_page (GTK_NOTEBOOK (aw->aw_notebook), page_num);
  
  if (!page)
    return;
  
  awp = gm_aw_get_awp (page);

  g_return_if_fail (awp != NULL);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (awp->awp_tree_view));

  gtk_list_store_clear (GTK_LIST_STORE (model));

  
  l = contacts;
  while (l) {

    contact = (GmContact *) (l->data);
    gtk_list_store_append (GTK_LIST_STORE (model), &iter);

    if (contact->fullname)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_FULLNAME, contact->fullname, -1);
    if (contact->comment)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_COMMENT, contact->comment, -1);
    if (contact->location)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_LOCATION, contact->location, -1);
    if (contact->email)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_EMAIL, contact->email, -1);
    if (contact->url)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_UURL, contact->url, -1);
    if (contact->software)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_SOFTWARE, contact->software, -1);
    if (contact->categories)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_CATEGORIES, contact->categories, -1);
    if (contact->speeddial)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_SPEED_DIAL, contact->speeddial, -1);
    if (contact->uid)
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_UUID, contact->uid, -1);

    /* Support only 3 basic states for now */
    switch (contact->state) {
    case CONTACT_AVAILABLE:
      status_icon = 
	gtk_widget_render_icon (addressbook_window,
				GM_STOCK_STATUS_AVAILABLE,
				GTK_ICON_SIZE_MENU, NULL);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_USER_WEIGHT, PANGO_WEIGHT_BOLD, -1);
      break;
    case CONTACT_BUSY:
      status_icon = 
	gtk_widget_render_icon (addressbook_window,
				GM_STOCK_STATUS_DO_NOT_DISTURB,
				GTK_ICON_SIZE_MENU, NULL);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			  COLUMN_USER_WEIGHT, PANGO_WEIGHT_BOLD, -1);
      break;
    default:
      status_icon = 
	gtk_widget_render_icon (addressbook_window,
				GM_STOCK_STATUS_OFFLINE,
				GTK_ICON_SIZE_MENU, NULL);
      break;
    }

    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			COLUMN_STATUS, status_icon, -1);

    g_object_unref (status_icon);

    l = g_slist_next (l);
  }

  if (msg) {

    gtk_statusbar_pop (GTK_STATUSBAR (awp->awp_statusbar), 0);
    gtk_statusbar_push (GTK_STATUSBAR (awp->awp_statusbar), 0, msg);
  }
}


static void 
gm_aw_update_menu_sensitivity (GtkWidget *addressbook_window,
			       gboolean is_editable,
			       gboolean is_local_selected,
			       gboolean is_remote_selected)
{
  GmAddressbookWindow *aw = NULL;
  GmContact *contact = NULL;

  gboolean is_sip = FALSE;

  g_return_if_fail (addressbook_window != NULL);
  g_return_if_fail (not (is_remote_selected && is_local_selected));

  aw = gm_aw_get_aw (addressbook_window);

  contact = gm_aw_get_selected_contact (addressbook_window);

  if (contact)
    is_sip = (GMURL (contact->url).GetType () == "sip");

  gtk_menu_set_sensitive (aw->aw_menu, "call",
			  is_remote_selected || is_local_selected);
  gtk_menu_set_sensitive (aw->aw_menu, "delete", (is_local_selected 
						  || (!is_remote_selected
						      && is_editable)));
  gtk_menu_set_sensitive (aw->aw_menu, "add", is_remote_selected);
  gtk_menu_set_sensitive (aw->aw_menu, "properties", (is_local_selected
						      || (!is_remote_selected
							  && is_editable)));
  gtk_menu_set_sensitive (aw->aw_menu, "message",
			  is_remote_selected || is_local_selected && is_sip);
  gtk_menu_set_sensitive (aw->aw_menu, "copy",
			  is_remote_selected || is_local_selected);
  gtk_menu_set_sensitive (aw->aw_menu, "emailwrite",
			  is_remote_selected || is_local_selected);
}


static gint
gm_aw_get_notebook_page (GtkWidget *addressbook_window,
			 GmAddressbook *addressbook)
{
  GmAddressbookWindow *aw = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gchar *test = NULL;
  int p = 0;

  g_return_val_if_fail (addressbook_window != NULL, -1);
  g_return_val_if_fail (addressbook != NULL, -1);

  aw = gm_aw_get_aw (addressbook_window);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (aw->aw_tree_view));

  for (int i = 0 ; i < 2 ; i++) {

    if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), 
					     &iter, (i == 0) ? "0:0" : "1:0")) {

      do {

	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			    COLUMN_NOTEBOOK_PAGE, &p,
			    COLUMN_AID, &test, 
			    -1);

	if (test && addressbook->aid && !strcmp (test, addressbook->aid)) {

	  g_free (test);
	  return p;
	}

	g_free (test);

      } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
    }
  }

  return -1;
}


GtkWidget *
gm_aw_contact_menu_new (GtkWidget *addressbook_window,
			GmAddressbook *addressbook,
			GmContact *contact)
{
  GtkWidget *chat_window = NULL;
  GtkWidget *menu = NULL;
  
  gboolean local = TRUE;
  gboolean is_sip = FALSE;
  gboolean has_email = TRUE;

  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  
  if (!addressbook || !gnomemeeting_addressbook_is_local (addressbook))
    local = FALSE;
 
  if (contact)
    is_sip = (GMURL (contact->url).GetType () == "sip");

  /* mi_ variables: (m)enu(i)tem, mi_sm_ indicates a (s)ub(m)enu, 
   * implemented as array */
  static MenuEntry mi_call_contact =
    /* call a contact, usage: general */
    GTK_MENU_ENTRY("call", _("C_all Contact"), NULL,
		   GM_STOCK_CONNECT_16, 0,
		   GTK_SIGNAL_FUNC (call_contact1_cb),
		   addressbook_window, TRUE);

  static MenuEntry mi_copy_url =
    /* copy a contact's URL to clipboard, usage: general */
    GTK_MENU_ENTRY("copy", _("_Copy URL to Clipboard"), NULL,
		   GTK_STOCK_COPY, 0,
		   GTK_SIGNAL_FUNC (copy_url_to_clipboard_cb),
		   addressbook_window, TRUE);

  static MenuEntry mi_email = 
    GTK_MENU_ENTRY("emailwrite", _("_Write e-Mail"), NULL,
		   GM_STOCK_EDIT, 0,
		   GTK_SIGNAL_FUNC (write_email_with_uricall_cb),
		   addressbook_window, has_email);

  static MenuEntry mi_add_to_local =
    /* add a contact to the local addressbook, usage: remote contacts only */
    GTK_MENU_ENTRY("add", _("Add Contact to _Address Book"), NULL,
		   GTK_STOCK_ADD, 0,
		   GTK_SIGNAL_FUNC (properties_cb),
		   addressbook_window, TRUE);

  static MenuEntry mi_send_message =
    /* send a contact a (SIP!) message, usage: SIP contacts only */
    GTK_MENU_ENTRY("message", _("_Send Message"), NULL,
		   GM_STOCK_MESSAGE, 0,
		   GTK_SIGNAL_FUNC (show_chat_window_cb),
		   chat_window, TRUE);

  static MenuEntry mi_edit_properties =
    /* edit a local contact's addressbook entry, usage: local contacts */
    GTK_MENU_ENTRY("properties", _("_Properties"), NULL,
		   GTK_STOCK_PROPERTIES, 0,
		   GTK_SIGNAL_FUNC (properties_cb),
		   addressbook_window, TRUE);

  static MenuEntry mi_delete_local =
    /* delete a local contact entry, usage: local contacts */
    GTK_MENU_ENTRY("delete", _("_Delete"), NULL,
		   GTK_STOCK_DELETE, 'd',
		   GTK_SIGNAL_FUNC (delete_cb),
		   addressbook_window, TRUE);

  static MenuEntry mi_new_contact =
    /* "new contact" dialog, usage: local context */
    GTK_MENU_ENTRY("add", _("New _Contact"), NULL,
		   GTK_STOCK_NEW, 0,
		   GTK_SIGNAL_FUNC (new_contact_cb),
		   addressbook_window, TRUE);

  static MenuEntry add_contact_menu_local [] =
    {
      mi_new_contact,

      GTK_MENU_END
    };

  static MenuEntry contact_menu_local [] =
    {
      mi_call_contact,

      mi_copy_url,

      mi_email,

      GTK_MENU_SEPARATOR,

      mi_edit_properties,

      GTK_MENU_SEPARATOR,

      mi_delete_local,

      GTK_MENU_SEPARATOR,

      mi_new_contact,

      GTK_MENU_END
    };


  static MenuEntry contact_menu_sip_local [] =
    {
      mi_call_contact,

      mi_send_message,

      mi_copy_url,

      mi_email,

      GTK_MENU_SEPARATOR,

      mi_edit_properties,

      GTK_MENU_SEPARATOR,

      mi_delete_local,

      GTK_MENU_SEPARATOR,

      mi_new_contact,

      GTK_MENU_END
    };


  static MenuEntry contact_menu_not_local [] =
    {
      mi_call_contact,

      mi_copy_url,

      mi_email,

      GTK_MENU_SEPARATOR,

      mi_add_to_local,

      GTK_MENU_END
    };


  static MenuEntry contact_menu_sip_not_local [] =
    {
      mi_call_contact,

      mi_send_message,

      mi_copy_url,

      mi_email,

      GTK_MENU_SEPARATOR,

      mi_add_to_local,

      GTK_MENU_END
    };

  if (contact && addressbook) {

    menu = gtk_menu_new ();
    if (local)
      gtk_build_menu (menu, is_sip?contact_menu_sip_local:contact_menu_local, NULL, NULL);
    else
      gtk_build_menu (menu, is_sip?contact_menu_sip_not_local:contact_menu_not_local, NULL, NULL);
  }
  else if (local) {

    menu = gtk_menu_new ();
    gtk_build_menu (menu, add_contact_menu_local, NULL, NULL);
  }

  return menu;
}


GtkWidget *
gm_aw_addressbook_menu_new (GtkWidget *addressbook_window)
{
  GtkWidget *menu = NULL;

  menu = gtk_menu_new ();

  
  static MenuEntry addressbook_menu [] =
    {
      GTK_MENU_ENTRY("properties", _("_Properties"), NULL,
		     GTK_STOCK_PROPERTIES, 0, 
		     GTK_SIGNAL_FUNC (properties_cb), 
		     addressbook_window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("delete", _("_Delete"), NULL,
		     GTK_STOCK_DELETE, 'd', 
		     GTK_SIGNAL_FUNC (delete_cb), 
		     addressbook_window, TRUE),

      GTK_MENU_END
    };

      
  gtk_build_menu (menu, addressbook_menu, NULL, NULL);

  return menu;
}


/* The Callbacks */
static gboolean 
aw_tree_selection_function_cb (GtkTreeSelection *selection,
			       GtkTreeModel *model,
			       GtkTreePath *path,
			       gboolean path_currently_selected,
			       gpointer data)
{
  if (gtk_tree_path_get_depth (path) <= 1)
    return FALSE;
  else
    return TRUE;
}


static void
call_contact1_cb (GtkWidget *widget,
		  gpointer data)
{
  GMManager *ep = NULL;
  
  GtkWidget *addressbook_window = NULL;
  GmContact *contact = NULL;

  GtkWidget *main_window = NULL;

  g_return_if_fail (data != NULL);


  addressbook_window = GTK_WIDGET (data);

  ep = GnomeMeeting::Process ()->GetManager ();
  
  if (ep->GetCallingState () != GMManager::Standby)
    return;

  contact = gm_aw_get_selected_contact (addressbook_window);

  if (contact) {
    /* present the main window */
    main_window = GnomeMeeting::Process ()->GetMainWindow ();
    gtk_window_present (GTK_WINDOW (main_window));

    /* Call the selected contact */
    GnomeMeeting::Process ()->Connect (contact->url);
    gmcontact_delete (contact);
  }
}


static void
show_chat_window_cb (GtkWidget *widget,
		     gpointer data)
{
  GMManager *ep = NULL;
  GmContact *contact = NULL;

  GtkWidget *addressbook_window = NULL;
  GtkWidget *chat_window = NULL;
  GtkWidget *statusicon = NULL;

  gchar *url = NULL;
  gchar *name = NULL;
  
  g_return_if_fail (data != NULL);

  chat_window = GTK_WIDGET (data);
  addressbook_window = GnomeMeeting::Process ()->GetAddressbookWindow ();
  statusicon = GnomeMeeting::Process ()->GetStatusicon ();

  ep = GnomeMeeting::Process ()->GetManager ();
  
  contact = gm_aw_get_selected_contact (addressbook_window);
  
  g_return_if_fail (contact != NULL);

  /* Check if there is an active call */
  gdk_threads_leave ();
  ep->GetCurrentConnectionInfo (name, url);
  gdk_threads_enter ();

  /* Add the tab if required */
  if (!gm_text_chat_window_has_tab (chat_window, contact->url)) {

    gm_text_chat_window_add_tab (chat_window, contact->url, contact->fullname);

    if (GMURL (url) == GMURL (contact->url))
      gm_chat_window_update_calling_state (chat_window, name, url, 
					   GMManager::Connected);
  }
  
  /* If the window is hidden, show it */
  if (!gnomemeeting_window_is_visible (GTK_WIDGET (data)))
    gnomemeeting_window_show (GTK_WIDGET (data));

  /* Reset the tray */
  gm_statusicon_signal_message (GTK_WIDGET (statusicon), FALSE);

  
  gmcontact_delete (contact);
  g_free (url);
  g_free (name);
}


static void
call_contact2_cb (GtkTreeView *tree_view,
		  GtkTreePath *arg1,
		  GtkTreeViewColumn *arg2,
		  gpointer data)
{
  g_return_if_fail (data != NULL);

  call_contact1_cb (NULL, data);
}


static void
new_contact_cb (GtkWidget *unused,
		gpointer data)
{
  GmAddressbook *abook = NULL;

  GtkWidget *addressbook = NULL;

  g_return_if_fail (data != NULL);

  addressbook = GTK_WIDGET (data);

  abook = gm_aw_get_selected_addressbook (addressbook);

  if (abook) {
    
    gm_addressbook_window_edit_contact_dialog_run (addressbook,
						   abook, 
						   NULL, 
						   FALSE,
						   addressbook);

    gm_addressbook_delete (abook);
  }
  
}


static void
new_addressbook_cb (GtkWidget *unused,
		    gpointer data)
{
  GtkWidget *addressbook_window = NULL;

  g_return_if_fail (data != NULL);

  addressbook_window = GTK_WIDGET (data);

  gm_addressbook_window_edit_addressbook_dialog_run (addressbook_window,
						     NULL,
						     addressbook_window);
}


static void
delete_cb (GtkWidget *unused,
	   gpointer data)
{
  GmContact *contact = NULL;
  GmAddressbook *abook = NULL;

  GtkWidget *addressbook_window = NULL;

  g_return_if_fail (data != NULL);

  addressbook_window = GTK_WIDGET (data);

  contact = gm_aw_get_selected_contact (addressbook_window);
  abook = gm_aw_get_selected_addressbook (addressbook_window);

  if (contact)
    gm_addressbook_window_delete_contact_dialog_run (addressbook_window, 
						     abook, 
						     contact, 
						     addressbook_window);
  else if (abook)
    gm_addressbook_window_delete_addressbook_dialog_run (addressbook_window,
							 abook,
							 addressbook_window);

  gmcontact_delete (contact);  
  gm_addressbook_delete (abook);
}


static void
properties_cb (GtkWidget *unused,
	       gpointer data)
{
  GmContact *contact = NULL;
  GmAddressbook *abook = NULL;
  gboolean edit_existing = FALSE;
  GtkWidget *addressbook_window = NULL;

  g_return_if_fail (data != NULL);

  addressbook_window = GTK_WIDGET (data);

  contact = gm_aw_get_selected_contact (addressbook_window);
  abook = gm_aw_get_selected_addressbook (addressbook_window);
  edit_existing = gnomemeeting_addressbook_is_local (abook);

  if (contact)
    gm_addressbook_window_edit_contact_dialog_run (addressbook_window,
						   abook, 
						   contact, 
						   edit_existing,
						   addressbook_window);
  else if (abook)
    gm_addressbook_window_edit_addressbook_dialog_run (addressbook_window,
						       abook,
						       addressbook_window);

  gmcontact_delete (contact);  
  gm_addressbook_delete (abook);
}


class SearchThread : public PThread
{
  PCLASSINFO (SearchThread, PThread);

public:
  SearchThread (GtkWidget *w)
    :PThread (1000, NoAutoDeleteThread), 
    addressbook_window (w) 
      { 
	Resume (); 
      }

  ~SearchThread ()
    {
      PWaitAndSignal m(quit_mutex);
    }

  void Main ()
    { 
      GSList *contacts = NULL;

      GdkCursor *cursor = NULL;
	
      int option = 0;
      int nbr = 0;

      gchar *filter = NULL;
      gchar *msg = NULL;
      
      PWaitAndSignal m(quit_mutex);

      /* Get the search parameters from the addressbook_window */
      gdk_threads_enter ();
      gm_aw_get_search_filter (addressbook_window, option, filter);
      addressbook = gm_aw_get_selected_addressbook (addressbook_window);
      gdk_threads_leave ();

      if (!addressbook)
        return;

      gdk_threads_enter ();
      cursor = gdk_cursor_new (GDK_WATCH);
      gdk_window_set_cursor (GTK_WIDGET (addressbook_window)->window, cursor);
      gdk_cursor_unref (cursor);
      gdk_threads_leave ();

      contacts =
	gnomemeeting_addressbook_get_contacts (addressbook, 
					       nbr,
					       TRUE,
					       (option == 0)?filter:NULL,
					       (option == 1)?filter:NULL,
					       (option == 2)?filter:NULL,
					       (option == 2)?filter:NULL,
					       NULL);

      if (nbr == -1)
	msg = g_strdup_printf (_("Error while fetching users list from %s"),
			       addressbook->name);
      /* Translators, pay attention to the singular/plural distinction */
      else if (nbr == (int) g_slist_length (contacts))
	msg = g_strdup_printf (ngettext ("Found %d user in %s", 
					 "Found %d users in %s", nbr), 
			       nbr, addressbook->name);
      else
	msg = g_strdup_printf (ngettext ("Found %d user in %s for a "
					 "total of %d users",
					 "Found %d users in %s for a "
					 "total of %d users", nbr),
			       g_slist_length (contacts),
			       addressbook->name, nbr);
	
      gdk_threads_enter ();
      gm_aw_update_addressbook (addressbook_window, 
				addressbook,
				contacts,
				msg);
      gdk_window_set_cursor (GTK_WIDGET (addressbook_window)->window, NULL);
      gdk_threads_leave ();

      gm_addressbook_delete (addressbook);

      g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
      g_slist_free (contacts);

      g_free (filter);
      g_free (msg);
    }
protected:
  GmAddressbook *addressbook;
  GtkWidget *addressbook_window;

  PMutex quit_mutex;
};


static void
search_addressbook1_cb (GtkWidget *unused,
			gpointer data)
{
  GtkWidget *addressbook_window = NULL;

  g_return_if_fail (data != NULL);

  addressbook_window = GTK_WIDGET (data);

  new SearchThread (addressbook_window);
}


static void
search_addressbook2_cb (GtkTreeView *tree_view,
			GtkTreePath *arg1,
			GtkTreeViewColumn *arg2,
			gpointer data)
{
  g_return_if_fail (data != NULL);

  search_addressbook1_cb (NULL, data);
}


static gint
contact_clicked_cb (GtkWidget *unused,
		    GdkEventButton *event,
		    gpointer data)
{
  GtkWidget *menu = NULL;
  
  GmAddressbook *addressbook = NULL;
  GmContact *contact = NULL;
  
  g_return_val_if_fail (data != NULL, FALSE);

  addressbook = 
    GM_ADDRESSBOOK (gm_aw_get_selected_addressbook (GTK_WIDGET (data)));

  contact = gm_aw_get_selected_contact (GTK_WIDGET (data));

  if (event->type == GDK_BUTTON_PRESS || event->type == GDK_KEY_PRESS) {

    if (event->button == 3) {

      menu = gm_aw_contact_menu_new (GTK_WIDGET (data), addressbook, contact);
      if (menu) {
	
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
			event->button, event->time);
	g_signal_connect (G_OBJECT (menu), "hide",
			  GTK_SIGNAL_FUNC (g_object_unref), (gpointer) menu);
	g_object_ref (G_OBJECT (menu));
	gtk_object_sink (GTK_OBJECT (menu));
      }
    }

    gmcontact_delete (contact);
  }
  
  gm_addressbook_delete (addressbook);

  return TRUE;
}


static gint
addressbook_clicked_cb (GtkWidget *unused,
			GdkEventButton *event,
			gpointer data)
{
  GtkWidget *menu = NULL;
  
  GmAddressbook *addressbook = NULL;
  
  g_return_val_if_fail (data != NULL, FALSE);

  addressbook = 
    GM_ADDRESSBOOK (gm_aw_get_selected_addressbook (GTK_WIDGET (data)));

  if (addressbook) {

    if (event->type == GDK_BUTTON_PRESS || event->type == GDK_KEY_PRESS) {

      if (event->button == 3 
	  && gnomemeeting_addressbook_is_editable (addressbook)) {

	menu = gm_aw_addressbook_menu_new (GTK_WIDGET (data));
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
			event->button, event->time);
	g_signal_connect (G_OBJECT (menu), "hide",
			  GTK_SIGNAL_FUNC (g_object_unref), (gpointer) menu);
	g_object_ref (G_OBJECT (menu));
	gtk_object_sink (GTK_OBJECT (menu));
      }
    }
  }

  gm_addressbook_delete (addressbook);

  return TRUE;
}


static void
contact_selected_cb (GtkTreeSelection *selection,
		     gpointer data)
{
  GtkWidget *addressbook_window = NULL;
  GmAddressbook *addressbook = NULL;
  GmContact *contact = NULL;

  gboolean editable = FALSE;
  gboolean ls = FALSE;
  gboolean rs = FALSE;

  g_return_if_fail (data != NULL);

  addressbook_window = GTK_WIDGET (data);

  addressbook = 
    GM_ADDRESSBOOK (gm_aw_get_selected_addressbook (addressbook_window));
  contact = gm_aw_get_selected_contact (addressbook_window);

  if (contact) {

    editable = gnomemeeting_addressbook_is_editable (addressbook);
    ls = gnomemeeting_addressbook_is_local (addressbook);
    rs = !ls;
    gmcontact_delete (contact);
  }

  gm_aw_update_menu_sensitivity (GTK_WIDGET (data),
				 editable, ls, rs);

  gm_addressbook_delete (addressbook);
}


static void
addressbook_selected_cb (GtkTreeSelection *selection,
			 gpointer data)
{
  GtkWidget *page = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *lselection = NULL;
  GtkTreeIter iter;

  GmAddressbook *addressbook = NULL;
  GmAddressbookWindow *aw = NULL;
  GmAddressbookWindowPage *awp = NULL;

  gint page_num = -1;
  gboolean editable = FALSE;

  g_return_if_fail (data != NULL);
  
  addressbook = 
    GM_ADDRESSBOOK (gm_aw_get_selected_addressbook (GTK_WIDGET (data)));
  if (addressbook) {

    editable = gnomemeeting_addressbook_is_editable (addressbook);
    gm_addressbook_delete (addressbook);

    aw = gm_aw_get_aw (GTK_WIDGET (data));

    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			  COLUMN_NOTEBOOK_PAGE, &page_num, -1);

      /* Select the good notebook page for the contact section */
      if (page_num != -1) {

	/* Selects the good notebook page */
	gtk_notebook_set_current_page (GTK_NOTEBOOK (aw->aw_notebook), 
				       page_num);	

	/* Unselect all rows of the list store in that notebook page */
	page =
	  gtk_notebook_get_nth_page (GTK_NOTEBOOK (aw->aw_notebook), page_num);

	if (page)
	  awp = gm_aw_get_awp (GTK_WIDGET (page));

	if (awp) {

	  lselection =
	    gtk_tree_view_get_selection (GTK_TREE_VIEW (awp->awp_tree_view));

	  if (lselection)
	    gtk_tree_selection_unselect_all (GTK_TREE_SELECTION (lselection));
	}
      }
    }
  }
  gm_aw_update_menu_sensitivity (GTK_WIDGET (data),
				 editable, FALSE, FALSE);
}


static void 
edit_addressbook_type_menu_changed_cb (GtkComboBox *menu,
				       gpointer data)
{
  g_return_if_fail (data != NULL);

  if (gtk_combo_box_get_active (GTK_COMBO_BOX (menu)) == 0)
    gtk_widget_hide_all (GTK_WIDGET (data));
  else
    gtk_widget_show_all (GTK_WIDGET (data));
}


static void
copy_url_to_clipboard_cb (GtkWidget *unused,
			  gpointer data)
{
  GtkClipboard *cb = NULL;
  GmContact *contact = NULL;

  GtkWidget *addressbook_window = NULL;

  g_return_if_fail (data != NULL);

  addressbook_window = GTK_WIDGET (data);

  contact = gm_aw_get_selected_contact (addressbook_window);

  if (contact && contact->url) {

    cb = gtk_clipboard_get (GDK_NONE);
    gtk_clipboard_set_text (cb, contact->url, -1);
    gmcontact_delete (contact);
  }
}


static void
write_email_with_uricall_cb (GtkWidget *unused,
			     gpointer data)
{

  gchar *email_uri = NULL;
	  
  GmContact *contact = NULL;

  GtkWidget *addressbook_window = NULL;

  g_return_if_fail (data != NULL);
  
  
  addressbook_window = GTK_WIDGET (data);

  contact = gm_aw_get_selected_contact (addressbook_window);

  if (contact && contact->email) {

    email_uri = g_strdup_printf ("mailto:%s <%s>", contact->fullname, contact->email);
    gm_open_uri (email_uri);

    g_free (email_uri);
    gmcontact_delete (contact);
  }
}



static gboolean
dnd_allow_drop_cb (GtkWidget *widget, 
		   gint x, 
		   gint y, 
		   gpointer unused)
{
  GtkTreePath *path = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  GmAddressbook *abook;
  gboolean result = false;
  
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (widget));

  /* find the row under cursor */
  if (gtk_tree_view_get_dest_row_at_pos (GTK_TREE_VIEW (widget),
					 x, y, &path, NULL)
      && gtk_tree_model_get_iter (model, &iter, path)) {
    /* find out if the addressbook is local */
    abook = gm_addressbook_new ();
    if (abook->aid)
      g_free (abook->aid);
    if (abook->url)
      g_free (abook->url);
    
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			COLUMN_NAME, &abook->name, 
			COLUMN_URL, &abook->url,
			COLUMN_AID, &abook->aid,
			COLUMN_CALL_ATTRIBUTE, &abook->call_attribute,
			-1);
    if (gnomemeeting_addressbook_is_local (abook)) {
      gtk_tree_view_set_drag_dest_row (GTK_TREE_VIEW (widget), path,
				       GTK_TREE_VIEW_DROP_INTO_OR_AFTER);
      result = true;
    }
    gm_addressbook_delete (abook);
  }
  
  return result;
}


static void
dnd_add_contact_server_cb (GtkWidget *widget,
			   GmContact *contact,
			   gint x, 
			   gint y, 
			   gpointer data)
{
  GtkTreePath *path = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  GtkWidget *window = NULL;
  GmAddressbook *abook = NULL;

  g_return_if_fail (data != NULL);
  
  window = (GtkWidget *)data;
  
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (widget));
  
  /* find the row of the drop */
  if (gtk_tree_view_get_dest_row_at_pos (GTK_TREE_VIEW (widget),
					 x, y, &path, NULL)
      && gtk_tree_model_get_iter (model, &iter, path)) {
    /* find out if the addressbook is local */
    abook = gm_addressbook_new ();
    if (abook->aid)
      g_free (abook->aid);
    if (abook->url)
      g_free (abook->url);
    
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			COLUMN_NAME, &abook->name, 
			COLUMN_URL, &abook->url,
			COLUMN_AID, &abook->aid,
			COLUMN_CALL_ATTRIBUTE, &abook->call_attribute,
			-1);
    if (gnomemeeting_addressbook_is_local (abook)) {
      gm_addressbook_window_edit_contact_dialog_run (window, abook, contact,
						     FALSE, window);
    }
    
    gm_addressbook_delete (abook);
  }
  
  gmcontact_delete (contact);
}


static void
dnd_add_contact_contactlist_cb (GtkWidget *widget,
				GmContact *contact,
				gint x,
				gint y,
				gpointer data)
{
  GtkWidget *window = NULL;
  GmAddressbook *abook = NULL;

  g_return_if_fail (data != NULL);

  window = (GtkWidget *)data;
  
  abook = gm_aw_get_selected_addressbook (window);
  
  if (gnomemeeting_addressbook_is_local (abook)) {
    gm_addressbook_window_edit_contact_dialog_run (window, abook, contact,
						   FALSE, window);
  }
  
  gm_addressbook_delete (abook);
  
  gmcontact_delete (contact);
}


static GmContact *
dnd_get_contact_cb (GtkWidget *widget, 
		    gpointer unused)
{
  GmContact *contact = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;
  
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (widget));
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    
    contact = gmcontact_new ();
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 
			COLUMN_FULLNAME, &contact->fullname, 
			COLUMN_SPEED_DIAL, &contact->speeddial,
			COLUMN_CATEGORIES, &contact->categories,
			COLUMN_UURL, &contact->url,
			COLUMN_EMAIL, &contact->email,
			COLUMN_UUID, &contact->uid,
			-1);
  }
  
  return contact;
}

/* Let's go for the implementation of the public API */

GtkWidget *
gm_addressbook_window_new ()
{
  GmAddressbookWindow *aw = NULL;
  GtkWidget *chat_window = NULL;

  GtkWidget *window = NULL;
  GtkWidget *hpaned = NULL;

  GtkWidget *main_vbox = NULL;
  GtkWidget *vbox2 = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *scroll = NULL;
  GdkPixbuf *icon = NULL;

  GtkCellRenderer *cell = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkAccelGroup *accel = NULL;
  GtkTreeStore *model = NULL;
  GtkTreeIter iter;

  GSList *addressbooks = NULL;
  GSList *l = NULL;

  int p = 0;


  /* The Top-level window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  icon = gtk_widget_render_icon (GTK_WIDGET (window),
				 GM_STOCK_ADDRESSBOOK_16,
				 GTK_ICON_SIZE_MENU, NULL);
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("address_book_window"), g_free);

  gtk_window_set_title (GTK_WINDOW (window), 
			_("Address Book"));
  gtk_window_set_icon (GTK_WINDOW (window), icon);
  gtk_window_set_position (GTK_WINDOW (window), 
			   GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (window), 670, 370);
  g_object_unref (icon);


  /* The GMObject data */
  aw = new GmAddressbookWindow ();
  g_object_set_data_full (G_OBJECT (window), "GMObject", 
			  aw, (GDestroyNotify) gm_aw_destroy);

  /* get the chat window */
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();

  /* The accelerators */
  accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (window), accel);
  /* Add a custom accelerator for ESC to hide the window */
  gtk_accel_group_connect (accel, GDK_Escape,
			   (GdkModifierType) 0,
			   GTK_ACCEL_LOCKED,
			   g_cclosure_new_swap (G_CALLBACK (hide_window_cb),
						(gpointer) window, NULL));


  /* A vbox that will contain the menubar, the hpaned containing
     the rest of the window */
  main_vbox = gtk_vbox_new (0, FALSE);
  gtk_container_add (GTK_CONTAINER (window), main_vbox);
  aw->aw_menu = gtk_menu_bar_new ();

  static MenuEntry addressbook_menu [] =
    {
      GTK_MENU_NEW(_("_File")),

      GTK_MENU_ENTRY("new_addressbook", _("New _Address Book"), NULL,
		     GM_STOCK_REMOTE_CONTACT, 'b', 
		     GTK_SIGNAL_FUNC (new_addressbook_cb),
		     window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("delete", _("_Delete"), NULL,
		     GTK_STOCK_DELETE, 'd', 
		     GTK_SIGNAL_FUNC (delete_cb), 
		     (gpointer) window, TRUE),

      GTK_MENU_ENTRY("properties", _("_Properties"), NULL,
		     GTK_STOCK_PROPERTIES, 0, 
		     GTK_SIGNAL_FUNC (properties_cb), 
		     (gpointer) window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("close", _("_Close"), NULL,
		     GTK_STOCK_CLOSE, 'w',
		     GTK_SIGNAL_FUNC (hide_window_cb),
		     (gpointer) window, TRUE),

      GTK_MENU_NEW(_("C_ontact")),

      GTK_MENU_ENTRY("call", _("C_all Contact"), NULL,
		     GM_STOCK_CONNECT_16, 0, 
		     GTK_SIGNAL_FUNC (call_contact1_cb), 
                     (gpointer) window, FALSE),

      GTK_MENU_ENTRY("message", _("_Send Message"), NULL,
		     GM_STOCK_MESSAGE, 0,
		     GTK_SIGNAL_FUNC (show_chat_window_cb),
		     (gpointer) chat_window, TRUE),

      GTK_MENU_ENTRY("copy", _("_Copy URL to Clipboard"), NULL,
		     GTK_STOCK_COPY, 0,
		     GTK_SIGNAL_FUNC (copy_url_to_clipboard_cb),
		     (gpointer) window, TRUE),

      GTK_MENU_ENTRY("emailwrite", _("_Write e-Mail"), NULL,
		     GM_STOCK_EDIT, 0,
		     GTK_SIGNAL_FUNC (write_email_with_uricall_cb),
		     (gpointer) window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("new_contact", _("New _Contact"), NULL,
		     GTK_STOCK_NEW, 'n', 
		     GTK_SIGNAL_FUNC (new_contact_cb), 
		     (gpointer) window, TRUE),

      GTK_MENU_ENTRY("add", _("Add Contact to _Address Book"), NULL,
		     GTK_STOCK_ADD, 0,
		     GTK_SIGNAL_FUNC (properties_cb),
		     (gpointer) window, TRUE),

      GTK_MENU_END
    };

  gtk_build_menu (aw->aw_menu, addressbook_menu, accel, NULL);

  gtk_box_pack_start (GTK_BOX (main_vbox), aw->aw_menu, FALSE, FALSE, 0);


  /* A hpaned to put the tree and the LDAP browser */
  hpaned = gtk_hpaned_new ();
  gtk_container_set_border_width (GTK_CONTAINER (hpaned), 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), hpaned, TRUE, TRUE, 0);

  /* The GtkTreeView that will store the address books list */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_paned_add1 (GTK_PANED (hpaned), frame);
  model = gtk_tree_store_new (NUM_COLUMNS_CONTACTS, 
			      GDK_TYPE_PIXBUF, 
			      G_TYPE_STRING,
			      G_TYPE_STRING, 
			      G_TYPE_INT, 
			      G_TYPE_BOOLEAN,
			      G_TYPE_INT,
			      G_TYPE_STRING,
			      G_TYPE_STRING); 

  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (frame), scroll);

  aw->aw_tree_view = gtk_tree_view_new ();  
  gtk_tree_view_set_model (GTK_TREE_VIEW (aw->aw_tree_view), 
			   GTK_TREE_MODEL (model));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (aw->aw_tree_view));
  gtk_container_add (GTK_CONTAINER (scroll), aw->aw_tree_view);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (aw->aw_tree_view), FALSE);
  gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
			       GTK_SELECTION_BROWSE);
  gtk_tree_selection_set_select_function (selection, (GtkTreeSelectionFunc) 
					  aw_tree_selection_function_cb, 
					  NULL, NULL);


  /* Two renderers for one column */
  column = gtk_tree_view_column_new ();
  cell = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_attributes (column, cell, 
				       "pixbuf", COLUMN_PIXBUF, 
				       "visible", COLUMN_PIXBUF_VISIBLE, 
				       NULL);

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_attributes (column, cell, 
				       "text", COLUMN_NAME, 
				       "weight", COLUMN_WEIGHT,
				       NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (aw->aw_tree_view),
			       GTK_TREE_VIEW_COLUMN (column));


  /* We update the address books list with the top-level categories */
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter,
		      COLUMN_NAME, _("Remote Contacts"), 
		      COLUMN_NOTEBOOK_PAGE, -1, 
		      COLUMN_PIXBUF_VISIBLE, FALSE,
		      COLUMN_WEIGHT, PANGO_WEIGHT_BOLD, 
		      -1);
  gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model),
		      &iter,
		      COLUMN_NAME, _("Local Contacts"), 
		      COLUMN_NOTEBOOK_PAGE, -1, 
		      COLUMN_PIXBUF_VISIBLE, FALSE,
		      COLUMN_WEIGHT, PANGO_WEIGHT_BOLD, 
		      -1);


  /* The LDAP browser in the second part of the GtkHPaned */
  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_paned_add2 (GTK_PANED (hpaned), vbox2);  

  /* Each page of the GtkNotebook contains a list of the users */
  aw->aw_notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (aw->aw_notebook), 0);
  gtk_box_pack_start (GTK_BOX (vbox2), aw->aw_notebook, 
		      TRUE, TRUE, 0);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (aw->aw_notebook), FALSE);
  gmcontacts_dnd_set_dest (GTK_WIDGET (aw->aw_notebook),
			    dnd_add_contact_contactlist_cb, window);


  g_signal_connect (G_OBJECT (aw->aw_tree_view), "event_after",
		    G_CALLBACK (addressbook_clicked_cb), 
		    window);

  g_signal_connect (G_OBJECT (aw->aw_tree_view), "row_activated",
		    G_CALLBACK (search_addressbook2_cb), 
		    window);
  
  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK (addressbook_selected_cb), 
		    window);

  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (delete_window_cb), NULL);

 
  gmcontacts_dnd_set_dest_conditional (GTK_WIDGET (aw->aw_tree_view), 
					dnd_add_contact_server_cb,
					dnd_allow_drop_cb, window);

  /* Add the various address books */
  addressbooks = gnomemeeting_get_remote_addressbooks ();
  l = addressbooks;
  while (l) {

    gm_aw_add_addressbook (window, GM_ADDRESSBOOK (l->data));

    p++;
    l = g_slist_next (l);
  }
  g_slist_foreach (addressbooks, (GFunc) gm_addressbook_delete, NULL);
  g_slist_free (addressbooks);

  addressbooks = gnomemeeting_get_local_addressbooks ();
  l = addressbooks;
  while (l) {

    gm_aw_add_addressbook (window, GM_ADDRESSBOOK (l->data));

    p++;
    l = g_slist_next (l);
  }
  g_slist_foreach (addressbooks, (GFunc) gm_addressbook_delete, NULL);
  g_slist_free (addressbooks);


  gtk_widget_show_all (GTK_WIDGET (main_vbox));


  return window;
}


void
gm_addressbook_window_edit_contact_dialog_run (GtkWidget *addressbook_window,
					       GmAddressbook *addressbook,
					       GmContact *contact,
					       gboolean edit_existing_contact,
					       GtkWidget *parent_window)
{
  GmContact *new_contact = NULL;

  GtkWidget *dialog = NULL;

  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  
  GtkWidget *fullname_entry = NULL;
  GtkWidget *url_entry = NULL;
  GtkWidget *email_entry = NULL;
  GtkWidget *categories_entry = NULL;
  GtkWidget *speeddial_entry = NULL;
  GtkWidget *table = NULL;
  GtkWidget *label = NULL;

  GtkWidget *option_menu = NULL;

  GmAddressbook *addb = NULL;
  GmAddressbook *addc = NULL;
  GmAddressbook *new_addressbook = NULL;

  GSList *contacts = NULL;
  GSList *list = NULL;
  GSList *l = NULL;

  gchar *label_text = NULL;
  gint result = 0;
  gint current_menu_index = 0;
  gint pos = 0;
  int nbr = 0;

  gboolean collision = TRUE;
  gboolean valid = FALSE;

  
  main_window = GnomeMeeting::Process ()->GetMainWindow (); 
  chat_window = GnomeMeeting::Process ()->GetChatWindow (); 
  

  /* Create the dialog to easily modify the info 
   * of a specific contact */
  dialog =
    gtk_dialog_new_with_buttons (_("Edit the Contact Information"), 
				 GTK_WINDOW (parent_window),
				 GTK_DIALOG_MODAL,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
				 NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				   GTK_RESPONSE_ACCEPT);

  table = gtk_table_new (6, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);


  /* Get the list of addressbooks */
  list = gnomemeeting_get_local_addressbooks ();


  /* The Full Name entry */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Name:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  fullname_entry = gtk_entry_new ();
  if (contact && contact->fullname)
    gtk_entry_set_text (GTK_ENTRY (fullname_entry), contact->fullname);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), fullname_entry, 1, 2, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (fullname_entry), TRUE);

  /* The URL entry */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("VoIP URL:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  url_entry = gtk_entry_new ();
  if (contact && contact->url)
    gtk_entry_set_text (GTK_ENTRY (url_entry), contact->url);
  else
    gtk_entry_set_text (GTK_ENTRY (url_entry), GMURL ().GetDefaultURL ());
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), url_entry, 1, 2, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (url_entry), TRUE);

  /* The email entry */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Email:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  email_entry = gtk_entry_new ();
  if (contact && contact->email)
    gtk_entry_set_text (GTK_ENTRY (email_entry), contact->email);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), email_entry, 1, 2, 2, 3, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (email_entry), TRUE);


  /* The Speed Dial */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Speed Dial:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  speeddial_entry = gtk_entry_new ();
  if (contact && contact->speeddial)
    gtk_entry_set_text (GTK_ENTRY (speeddial_entry),
			contact->speeddial);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), speeddial_entry,
		    1, 2, 3, 4, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (speeddial_entry), TRUE);

  /* The Categories */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Categories:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  categories_entry = gtk_entry_new ();
  if (contact && contact->categories)
    gtk_entry_set_text (GTK_ENTRY (categories_entry),
			contact->categories);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), categories_entry,
		    1, 2, 4, 5, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (categories_entry), TRUE);

  /* The different local addressbooks are not displayed when
   * we are editing a contact from a local addressbook */
  if (!edit_existing_contact) {

    label = gtk_label_new (NULL);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    label_text = g_strdup_printf ("<b>%s</b>", _("Local Addressbook:"));
    gtk_label_set_markup (GTK_LABEL (label), label_text);
    g_free (label_text);

    option_menu = gtk_combo_box_new_text ();

    l = list;
    pos = 0;
    while (l) {

      addb = GM_ADDRESSBOOK (l->data);
      if (addressbook && addb 
	  && addb->name && addressbook->name 
	  && !strcmp (addb->name, addressbook->name))
	current_menu_index = pos;

      gtk_combo_box_append_text (GTK_COMBO_BOX (option_menu), addb->name);

      l = g_slist_next (l);
      pos++;
    }

    gtk_combo_box_set_active (GTK_COMBO_BOX (option_menu), current_menu_index);

    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6, 
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL),
		      3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
    gtk_table_attach (GTK_TABLE (table), option_menu,
		      1, 2, 5, 6, 
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL),
		      GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  }


  /* Pack the gtk entries and the list store in the window */
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table,
		      FALSE, FALSE, 3 * GNOMEMEETING_PAD_SMALL);
  gtk_widget_show_all (dialog);


  /* Now run the dialog */

  /* The result can be invalid if both a full name and url are missing,
   * or later if a collision is detected. The dialog will be run during
   * that time.
   */
  while (!valid)  { 

    result = gtk_dialog_run (GTK_DIALOG (dialog));

    valid = strcmp (gtk_entry_get_text (GTK_ENTRY (fullname_entry)), "") 
      || strcmp (gtk_entry_get_text (GTK_ENTRY (url_entry)), "");


    switch (result) {

      case GTK_RESPONSE_ACCEPT:

      if (valid) {

	new_contact = gmcontact_new ();
	new_contact->fullname = 
	  g_strdup (gtk_entry_get_text (GTK_ENTRY (fullname_entry)));
	new_contact->speeddial = 
	  g_strdup (gtk_entry_get_text (GTK_ENTRY (speeddial_entry)));
	new_contact->categories = 
	  g_strdup (gtk_entry_get_text (GTK_ENTRY (categories_entry)));
	new_contact->url = 
	  g_strdup (gtk_entry_get_text (GTK_ENTRY (url_entry)));
	new_contact->email =
	  g_strdup (gtk_entry_get_text (GTK_ENTRY (email_entry)));

	/* We were editing an existing contact */
	if (edit_existing_contact) {

	  /* We keep the old UID */
	  new_contact->uid = g_strdup (contact->uid);
	  new_addressbook = addressbook;
	}
	else {

	  /* Forget the selected addressbook and use the dialog one instead
	   * if the user could choose it in the dialog */
	  current_menu_index = gtk_combo_box_get_active (GTK_COMBO_BOX (option_menu));

          addc = GM_ADDRESSBOOK (g_slist_nth_data (list, current_menu_index)); 

	  if (addc) 
	    new_addressbook = addc;
	  else {

	    new_addressbook = gm_addressbook_new ();
	    new_addressbook->name = _("Personal");
	    (void)gnomemeeting_addressbook_add (new_addressbook);
	    gm_aw_add_addressbook (addressbook_window, new_addressbook);
	  }
	}

	/* We are editing an existing contact, compare with the old values */
	if (edit_existing_contact)
	  collision = gm_aw_check_contact_collision (addressbook_window,
						     new_contact, 
						     contact,
						     parent_window);
	else /* We are adding a new contact */ 
	  collision = gm_aw_check_contact_collision (addressbook_window,
						     new_contact, 
						     NULL,
						     parent_window);

	if (!collision) {

	  if (edit_existing_contact)
	    gnomemeeting_addressbook_modify_contact (new_addressbook, 
						     new_contact);
	  else 
	    gnomemeeting_addressbook_add_contact (new_addressbook, new_contact);



	  /* Update the address book with the content */
	  contacts = gnomemeeting_addressbook_get_contacts (new_addressbook,
							    nbr,
							    FALSE,
							    NULL,
							    NULL,
							    NULL,
							    NULL,
							    NULL);

	  gm_aw_update_addressbook (addressbook_window, 
				    new_addressbook, 
				    contacts,
				    NULL);

	  g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
	  g_slist_free (contacts);


	  /* Find speed dials and update the menu */
	  contacts = gnomemeeting_addressbook_get_contacts (NULL,
							    nbr,
							    FALSE,
							    NULL,
							    NULL,
							    NULL,
							    NULL,
							    "*");

	  gm_main_window_speed_dials_menu_update (main_window, contacts);

	  g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
	  g_slist_free (contacts);


	  /* Update the urls history */
	  gm_main_window_urls_history_update (main_window);
	  gm_text_chat_window_urls_history_update (chat_window);
	}

	gmcontact_delete (new_contact);
      }
      else {

	gnomemeeting_error_dialog (GTK_WINDOW (addressbook_window), _("Missing information"), _("Please make sure to provide at least a full name or an URL for the contact."));
      }

      break;

    case GTK_RESPONSE_DELETE_EVENT:
    case GTK_RESPONSE_CANCEL:

      collision = FALSE;
      valid = TRUE;
      break;
    }

    if (collision)
      valid = FALSE;
  }
  
  gtk_widget_destroy (dialog);
  
  g_slist_foreach (list, (GFunc) gm_addressbook_delete, NULL);
  g_slist_free (list);
}


void
gm_addressbook_window_delete_contact_dialog_run (GtkWidget *addressbook_window,
						 GmAddressbook *addressbook,
						 GmContact *contact,
						 GtkWidget *parent_window)
{
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;
  
  GtkWidget *dialog = NULL;

  GSList *contacts = NULL;

  gchar *confirm_msg = NULL;
  int nbr = 0;

  g_return_if_fail (addressbook != NULL);
  g_return_if_fail (contact != NULL);

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();


  confirm_msg = g_strdup_printf (_("Are you sure you want to delete %s from %s?"),
				 contact->fullname, addressbook->name);
  dialog =
    gtk_message_dialog_new (GTK_WINDOW (parent_window),
			    GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
			    GTK_BUTTONS_YES_NO, "%s", confirm_msg);
  g_free (confirm_msg);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				   GTK_RESPONSE_YES);

  gtk_widget_show_all (dialog);


  /* Now run the dialg */
  switch (gtk_dialog_run (GTK_DIALOG (dialog))) {

  case GTK_RESPONSE_YES:

    gnomemeeting_addressbook_delete_contact (addressbook, contact);
    
    contacts = gnomemeeting_addressbook_get_contacts (addressbook, 
						      nbr,
						      FALSE,
						      NULL,
						      NULL,
						      NULL,
						      NULL,
						      NULL);
    gm_aw_update_addressbook (addressbook_window, addressbook, contacts, NULL);
    g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
    g_slist_free (contacts);
    
    /* Find speed dials and update the menu */
    contacts = gnomemeeting_addressbook_get_contacts (NULL,
						      nbr,
						      FALSE,
						      NULL,
						      NULL,
						      NULL,
						      NULL,
						      "*");

    gm_main_window_speed_dials_menu_update (main_window, contacts);

    g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
    g_slist_free (contacts);
    
	
    /* Update the urls history */
    gm_main_window_urls_history_update (main_window);
    gm_text_chat_window_urls_history_update (chat_window);

    break;
  }

  gtk_widget_destroy (dialog);
}


void
gm_addressbook_window_edit_addressbook_dialog_run (GtkWidget *addressbook_window,
						   GmAddressbook *addb,
						   GtkWidget *parent_window)
{
  GmAddressbook *addc = NULL;

  GtkWidget *dialog = NULL;

  GtkWidget *base_entry = NULL;
  GtkWidget *hostname_entry = NULL;
  GtkWidget *addressbook_name_entry = NULL;
  GtkWidget *port_entry = NULL;
  GtkWidget *search_attribute_entry = NULL;
  GtkWidget *type_option_menu = NULL;
  GtkWidget *scope_option_menu = NULL;

  GtkWidget *table = NULL;
  GtkWidget *itable = NULL;
  GtkWidget *label = NULL;

  GtkSizeGroup *labels_group = NULL;
  GtkSizeGroup *options_group = NULL;

  const char *hostname = NULL;
  const char *port = NULL;
  const char *base = NULL;
  char *scope = NULL;
  char *prefix = NULL;

  PString entry;
  char default_hostname [256] = "";
  char default_port [256] = "";
  char default_base [256] = "";
  char default_scope [256] = "";
  char default_prefix [256] = "";
  int done = -1;
  int history = 0;

  gchar *label_text = NULL;
  int result = -1;
  gboolean valid = TRUE;

  g_return_if_fail (addressbook_window != NULL);


  /* Parse the URL if any and if remote addressbook */
  if (addb 
      && addb->url
      && !g_str_has_prefix (addb->url, "file:")) {

    entry = addb->url;
    entry.Replace (":", " ", TRUE);
    entry.Replace ("/", " ", TRUE);
    entry.Replace ("?", " ", TRUE);

    done = sscanf ((const char *) entry, 
		   "%255s %255s %255s %255s %255s", 
		   default_prefix, default_hostname, 
		   default_port, default_base, default_scope);
  }


  /* Create the dialog to create a new addressbook */
  dialog =
    gtk_dialog_new_with_buttons (addb ?
				 _("Edit an address book") 
				 :_("Add an address book"), 
				 GTK_WINDOW (parent_window),
				 GTK_DIALOG_MODAL,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
				 NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				   GTK_RESPONSE_ACCEPT);

  table = gtk_table_new (3, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3 * GNOMEMEETING_PAD_SMALL);
  labels_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
  options_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);


  /* The Server Name entry */
  label = gtk_label_new (NULL);
  gtk_size_group_add_widget (labels_group, label);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Name:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  addressbook_name_entry = gtk_entry_new ();
  gtk_size_group_add_widget (options_group, addressbook_name_entry);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), addressbook_name_entry, 1, 2, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (addressbook_name_entry), TRUE);
  if (addb && addb->name)
    gtk_entry_set_text (GTK_ENTRY (addressbook_name_entry), addb->name);


  /* Addressbook type if not edit */
  if (!addb) {
    
    label = gtk_label_new (NULL);
    gtk_size_group_add_widget (labels_group, label);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    label_text = g_strdup_printf ("<b>%s</b>", _("Type:"));
    gtk_label_set_markup (GTK_LABEL (label), label_text);
    g_free (label_text);

    type_option_menu = gtk_combo_box_new_text ();

    gtk_combo_box_append_text (GTK_COMBO_BOX (type_option_menu), _("Local"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (type_option_menu), _("Remote LDAP"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (type_option_menu), _("Remote ILS"));

    gtk_combo_box_set_active (GTK_COMBO_BOX (type_option_menu), 0);

    gtk_size_group_add_widget (options_group, type_option_menu);


    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, 
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL),
		      3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
    gtk_table_attach (GTK_TABLE (table), type_option_menu, 1, 2, 1, 2, 
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL),
		      GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  }


  /* Create another table */
  itable = gtk_table_new (5, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (itable), 3 * GNOMEMEETING_PAD_SMALL);
  gtk_table_set_col_spacings (GTK_TABLE (itable), 3 * GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (table), itable, 0, 2, 2, 3, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL), 0, 0);


  /* The Server Name entry */
  label = gtk_label_new (NULL);
  gtk_size_group_add_widget (labels_group, label);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Hostname:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  hostname_entry = gtk_entry_new ();
  gtk_size_group_add_widget (options_group, hostname_entry);
  gtk_table_attach (GTK_TABLE (itable), label, 0, 1, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (itable), hostname_entry, 1, 2, 0, 1, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (hostname_entry), TRUE);
  if (addb)
    gtk_entry_set_text (GTK_ENTRY (hostname_entry), default_hostname);


  /* The Server Port entry */
  label = gtk_label_new (NULL);
  gtk_size_group_add_widget (labels_group, label);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Port:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  port_entry = gtk_entry_new ();
  gtk_size_group_add_widget (options_group, port_entry);
  gtk_table_attach (GTK_TABLE (itable), label, 0, 1, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (itable), port_entry, 1, 2, 1, 2, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (port_entry), TRUE);
  if (addb)
    gtk_entry_set_text (GTK_ENTRY (port_entry), default_port);
  else
    gtk_entry_set_text (GTK_ENTRY (port_entry), "389");


  /* The Base entry */
  label = gtk_label_new (NULL);
  gtk_size_group_add_widget (labels_group, label);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Base DN:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  base_entry = gtk_entry_new ();
  gtk_size_group_add_widget (options_group, base_entry);
  gtk_table_attach (GTK_TABLE (itable), label, 0, 1, 2, 3, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (itable), base_entry, 1, 2, 2, 3, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (base_entry), TRUE);
  if (addb)
    gtk_entry_set_text (GTK_ENTRY (base_entry), default_base);

  
  /* Addressbook search scope */
  label = gtk_label_new (NULL);
  gtk_size_group_add_widget (labels_group, label);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Search Scope:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  scope_option_menu = gtk_combo_box_new_text ();

  gtk_combo_box_append_text (GTK_COMBO_BOX (scope_option_menu), _("Subtree"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (scope_option_menu), _("One Level"));

  gtk_combo_box_set_active (GTK_COMBO_BOX (scope_option_menu), 0);
  
  gtk_size_group_add_widget (options_group, scope_option_menu);
  
  if (addb) {

    if (!strcmp (default_scope, "one"))
      history = 1;

      gtk_combo_box_set_active (GTK_COMBO_BOX (scope_option_menu), history);

    history = 0;
  }

  gtk_table_attach (GTK_TABLE (itable), label, 0, 1, 3, 4, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (itable), scope_option_menu, 1, 2, 3, 4, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);


  /* The search attribute entry */
  label = gtk_label_new (NULL);
  gtk_size_group_add_widget (labels_group, label);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", _("Search Attribute:"));
  gtk_label_set_markup (GTK_LABEL (label), label_text);
  g_free (label_text);

  search_attribute_entry = gtk_entry_new ();
  gtk_size_group_add_widget (options_group, search_attribute_entry);
  gtk_table_attach (GTK_TABLE (itable), label, 0, 1, 4, 5, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    3 * GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_table_attach (GTK_TABLE (itable), search_attribute_entry, 1, 2, 4, 5, 
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    GNOMEMEETING_PAD_SMALL, GNOMEMEETING_PAD_SMALL);
  gtk_entry_set_activates_default (GTK_ENTRY (search_attribute_entry), TRUE);
  if (addb && addb->call_attribute)
    gtk_entry_set_text (GTK_ENTRY (search_attribute_entry), 
			addb->call_attribute);
  else
    gtk_entry_set_text (GTK_ENTRY (search_attribute_entry), "rfc822mailbox");


  /* Pack the gtk entries and the list store in the window */
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table,
		      FALSE, FALSE, 3 * GNOMEMEETING_PAD_SMALL);
  

  /* Only show what is needed and hide what is not */
  gtk_widget_show_all (table);
  if (type_option_menu) { /* We are not editing */
    
    g_signal_connect (G_OBJECT (type_option_menu), "changed",
		      GTK_SIGNAL_FUNC (edit_addressbook_type_menu_changed_cb),
		      (gpointer) itable);

  }
  if (!addb || (addb && gnomemeeting_addressbook_is_local (addb)))
    gtk_widget_hide_all (itable);
  
  gtk_widget_show (dialog);
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);


  /* Now run the dialg */
  do {

    result = gtk_dialog_run (GTK_DIALOG (dialog));

    switch (result) {

    case GTK_RESPONSE_ACCEPT:

      addc = gm_addressbook_new ();

      if (addb && addb->aid)
	addc->aid = g_strdup (addb->aid);
      addc->name = 
	g_strdup (gtk_entry_get_text (GTK_ENTRY (addressbook_name_entry)));
      addc->call_attribute =
	g_strdup (gtk_entry_get_text (GTK_ENTRY (search_attribute_entry)));

      if (!strcmp (addc->name, ""))
	valid = FALSE;
      else
	valid = TRUE;

      if ((addb && !gnomemeeting_addressbook_is_local (addb))
	  ||
          (!addb && gtk_combo_box_get_active (GTK_COMBO_BOX (type_option_menu)) != 0)) {

	hostname = gtk_entry_get_text (GTK_ENTRY (hostname_entry));
	port = gtk_entry_get_text (GTK_ENTRY (port_entry));
	base = gtk_entry_get_text (GTK_ENTRY (base_entry));
	if (gtk_combo_box_get_active (GTK_COMBO_BOX (scope_option_menu))==0)
	  scope = g_strdup ("sub");
	else
	  scope = g_strdup ("one");
	if (addb)
	  prefix = g_strdup (default_prefix);
	else {
	  
	  if (gtk_combo_box_get_active (GTK_COMBO_BOX (type_option_menu))==1)
	    prefix = g_strdup ("ldap");
	  else
	    prefix = g_strdup ("ils");
	}
	if (!valid || !strcmp (hostname, "") 
	    || !strcmp (port, "") || !strcmp (base, ""))
	  valid = FALSE;
	else
	  valid = TRUE;

	addc->url = g_strdup_printf ("%s://%s:%s/%s??%s", 
				     prefix, hostname, port, base, scope);
      }


      /* Do nothing if there is missing information */
      if (valid) {
	
	if (addb) {

	  if (gnomemeeting_addressbook_modify (addc))
	    gm_aw_modify_addressbook (addressbook_window, addc);
	}
	else {

	  if (gnomemeeting_addressbook_add (addc))
	    gm_aw_add_addressbook (addressbook_window, addc);
	}
      }
      else {
	
	gnomemeeting_error_dialog (GTK_WINDOW (addressbook_window), _("Missing information"), _("Please make sure you fill in all required fields."));
      }
      gm_addressbook_delete (addc);

      g_free (scope);
      g_free (prefix);

      break;

    case GTK_RESPONSE_DELETE_EVENT:
    case GTK_RESPONSE_CANCEL:

      valid = TRUE;

      break;      
    }
  } while (!valid);

  gtk_widget_destroy (dialog);
}


void 
gm_addressbook_window_delete_addressbook_dialog_run (GtkWidget *addressbook_window,
						     GmAddressbook *addressbook,
						     GtkWidget *parent_window)
{
  GtkWidget *dialog = NULL;
  GtkWidget *main_window = NULL;
  GtkWidget *chat_window = NULL;

  GSList *contacts = NULL;

  gchar *confirm_msg = NULL;
  int nbr = 0;

  g_return_if_fail (addressbook_window != NULL);
  g_return_if_fail (addressbook != NULL);


  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  chat_window = GnomeMeeting::Process ()->GetChatWindow ();
  
  
  /* Create the dialog to delete the addressbook */
  confirm_msg = 
    g_strdup_printf (_("Are you sure you want to delete %s and all its contacts?"), addressbook->name);
  dialog =
    gtk_message_dialog_new (GTK_WINDOW (addressbook_window),
			    GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
			    GTK_BUTTONS_YES_NO, "%s", confirm_msg);
  g_free (confirm_msg);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
				   GTK_RESPONSE_YES);

  gtk_widget_show_all (dialog);


  /* Now run the dialg */
  switch (gtk_dialog_run (GTK_DIALOG (dialog))) {

  case GTK_RESPONSE_YES:

    if (gnomemeeting_addressbook_delete (addressbook)) {
      
      gm_aw_delete_addressbook (addressbook_window, addressbook);

      /* Find speed dials and update the menu */
      contacts = gnomemeeting_addressbook_get_contacts (NULL,
							nbr,
							FALSE,
							NULL,
							NULL,
							NULL,
							NULL,
							"*");

      gm_main_window_speed_dials_menu_update (main_window, contacts);

      g_slist_foreach (contacts, (GFunc) gmcontact_delete, NULL);
      g_slist_free (contacts);


      /* Update the urls history */
      gm_main_window_urls_history_update (main_window);
      gm_text_chat_window_urls_history_update (chat_window);
    }
    break;
  }

  gtk_widget_destroy (dialog);
}