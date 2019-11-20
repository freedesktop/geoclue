/* vim: set et ts=8 sw=8: */
/*
 * Geoclue convenience library.
 *
 * Copyright 2015 Red Hat, Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author: Zeeshan Ali (Khattak) <zeeshanak@gnome.org>
 */

/**
 * SECTION: gclue-simple
 * @title: GClueSimple
 * @short_description: Simplified convenience API
 *
 * #GClueSimple make it very simple to get latest location and monitoring
 * location updates. It takes care of the boring tasks of creating a
 * #GClueClientProxy instance, starting it, waiting till we have a location fix
 * and then creating a #GClueLocationProxy instance for it.
 *
 * Use #gclue_simple_new() or #gclue_simple_new_sync() to create a new
 * #GClueSimple instance. Once you have a #GClueSimple instance, you can get the
 * latest location using #gclue_simple_get_location() or reading the
 * #GClueSimple:location property. To monitor location updates, connect to
 * notify signal for this property.
 *
 * While most applications will find this API very useful, it is most
 * useful for applications that simply want to get the current location as
 * quickly as possible and do not care about accuracy (much).
 */

#include <glib/gi18n.h>

#include "gclue-simple.h"
#include "gclue-helpers.h"
#include "xdp-location.h"

#define BUS_NAME "org.freedesktop.GeoClue2"

static void
gclue_simple_async_initable_init (GAsyncInitableIface *iface);

struct _GClueSimplePrivate
{
        char *desktop_id;
        GClueAccuracyLevel accuracy_level;

        GClueClient *client;
        GClueLocation *location;

        gulong update_id;

        GTask *task;
        GCancellable *cancellable;

        char *sender;
        XdpLocation *portal;
        gulong location_updated_id;
        guint response_id;
        char *session_id;
};

G_DEFINE_TYPE_WITH_CODE (GClueSimple,
                         gclue_simple,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_ASYNC_INITABLE,
                                                gclue_simple_async_initable_init)
                         G_ADD_PRIVATE (GClueSimple));

enum
{
        PROP_0,
        PROP_DESKTOP_ID,
        PROP_ACCURACY_LEVEL,
        PROP_CLIENT,
        PROP_LOCATION,
        LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

static void clear_portal (GClueSimple *simple);

static void
gclue_simple_finalize (GObject *object)
{
        GClueSimplePrivate *priv = GCLUE_SIMPLE (object)->priv;

        g_clear_pointer (&priv->desktop_id, g_free);
        if (priv->update_id != 0) {
                g_signal_handler_disconnect (priv->client, priv->update_id);
                priv->update_id = 0;
        }
        if (priv->cancellable != NULL)
                g_cancellable_cancel (priv->cancellable);
        g_clear_object (&priv->cancellable);
        g_clear_object (&priv->client);
        g_clear_object (&priv->location);
        g_clear_object (&priv->task);

        clear_portal (GCLUE_SIMPLE (object));

        /* Chain up to the parent class */
        G_OBJECT_CLASS (gclue_simple_parent_class)->finalize (object);
}

static void
gclue_simple_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
        GClueSimple *simple = GCLUE_SIMPLE (object);

        switch (prop_id) {
        case PROP_CLIENT:
                g_value_set_object (value, simple->priv->client);
                break;

        case PROP_LOCATION:
                g_value_set_object (value, simple->priv->location);
                break;

        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
}

static void
gclue_simple_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
        GClueSimple *simple = GCLUE_SIMPLE (object);

