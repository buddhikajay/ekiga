
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
 *                         gmcontacts-convert.h - description
 *                         ----------------------------------
 *   begin                : July 2004
 *   copyright            : (C) 2004 by Julien Puydt
 *   description          : Declaration of the routines to convert a GmContact
 *                          to another format. Only files in lib/contacts/
 *                          should use it.
 *
 */


#if !defined (_GM_CONTACTS_H_INSIDE__)
#error "Only <contacts/gmcontacts.h> can be included directly."
#endif

#include <glib.h>
#include "gmcontacts.h"

#ifndef _GM_CONTACTS_CONVERT_H_
#define _GM_CONTACTS_CONVERT_H_

G_BEGIN_DECLS

/* Description: this function takes a GmContact, and returns it as a vcard
 *              in the form of a gchar*
 * PRE: a non-NULL GmContact
 */
gchar *gmcontact_to_vcard (GmContact *);

/* Description: this function takes a vcard in the form of a gchar*, and
 *              returns it as a GmContact.
 * PRE: a non-NULL gchar*
 */
GmContact *vcard_to_gmcontact (const gchar *);

G_END_DECLS

#endif