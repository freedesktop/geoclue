/* vim: set et ts=8 sw=8: */
/* gclue-service-client.h
 *
 * Copyright (C) 2013 Red Hat, Inc.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Zeeshan Ali (Khattak) <zeeshanak@gnome.org>
 */

#ifndef GCLUE_SERVICE_CLIENT_H
#define GCLUE_SERVICE_CLIENT_H

#include <glib-object.h>
#include "geoclue-interface.h"

G_BEGIN_DECLS

#define GClUE_TYPE_SERVICE_CLIENT            (gclue_service_client_get_type())
#define GCLUE_SERVICE_CLIENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GClUE_TYPE_SERVICE_CLIENT, GClueServiceClient))
#define GCLUE_SERVICE_CLIENT_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GClUE_TYPE_SERVICE_CLIENT, GClueServiceClient const))
#define GCLUE_SERVICE_CLIENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GClUE_TYPE_SERVICE_CLIENT, GClueServiceClientClass))
#define GClUE_IS_SERVICE_CLIENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GClUE_TYPE_SERVICE_CLIENT))
#define GClUE_IS_SERVICE_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GClUE_TYPE_SERVICE_CLIENT))
#define GCLUE_SERVICE_CLIENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GClUE_TYPE_SERVICE_CLIENT, GClueServiceClientClass))

typedef struct _GClueServiceClient        GClueServiceClient;
typedef struct _GClueServiceClientClass   GClueServiceClientClass;
typedef struct _GClueServiceClientPrivate GClueServiceClientPrivate;

struct _GClueServiceClient
{
        GClueClientSkeleton parent;

        /*< private >*/
        GClueServiceClientPrivate *priv;
};

struct _GClueServiceClientClass
{
        GClueClientSkeletonClass parent_class;
};

GType gclue_service_client_get_type (void) G_GNUC_CONST;

GClueServiceClient * gclue_service_client_new      (const char      *peer,
                                                    const char      *path,
                                                    GDBusConnection *connection,
                                                    GError         **error);
const char *         gclue_service_client_get_path (GClueServiceClient    *client);

G_END_DECLS

#endif /* GCLUE_SERVICE_CLIENT_H */