        switch (prop_id) {
        case PROP_DESKTOP_ID:
                simple->priv->desktop_id = g_value_dup_string (value);
                break;

        case PROP_ACCURACY_LEVEL:
                simple->priv->accuracy_level = g_value_get_enum (value);
                break;

        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
}

static void
gclue_simple_class_init (GClueSimpleClass *klass)
{
        GObjectClass *object_class;

        object_class = G_OBJECT_CLASS (klass);
        object_class->finalize = gclue_simple_finalize;
        object_class->get_property = gclue_simple_get_property;
        object_class->set_property = gclue_simple_set_property;

        /**
         * GClueSimple:desktop-id:
         *
         * The Desktop ID of the application.
         */
        gParamSpecs[PROP_DESKTOP_ID] = g_param_spec_string ("desktop-id",
                                                            "DesktopID",
                                                            "Desktop ID",
                                                            NULL,
                                                            G_PARAM_WRITABLE |
                                                            G_PARAM_CONSTRUCT_ONLY);
        g_object_class_install_property (object_class,
                                         PROP_DESKTOP_ID,
                                         gParamSpecs[PROP_DESKTOP_ID]);

        /**
         * GClueSimple:accuracy-level:
         *
         * The requested maximum accuracy level.
         */
        gParamSpecs[PROP_ACCURACY_LEVEL] = g_param_spec_enum ("accuracy-level",
                                                              "AccuracyLevel",
                                                              "Requested accuracy level",
                                                              GCLUE_TYPE_ACCURACY_LEVEL,
                                                              GCLUE_ACCURACY_LEVEL_NONE,
                                                              G_PARAM_WRITABLE |
                                                              G_PARAM_CONSTRUCT_ONLY);
        g_object_class_install_property (object_class,
                                         PROP_ACCURACY_LEVEL,
                                         gParamSpecs[PROP_ACCURACY_LEVEL]);

        /**
         * GClueSimple:client:
         *
         * The client proxy.
         */
        gParamSpecs[PROP_CLIENT] = g_param_spec_object ("client",
                                                        "Client",
                                                        "Client proxy",
                                                         GCLUE_TYPE_CLIENT_PROXY,
                                                         G_PARAM_READABLE);
        g_object_class_install_property (object_class,
                                         PROP_CLIENT,
                                         gParamSpecs[PROP_CLIENT]);

        /**
         * GClueSimple:location:
         *
         * The current location.
         */
        gParamSpecs[PROP_LOCATION] = g_param_spec_object ("location",
                                                          "Location",
                                                          "Location proxy",
                                                          GCLUE_TYPE_LOCATION_PROXY,
                                                          G_PARAM_READABLE);
        g_object_class_install_property (object_class,
                                         PROP_LOCATION,
                                         gParamSpecs[PROP_LOCATION]);
}

static void
on_location_proxy_ready (GObject      *source_object,
                         GAsyncResult *res,
                         gpointer      user_data)
{
        GClueSimplePrivate *priv = GCLUE_SIMPLE (user_data)->priv;
        GClueLocation *location;
        GError *error = NULL;

        location = gclue_location_proxy_new_for_bus_finish (res, &error);
        if (error != NULL) {
                if (priv->task != NULL) {
                        g_task_return_error (priv->task, error);
                        g_clear_object (&priv->task);
                } else {
                        g_warning ("Failed to create location proxy: %s",
                                   error->message);
                }

                return;
        }
        g_clear_object (&priv->location);
        priv->location = location;

        if (priv->task != NULL) {
                g_task_return_boolean (priv->task, TRUE);
                g_clear_object (&priv->task);
        } else {
                g_object_notify (G_OBJECT (user_data), "location");
        }
}

static void
on_location_updated (GClueClient *client,
                     const char  *old_location,
                     const char  *new_location,
                     gpointer     user_data)
{
        GClueSimplePrivate *priv = GCLUE_SIMPLE (user_data)->priv;

        if (new_location == NULL || g_strcmp0 (new_location, "/") == 0)
                return;

        gclue_location_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
                                          G_DBUS_PROXY_FLAGS_NONE,
                                          BUS_NAME,
                                          new_location,
                                          priv->cancellable,
                                          on_location_proxy_ready,
                                          user_data);
}

static void
on_client_started (GObject      *source_object,
                   GAsyncResult *res,
                   gpointer      user_data)
{
        GTask *task = G_TASK (user_data);
        GClueClient *client = GCLUE_CLIENT (source_object);
        GClueSimple *simple;
        const char *location;
        GError *error = NULL;

        simple = g_task_get_source_object (task);

        gclue_client_call_start_finish (client, res, &error);
        if (error != NULL) {
                g_task_return_error (task, error);
                g_clear_object (&simple->priv->task);

                return;
        }

        location = gclue_client_get_location (client);

        on_location_updated (client, NULL, location, simple);
}

