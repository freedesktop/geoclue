/*
 * Geoclue
 * geoclue-position.c - Client API for accessing GcIfacePosition
 *
 * Author: Iain Holmes <iain@openedhand.com>
 */

/**
 * SECTION:geoclue-position
 * @short_description: Geoclue position client API
 *
 * #GeocluePosition contains position-related methods and signals. 
 * It is part of the Geoclue public client API which uses the D-Bus 
 * API to communicate with the actual provider.
 * 
 * After a #GeocluePosition is created with geoclue_position_new(), the 
 * geoclue_position_get_position() method and the PositionChanged signal
 * can be used to obtain the current position.
 */

#include <geoclue/geoclue-position.h>
#include <geoclue/geoclue-marshal.h>

#include "gc-iface-position-bindings.h"

typedef struct _GeocluePositionPrivate {
	int dummy;
} GeocluePositionPrivate;

enum {
	POSITION_CHANGED,
	LAST_SIGNAL
};

static guint32 signals[LAST_SIGNAL] = {0, };

#define GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GEOCLUE_TYPE_POSITION, GeocluePositionPrivate))

G_DEFINE_TYPE (GeocluePosition, geoclue_position, GEOCLUE_TYPE_PROVIDER);

static void
finalize (GObject *object)
{
	G_OBJECT_CLASS (geoclue_position_parent_class)->finalize (object);
}

static void
dispose (GObject *object)
{
	G_OBJECT_CLASS (geoclue_position_parent_class)->dispose (object);
}

static void
position_changed (DBusGProxy      *proxy,
		  int              fields,
		  int              timestamp,
		  double           latitude,
		  double           longitude,
		  double           altitude,
		  GPtrArray       *accuracy,
		  GeocluePosition *position)
{
	g_signal_emit (position, signals[POSITION_CHANGED], 0, fields,
		       timestamp, latitude, longitude, altitude, accuracy);
}

static GObject *
constructor (GType                  type,
	     guint                  n_props,
	     GObjectConstructParam *props)
{
	GObject *object;
	GeoclueProvider *provider;

	object = G_OBJECT_CLASS (geoclue_position_parent_class)->constructor
		(type, n_props, props);
	provider = GEOCLUE_PROVIDER (object);

	dbus_g_proxy_add_signal (provider->proxy, "PositionChanged",
				 G_TYPE_INT, G_TYPE_INT, G_TYPE_DOUBLE,
				 G_TYPE_DOUBLE, G_TYPE_DOUBLE, 
/* 				 dbus_g_type_get_collection ("GPtrArray", GEOCLUE_ACCURACY_TYPE), */
				 G_TYPE_POINTER,
				 G_TYPE_INVALID);
	dbus_g_proxy_connect_signal (provider->proxy, "PositionChanged",
				     G_CALLBACK (position_changed),
				     object, NULL);

	return object;
}

static void
geoclue_position_class_init (GeocluePositionClass *klass)
{
	GObjectClass *o_class = (GObjectClass *) klass;

	o_class->finalize = finalize;
	o_class->dispose = dispose;
	o_class->constructor = constructor;

	g_type_class_add_private (klass, sizeof (GeocluePositionPrivate));

	signals[POSITION_CHANGED] = g_signal_new ("position-changed",
						  G_TYPE_FROM_CLASS (klass),
						  G_SIGNAL_RUN_FIRST |
						  G_SIGNAL_NO_RECURSE,
						  G_STRUCT_OFFSET (GeocluePositionClass, position_changed), 
						  NULL, NULL,
						  geoclue_marshal_VOID__INT_INT_DOUBLE_DOUBLE_DOUBLE_POINTER,
						  G_TYPE_NONE, 6,
						  G_TYPE_INT, G_TYPE_INT,
						  G_TYPE_DOUBLE, G_TYPE_DOUBLE,
						  G_TYPE_DOUBLE, G_TYPE_POINTER);
}

static void
geoclue_position_init (GeocluePosition *position)
{
}

/**
 * geoclue_position_new:
 * @service: D-Bus service name
 * @path: D-Bus path name
 *
 * Creates a #GeocluePosition with given D-Bus service name and path.
 * 
 * Return value: Pointer to a new #GeocluePosition
 */
GeocluePosition *
geoclue_position_new (const char *service,
		      const char *path)
{
	return g_object_new (GEOCLUE_TYPE_POSITION,
			     "service", service,
			     "path", path,
			     "interface", GEOCLUE_POSITION_INTERFACE_NAME,
			     NULL);
}

/**
 * geoclue_position_get_position:
 * @position: A #GeocluePosition object
 * @timestamp: Pointer to returned Unix timestamp
 * @latitude: Pointer to returned latitude in degrees
 * @longitude: Pointer to returned longitude in degrees
 * @altitude: Pointer to returned altitude in meters
 * @accuracy: Pointer to returned #GeoclueAccuracy
 * @error: Pointer to returned #Gerror
 * 
 * Obtains the current position. @timestamp will contain the time of 
 * the actual position measurement. @accuracy is a rough estimate of the
 * accuracy of the current position.
 * 
 * If the caller is not interested in some values, the pointers can be 
 * left %NULL.
 * 
 * Return value: A #GeocluePositionFields bitfield representing the 
 * validity of the position values.
 */
GeocluePositionFields
geoclue_position_get_position (GeocluePosition  *position,
			       int              *timestamp,
			       double           *latitude,
			       double           *longitude,
			       double           *altitude,
			       GeoclueAccuracy **accuracy,
			       GError          **error)
{
	GeoclueProvider *provider = GEOCLUE_PROVIDER (position);
	double la, lo, al;
	int ts, fields;
	GeoclueAccuracy *acc;

	if (!org_freedesktop_Geoclue_Position_get_position (provider->proxy,
							    &fields, &ts,
							    &la, &lo, &al,
							    &acc, error)) {
		return GEOCLUE_POSITION_FIELDS_NONE;
	}

	if (timestamp != NULL) {
		*timestamp = ts;
	}

	if (latitude != NULL && (fields & GEOCLUE_POSITION_FIELDS_LATITUDE)) {
		*latitude = la;
	}

	if (longitude != NULL && (fields & GEOCLUE_POSITION_FIELDS_LONGITUDE)) {
		*longitude = lo;
	}

	if (altitude != NULL && (fields & GEOCLUE_POSITION_FIELDS_ALTITUDE)) {
		*altitude = al;
	}

	if (accuracy != NULL) {
		*accuracy = acc;
	}

	return fields;
}