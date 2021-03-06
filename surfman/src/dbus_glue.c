/*
 * Copyright (c) 2013 Citrix Systems, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "project.h"

dbus_bool_t
dbus_display_text (DBusMessage *msg, DBusMessage *reply)
{
  DBusError err;
  dbus_bool_t ret;
  const char *text;

  dbus_error_init (&err);
  ret = dbus_message_get_args (msg, &err,
                               DBUS_TYPE_STRING, &text,
                               DBUS_TYPE_INVALID);

  if (!ret)
    {
      surfman_error ("getting message args: %s", err.message);
      return FALSE;
    }

  if (splash_text (text))
    return FALSE;

  return TRUE;
}

dbus_bool_t
dbus_display_image (DBusMessage *msg, DBusMessage *reply)
{
  DBusError err;
  dbus_bool_t ret;
  const char *filename;

  dbus_error_init (&err);
  ret = dbus_message_get_args (msg, &err,
                               DBUS_TYPE_STRING, &filename,
                               DBUS_TYPE_INVALID);

  if (!ret)
    {
      surfman_error ("getting message args: %s", err.message);
      return FALSE;
    }

  if (splash_picture (filename))
    return FALSE;

  return TRUE;
}

dbus_bool_t
dbus_dump_all_screens (DBusMessage *msg, DBusMessage *reply)
{
  DBusError err;
  dbus_bool_t ret;
  const char *directoryname;

  dbus_error_init (&err);
  ret = dbus_message_get_args (msg, &err,
                               DBUS_TYPE_STRING, &directoryname,
                               DBUS_TYPE_INVALID);

  if (!ret)
    {
      surfman_error ("getting message args: %s", err.message);
      return FALSE;
    }

  if (dump_all_screens(directoryname))
    return FALSE;

  return TRUE;
}

dbus_bool_t
dbus_increase_brightness (DBusMessage *msg, DBusMessage *reply)
{
  plugin_increase_brightness ();

  return TRUE;
}

dbus_bool_t
dbus_decrease_brightness (DBusMessage *msg, DBusMessage *reply)
{
  plugin_decrease_brightness ();

  return TRUE;
}

dbus_bool_t
dbus_dpms_on (DBusMessage *msg, DBusMessage *reply)
{
  plugin_dpms_on ();

  //Re-display the most recently displayed domain.
  domain_set_visible(NULL, false);
  return TRUE;
}

dbus_bool_t
dbus_dpms_off (DBusMessage *msg, DBusMessage *reply)
{
  plugin_dpms_off ();

  return TRUE;
}

dbus_bool_t
dbus_pre_s3 (DBusMessage *msg, DBusMessage *reply)
{
  plugin_pre_s3 ();

  return TRUE;
}

dbus_bool_t
dbus_post_s3 (DBusMessage *msg, DBusMessage *reply)
{
  plugin_post_s3 ();

  return TRUE;
}

/* XXX: This one should go away */
dbus_bool_t
dbus_set_pv_display (DBusMessage *msg, DBusMessage *reply)
{
  DBusError err;
  dbus_bool_t ret;
  struct domain *d;
  struct device *dev;
  uint32_t domid;
  const char *be_type;

  dbus_error_init (&err);
  ret = dbus_message_get_args(msg, &err,
                              DBUS_TYPE_INT32, &domid,
                              DBUS_TYPE_STRING, &be_type,
                              DBUS_TYPE_INVALID);
  if (!ret)
    {
      surfman_error ("Error getting message args: %s", err.message);
      return FALSE;
    }

  d = domain_by_domid (domid);
  if (!d)
    d = domain_create (domid);

  dev = xenfb_device_create (d);
  ret = !!dev;

  if (reply && ret)
    dbus_message_append_args (reply,
                              DBUS_TYPE_INVALID);

  return ret;
}