static void
on_client_created (GObject      *source_object,
                   GAsyncResult *res,
                   gpointer      user_data)
{
        GTask *task = G_TASK (user_data);
        GClueSimple *simple = g_task_get_source_object (task);
        GClueSimplePrivate *priv = simple->priv;
        GError *error = NULL;

        priv->client = gclue_client_proxy_create_full_finish (res, &error);
        if (error != NULL) {
                g_task_return_error (task, error);
                g_clear_object (&priv->task);

                return;
        }

        priv->task = task;
        priv->update_id =
                g_signal_connect (priv->client,
                                  "location-updated",
                                  G_CALLBACK (on_location_updated),
                                  simple);

        gclue_client_call_start (priv->client,
                                 g_task_get_cancellable (task),
                                 on_client_started,
                                 task);
}

/* We use the portal if we are inside a flatpak,
 * or if GTK_USE_PORTAL is set in the environment.
 */
static gboolean
should_use_portal (void)
{
  static const char *use_portal = NULL;

  if (G_UNLIKELY (use_portal == NULL))
    {
      if (g_file_test ("/.flatpak-info", G_FILE_TEST_EXISTS))
        use_portal = "1";
      else
        {
          use_portal = g_getenv ("GTK_USE_PORTAL");
          if (!use_portal)
            use_portal = "";
        }
    }

  return use_portal[0] == '1';
}

#define PORTAL_BUS_NAME "org.freedesktop.portal.Desktop"
#define PORTAL_OBJECT_PATH "/org/freedesktop/portal/desktop"
#define PORTAL_REQUEST_INTERFACE "org.freedesktop.portal.Request"
#define PORTAL_SESSION_INTERFACE "org.freedesktop.portal.Session"

static void
clear_portal (GClueSimple *simple)
{
        GClueSimplePrivate *priv = simple->priv;

        if (priv->portal) {
                GDBusConnection *bus = g_dbus_proxy_get_connection (G_DBUS_PROXY (priv->portal));

                if (priv->session_id)
                        g_dbus_connection_call (bus,
                                                PORTAL_BUS_NAME,
                                                priv->session_id,
                                                PORTAL_SESSION_INTERFACE,
                                                "Close",
                                                NULL, NULL, 0, -1, NULL, NULL, NULL);

                if (priv->response_id) {
                        g_dbus_connection_signal_unsubscribe (bus, priv->response_id);
                        priv->response_id = 0;
                }
        }

        g_clear_object (&priv->portal);
        priv->location_updated_id = 0;
        g_clear_pointer (&priv->session_id, g_free);
        g_clear_pointer (&priv->sender, g_free);
}

static void
on_portal_location_updated (XdpLocation *portal,
                            const char *session_handle,
                            GVariant *data,
                            gpointer user_data)
{
        GClueSimple *simple = user_data;
        GClueSimplePrivate *priv = simple->priv;
        double latitude;
        double longitude;
        double altitude;
        double accuracy;
        double speed;
        double heading;
        const char *description;
        GVariant *timestamp;
        GClueLocation *location = gclue_location_skeleton_new ();

        g_variant_lookup (data, "Latitude", "d", &latitude);
        g_variant_lookup (data, "Longitude", "d", &longitude);
        g_variant_lookup (data, "Altitude", "d", &altitude);
        g_variant_lookup (data, "Accuracy", "d", &accuracy);
        g_variant_lookup (data, "Speed", "d", &speed);
        g_variant_lookup (data, "Heading", "d", &heading);
        g_variant_lookup (data, "Description", "&s", &description);
        g_variant_lookup (data, "Timestamp", "@(tt)", &timestamp);

        gclue_location_set_latitude (location, latitude);
        gclue_location_set_longitude (location, longitude);
        gclue_location_set_altitude (location, altitude);
        gclue_location_set_accuracy (location, accuracy);
        gclue_location_set_speed (location, speed);
        gclue_location_set_heading (location, heading);
        gclue_location_set_description (location, description);
        gclue_location_set_timestamp (location, timestamp);

        g_set_object (&priv->location, location);

        if (priv->task) {
                g_task_return_boolean (priv->task, TRUE);
                g_clear_object (&priv->task);
        }
        else {
                g_object_notify (G_OBJECT (simple), "location");
        }

        g_object_unref (location);
}

