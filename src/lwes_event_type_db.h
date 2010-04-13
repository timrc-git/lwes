/*======================================================================*
 * Copyright (C) 2008 Light Weight Event System                         *
 * All rights reserved.                                                 *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, write to the Free Software          *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                   *
 * Boston, MA 02110-1301 USA.                                           *
 *======================================================================*/
#ifndef __LWES_EVENT_TYPE_DB
#define __LWES_EVENT_TYPE_DB

#include "lwes_types.h"

#include <stdio.h>   /* for FILENAME_MAX */
#include <stdlib.h>  /* for malloc */
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif 

/*! \file lwes_event_type_db.h
 *  \brief Functions for an event specification file database
 *
 *  This is used in cases where you want to restrict the events allowed
 *  to be emitted or listened to to a particular esf file
 */

/*! \struct lwes_event_type_db_attribute lwes_event_type_db.h
 *  \brief The attributes stored in the database
 */
struct lwes_event_type_db_attribute
{
  /*! An event */
  struct lwes_event *event;
};

/*! \struct lwes_event_type_db lwes_event_type_db.h
 *  \brief The data base itself
 */
struct lwes_event_type_db
{
  /*! holds the file name which describes the database (aka, the esf file) */
  char esf_filename[FILENAME_MAX];
  /*! holds a hash of event descriptions by the event name
      for events which are described in the esf file */
  struct lwes_hash *events;
};

/*! \brief Creates the memory for the event_type_db.
 *  
 *  This creates memory which should be freed with lwes_event_type_db_destroy
 *
 *  \param[in] filename the path to the file containing the esf description
 *
 *  \see lwes_event_type_db_destroy
 *
 *  \return the newly created db on success, NULL on failure
 */
struct lwes_event_type_db *
lwes_event_type_db_create
  (const char *filename);

/*! \brief Cleanup the memory for the event_type_db.
 *
 *  This frees the memory created by lwes_event_type_db_create.
 *
 *  \param[in] db the db to free
 *
 *  \return 0 on success, a negative number on failure
 */
int
lwes_event_type_db_destroy
  (struct lwes_event_type_db *db);

/*! \brief Add an an event name to the database
 *
 * \param[in] db the db to add the event name to
 * \param[in] event_name the name of an event
 *
 * \return 0 if the add is successful, a negative number on failure
 */
int
lwes_event_type_db_add_event
  (struct lwes_event_type_db *db,
   LWES_SHORT_STRING event_name);

/*! \brief Add an attribute name to an event name of the database
 *
 * \param[in] db the db to add the attr_name into
 * \param[in] event_name the name of an event
 * \param[in] attr_name the name of an attribute
 * \param[in] type the type of the attribute
 *
 * \return 0 if the add is successful, a negative number on failure
 */
int
lwes_event_type_db_add_attribute
  (struct lwes_event_type_db *db,
   LWES_SHORT_STRING event_name,
   LWES_SHORT_STRING attr_name,
   LWES_TYPE type);

/*! \brief Check for an event in the database
 *
 *  \param[in] db the db to check for the event in
 *  \param[in] event_name the name of an event
 *
 *  \return 1 if the event is in the db, 0 if it is not
 */
int
lwes_event_type_db_check_for_event
  (struct lwes_event_type_db *db,
   LWES_SHORT_STRING event_name);

/*! \brief Check for an attribute in an event in the database
 *
 *  \param[in] db the db to check
 *  \param[in] attr_name the attribute name to check for
 *  \param[in] event_name the event name to check in
 *
 *  \return 1 if the attribute is in the event in the db, 0 if it is not
 */
int
lwes_event_type_db_check_for_attribute
  (struct lwes_event_type_db *db,
   LWES_CONST_SHORT_STRING attr_name,
   LWES_CONST_SHORT_STRING event_name);

/*! \brief Check the type of an attribute in an event in the db
 *
 *  \param[in] db the db to check
 *  \param[in] type_value the type to check against
 *  \param[in] attr_name the attribute name to check for
 *  \param[in] event_name the event name to check in
 *
 *  \return 1 if the attribute in the event in the db is of the asked for type,
 *          0 if it is not of the specified type
 */
int
lwes_event_type_db_check_for_type
  (struct lwes_event_type_db *db,
   LWES_TYPE type_value,
   LWES_CONST_SHORT_STRING attr_name,
   LWES_CONST_SHORT_STRING event_name);

#ifdef __cplusplus
}
#endif 

#endif /* __LWES_EVENT_TYPE_DB */