dbus_bool_t
dbus_set_visible (DBusMessage *msg, DBusMessage *reply)
{
  DBusError err;
  dbus_bool_t ret;
  uint32_t domid; int domid_count;
  uint32_t timeout;
  dbus_bool_t force;
  struct domain *d;

  dbus_error_init (&err);
  ret = dbus_message_get_args(msg, &err,
                    DBUS_TYPE_INT32, &domid,
                    DBUS_TYPE_INT32, &timeout,
                    DBUS_TYPE_BOOLEAN, &force,
                    DBUS_TYPE_INVALID);
  if (!ret)
    {
      surfman_error ("Error getting message args: %s", err.message);
      return FALSE;
    }

  d = domain_by_domid (domid);
  if (!d)
    {
      surfman_error ("Domain %d not found", domid);
      return FALSE;
    }

  ret = !domain_set_visible (d, force);

  if (reply && ret)
    dbus_message_append_args (reply,
                              DBUS_TYPE_INVALID);

  return ret;
}

dbus_bool_t
dbus_vgpu_mode (DBusMessage *msg, DBusMessage *reply)
{
  return FALSE;
}

dbus_bool_t
dbus_get_visible (DBusMessage *msg, DBusMessage *reply)
{
  DBusError err;
  dbus_bool_t ret;
  int32_t *domids;
  uint32_t len = 1;

  domids = calloc (1, sizeof (int32_t));
  *domids = domain_get_visible ();

  surfman_info ("get_visible() = %d", *domids);

  if (*domids == -1)
    {
      surfman_error ("No visible domain");
      return FALSE;
    }

  if (reply)
    dbus_message_append_args (reply,
                              DBUS_TYPE_ARRAY, DBUS_TYPE_INT32, &domids, len,
                              DBUS_TYPE_INVALID);
  return TRUE;
}

dbus_bool_t
dbus_has_vgpu (DBusMessage *msg, DBusMessage *reply)
{
  DBusError err;
  dbus_bool_t ret;
  int32_t domid;
  struct domain *d;
  dbus_bool_t has_vgpu;
  
  dbus_error_init (&err);
  ret = dbus_message_get_args(msg, &err,
                    DBUS_TYPE_INT32, &domid,
                    DBUS_TYPE_INVALID);
  if (!ret)
    {
      surfman_error ("Error getting message args: %s", err.message);
      return FALSE;
    }

  surfman_info ("has_vgpu(%d)", domid);

  d = domain_by_domid (domid);
  if (!d)
    {
      surfman_error ("Domain %d not found", domid);
      return FALSE;
    }

  has_vgpu = domain_has_vgpu(d);

  if (reply)
    dbus_message_append_args (reply,
                              DBUS_TYPE_BOOLEAN, &has_vgpu,
                              DBUS_TYPE_INVALID);
  return TRUE;
}


dbus_bool_t
dbus_notify_death (DBusMessage *msg, DBusMessage *reply)
{
  DBusError err;
  dbus_bool_t ret;
  int32_t domid, sstate;
  struct domain *d;

  dbus_error_init (&err);
  ret = dbus_message_get_args(msg, &err,
                    DBUS_TYPE_INT32, &domid,
                    DBUS_TYPE_INT32, &sstate,
                    DBUS_TYPE_INVALID);
  if (!ret)
    {
      surfman_error ("Error getting message args: %s", err.message);
      return FALSE;
    }

  surfman_info ("notify_death(%d,%d)", domid, sstate);

#if 1
  surfman_warning ("THIS RPC IS DEPRECATED AND SHOULD NOT BE USED ANY MORE.");
#else
  d = domain_by_domid (domid);
  if (!d)
    {
      surfman_error ("Domain %d not found", domid);
      return FALSE;
    }

  domain_destroy (d);
#endif

  if (reply && ret)
    dbus_message_append_args (reply,
                              DBUS_TYPE_INVALID);

  return ret;
}

dbus_bool_t
dbus_notify_visible_domain_changed(int domid)
{
  extern DBusConnection *connection;
  dbus_bool_t r = FALSE;
  dbus_int32_t v_domid = domid;
  DBusMessage *msg = dbus_message_new_signal("/", "com.citrix.xenclient.surfman", "visible_domain_changed");
  dbus_message_append_args(msg, DBUS_TYPE_INT32, &v_domid, DBUS_TYPE_INVALID);
  r = dbus_connection_send(connection, msg, NULL);
  dbus_message_unref(msg);
  return r;
}