static void
on_started (GDBusConnection *bus,
            const char *sender_name,
            const char *object_path,
            const char *interface_name,
            const char *signal_name,
            GVariant *parameters,
            gpointer user_data)
{
        GClueSimple *simple = user_data;
        GClueSimplePrivate *priv = simple->priv;
        guint32 response;
        g_autoptr(GVariant) ret = NULL;

        g_dbus_connection_signal_unsubscribe (bus, priv->response_id);
        priv->response_id = 0;

        g_variant_get (parameters, "(u@a{sv})", &response, &ret);

        if (response != 0) {
                clear_portal (simple);
                g_task_return_new_error (priv->task, G_IO_ERROR, G_IO_ERROR_FAILED, "Start failed");
                g_clear_object (&priv->task);
        }
}

static void
on_session_created (GObject *source,
                    GAsyncResult *result,
                    gpointer user_data)
{
        GTask *task = G_TASK (user_data);
        GClueSimple *simple = g_task_get_source_object (task);
        GClueSimplePrivate *priv = simple->priv;
        GDBusConnection *bus = g_dbus_proxy_get_connection (G_DBUS_PROXY (priv->portal));
        GError *error = NULL;
        g_autofree char *handle = NULL;
        g_autofree char *token = NULL;
        g_autofree char *request_path = NULL;
        GVariantBuilder options;

        if (!xdp_location_call_create_session_finish (priv->portal, &handle, result, &error)) {
                clear_portal (simple);
                g_task_return_error (task, error);
                g_object_unref (task);
                return;
        }

        if (!g_str_equal (handle, priv->session_id)) {
                clear_portal (simple);
                g_task_return_new_error (task, G_IO_ERROR, G_IO_ERROR_FAILED, "Unexpected session id");
                g_object_unref (task);
                return;
        }

        priv->task = task;

        token = g_strdup_printf ("geoclue%d", g_random_int_range (0, G_MAXINT));
        request_path = g_strconcat (PORTAL_OBJECT_PATH, "/request/", priv->sender, "/", token, NULL);
        priv->response_id = g_dbus_connection_signal_subscribe (bus,
                                                                PORTAL_BUS_NAME,
                                                                PORTAL_REQUEST_INTERFACE,
                                                                "Response",
                                                                request_path,
                                                                NULL,
                                                                G_DBUS_SIGNAL_FLAGS_NO_MATCH_RULE,
                                                                on_started,
                                                                simple,
                                                                NULL);

        g_variant_builder_init (&options, G_VARIANT_TYPE_VARDICT);
        g_variant_builder_add (&options, "{sv}", "handle_token", g_variant_new_string (token));
        xdp_location_call_start (priv->portal,
                                 priv->session_id,
                                 "", /* FIXME parent window */
                                 g_variant_builder_end (&options),
                                 NULL, NULL, NULL);
}

static int
accuracy_level_to_portal (GClueAccuracyLevel level)
{
        switch (level) {
        case GCLUE_ACCURACY_LEVEL_NONE:
                return 0;
        case GCLUE_ACCURACY_LEVEL_COUNTRY:
                return 1;
        case GCLUE_ACCURACY_LEVEL_CITY:
                return 2;
        case GCLUE_ACCURACY_LEVEL_NEIGHBORHOOD:
                return 3;
        case GCLUE_ACCURACY_LEVEL_STREET:
                return 4;
        case GCLUE_ACCURACY_LEVEL_EXACT:
                return 5;
        default:
                g_assert_not_reached ();
        }

        return 0;
}

