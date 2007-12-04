/*
 * Geoclue
 * geoclue-types.c - 
 *
 * Author: Iain Holmes <iain@openedhand.com>
 */

#include <geoclue/geoclue-marshal.h>
#include <geoclue/geoclue-types.h>
#include <geoclue/geoclue-accuracy.h>

void
geoclue_types_init (void)
{
	dbus_g_type_specialized_init ();
	dbus_g_object_register_marshaller (geoclue_marshal_VOID__INT_INT_DOUBLE_DOUBLE_DOUBLE_BOXED,
					   G_TYPE_NONE,
					   G_TYPE_INT,
					   G_TYPE_INT,
					   G_TYPE_DOUBLE,
					   G_TYPE_DOUBLE,
					   G_TYPE_DOUBLE,
					   dbus_g_type_get_collection ("GPtrArray", GEOCLUE_ACCURACY_TYPE),
					   G_TYPE_INVALID);
	dbus_g_object_register_marshaller (geoclue_marshal_VOID__INT_INT_DOUBLE_DOUBLE_DOUBLE,
					   G_TYPE_NONE,
					   G_TYPE_INT,
					   G_TYPE_INT,
					   G_TYPE_DOUBLE,
					   G_TYPE_DOUBLE,
					   G_TYPE_DOUBLE,
					   G_TYPE_INVALID);
	dbus_g_object_register_marshaller (geoclue_marshal_VOID__INT_BOXED_BOXED,
					   G_TYPE_NONE,
					   G_TYPE_INT,
					   dbus_g_type_get_map ("GHashTable",
								G_TYPE_STRING,
								G_TYPE_STRING),
					   dbus_g_type_get_collection ("GPtrArray", GEOCLUE_ACCURACY_TYPE),
					   G_TYPE_INVALID);
}