static void
on_portal_created (GObject      *source_object,
                   GAsyncResult *res,
                   gpointer      user_data)
{
        GTask *task = G_TASK (user_data);
        GClueSimple *simple = g_task_get_source_object (task);
        GClueSimplePrivate *priv = simple->priv;
        GDBusConnection *bus;
        GError *error = NULL;
        int i;
        g_autofree char *session_token = NULL;
        GVariantBuilder options;

        priv->portal = xdp_location_proxy_new_for_bus_finish (res, &error);

        if (error != NULL) {
                g_task_return_error (task, error);
                g_object_unref (task);
                return;
        }

        bus = g_dbus_proxy_get_connection (G_DBUS_PROXY (priv->portal));

        priv->location_updated_id = g_signal_connect (priv->portal,
                                                      "location-updated",
                                                      G_CALLBACK (on_portal_location_updated),
                                                      simple);

        priv->sender = g_strdup (g_dbus_connection_get_unique_name (bus) + 1);
        for (i = 0; priv->sender[i]; i++)
                if (priv->sender[i] == '.')
                        priv->sender[i] = '_';

        session_token = g_strdup_printf ("geoclue%d", g_random_int_range (0, G_MAXINT));
        priv->session_id = g_strconcat (PORTAL_OBJECT_PATH, "/session/", priv->sender, "/",
                                        session_token, NULL);

        g_variant_builder_init (&options, G_VARIANT_TYPE_VARDICT);
        g_variant_builder_add (&options, "{sv}",
                               "session_handle_token", g_variant_new_string (session_token));
        g_variant_builder_add (&options, "{sv}", "distance-threshold", g_variant_new_uint32 (0));
        g_variant_builder_add (&options, "{sv}", "time-threshold", g_variant_new_uint32 (0));
        g_variant_builder_add (&options, "{sv}", "accuracy", g_variant_new_uint32 (accuracy_level_to_portal (simple->priv->accuracy_level)));

        xdp_location_call_create_session (priv->portal,
                                          g_variant_builder_end (&options),
                                          NULL, on_session_created, task);
}

static void
gclue_simple_init_async (GAsyncInitable     *initable,
                         int                 io_priority,
                         GCancellable       *cancellable,
                         GAsyncReadyCallback callback,
                         gpointer            user_data)
{
        GTask *task;
        GClueSimple *simple = GCLUE_SIMPLE (initable);

        task = g_task_new (initable, cancellable, callback, user_data);

        if (should_use_portal ()) {
                xdp_location_proxy_new_for_bus (G_BUS_TYPE_SESSION,
                                                G_DBUS_PROXY_FLAGS_NONE,
                                                PORTAL_BUS_NAME,
                                                PORTAL_OBJECT_PATH,
                                                cancellable,
                                                on_portal_created,
                                                task);
        }
        else {
                gclue_client_proxy_create_full (simple->priv->desktop_id,
                                                simple->priv->accuracy_level,
                                                GCLUE_CLIENT_PROXY_CREATE_AUTO_DELETE,
                                                cancellable,
                                                on_client_created,
                                                task);
        }
}

static gboolean
gclue_simple_init_finish (GAsyncInitable *initable,
                          GAsyncResult   *result,
                          GError        **error)
{
        return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gclue_simple_async_initable_init (GAsyncInitableIface *iface)
{
        iface->init_async = gclue_simple_init_async;
        iface->init_finish = gclue_simple_init_finish;
}

static void
gclue_simple_init (GClueSimple *simple)
{
        simple->priv = G_TYPE_INSTANCE_GET_PRIVATE (simple,
                                                  GCLUE_TYPE_SIMPLE,
                                                  GClueSimplePrivate);
        simple->priv->cancellable = g_cancellable_new ();
}

/**
 * gclue_simple_new:
 * @desktop_id: The desktop file id (the basename of the desktop file).
 * @accuracy_level: The requested accuracy level as #GClueAccuracyLevel.
 * @cancellable: (allow-none): A #GCancellable or %NULL.
 * @callback: A #GAsyncReadyCallback to call when the results are ready.
 * @user_data: User data to pass to @callback.
 *
 * Asynchronously creates a #GClueSimple instance. Use
 * #gclue_simple_new_finish() to get the created #GClueSimple instance.
 *
 * See #gclue_simple_new_sync() for the synchronous, blocking version
 * of this function.
 */
void
gclue_simple_new (const char         *desktop_id,
                  GClueAccuracyLevel  accuracy_level,
                  GCancellable       *cancellable,
                  GAsyncReadyCallback callback,
                  gpointer            user_data)
{
        g_async_initable_new_async (GCLUE_TYPE_SIMPLE,
                                    G_PRIORITY_DEFAULT,
                                    cancellable,
                                    callback,
                                    user_data,
                                    "desktop-id", desktop_id,
                                    "accuracy-level", accuracy_level,
                                    NULL);
}

/**
 * gclue_simple_new_finish:
 * @result: The #GAsyncResult obtained from the #GAsyncReadyCallback passed to
 *          #gclue_simple_new().
 * @error: Return location for error or %NULL.
 *
 * Finishes an operation started with #gclue_simple_new().
 *
 * Returns: (transfer full) (type GClueSimple): The constructed proxy
 * object or %NULL if @error is set.
 */
GClueSimple *
gclue_simple_new_finish (GAsyncResult *result,
                         GError      **error)
{
        GObject *object;
        GObject *source_object;

        source_object = g_async_result_get_source_object (result);
        object = g_async_initable_new_finish (G_ASYNC_INITABLE (source_object),
                                              result,
                                              error);
        g_object_unref (source_object);
        if (object != NULL)
                return GCLUE_SIMPLE (object);
        else
                return NULL;
}

static void
on_simple_ready (GObject      *source_object,
                 GAsyncResult *res,
                 gpointer      user_data)
{
        GClueSimple *simple;
        GTask *task = G_TASK (user_data);
        GMainLoop *main_loop;
        GError *error = NULL;

        simple = gclue_simple_new_finish (res, &error);
        if (error != NULL) {
                g_task_return_error (task, error);

                goto out;
        }

        g_task_return_pointer (task, simple, g_object_unref);

out:
        main_loop = g_task_get_task_data (task);
        g_main_loop_quit (main_loop);
}

/**
 * gclue_simple_new_sync:
 * @desktop_id: The desktop file id (the basename of the desktop file).
 * @accuracy_level: The requested accuracy level as #GClueAccuracyLevel.
 * @cancellable: (allow-none): A #GCancellable or %NULL.
 * @error: Return location for error or %NULL.
 *
 * The synchronous and blocking version of #gclue_simple_new().
 *
 * Returns: (transfer full) (type GClueSimple): The new #GClueSimple object or
 * %NULL if @error is set.
 */
GClueSimple *
gclue_simple_new_sync (const char        *desktop_id,
                       GClueAccuracyLevel accuracy_level,
                       GCancellable      *cancellable,
                       GError           **error)
{
        GClueSimple *simple;
        GMainLoop *main_loop;
        GTask *task;

        task = g_task_new (NULL, cancellable, NULL, NULL);
        main_loop = g_main_loop_new (NULL, FALSE);
        g_task_set_task_data (task,
                              main_loop,
                              (GDestroyNotify) g_main_loop_unref);

        gclue_simple_new (desktop_id,
                          accuracy_level,
                          cancellable,
                          on_simple_ready,
                          task);

        g_main_loop_run (main_loop);

        simple = g_task_propagate_pointer (task, error);

        g_object_unref (task);

        return simple;
}

/**
 * gclue_simple_get_client:
 * @simple: A #GClueSimple object.
 *
 * Gets the client proxy, or %NULL if @simple is not using a client proxy.
 *
 * Returns: (transfer none) (type GClueClientProxy): The client object.
 */
GClueClient *
gclue_simple_get_client (GClueSimple *simple)
{
        g_return_val_if_fail (GCLUE_IS_SIMPLE(simple), NULL);

        return simple->priv->client;
}

/**
 * gclue_simple_get_location:
 * @simple: A #GClueSimple object.
 *
 * Gets the current location.
 *
 * Returns: (transfer none) (type GClueLocation): The last known location
 * as #GClueLocation.
 */
GClueLocation *
gclue_simple_get_location (GClueSimple *simple)
{
        g_return_val_if_fail (GCLUE_IS_SIMPLE(simple), NULL);

        return simple->priv->location;
}
