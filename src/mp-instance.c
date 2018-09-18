/*
 * Copyright © 2018 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include "mp-instance.h"

enum {
    PROP_NAME = 1,
    PROP_STATE
};

struct _MpInstance
{
    GObject parent_instance;

    gchar  *name;
    gchar  *state;
};

G_DEFINE_TYPE (MpInstance, mp_instance, G_TYPE_OBJECT)

static void
mp_instance_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    MpInstance *instance = MP_INSTANCE (object);

    switch (prop_id) {
    case PROP_NAME:
        g_clear_pointer (&instance->name, g_free);
        instance->name = g_value_dup_string (value);
        break;
    case PROP_STATE:
        g_clear_pointer (&instance->state, g_free);
        instance->state = g_value_dup_string (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
mp_instance_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    MpInstance *instance = MP_INSTANCE (object);

    switch (prop_id) {
    case PROP_NAME:
        g_value_set_string (value, instance->name);
        break;
    case PROP_STATE:
        g_value_set_string (value, instance->state);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
mp_instance_dispose (GObject *object)
{
    MpInstance *instance = MP_INSTANCE (object);

    g_clear_pointer (&instance->name, g_free);
    g_clear_pointer (&instance->state, g_free);

    G_OBJECT_CLASS (mp_instance_parent_class)->dispose (object);
}

void
mp_instance_class_init (MpInstanceClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = mp_instance_get_property;
    object_class->set_property = mp_instance_set_property;
    object_class->dispose = mp_instance_dispose;

    g_object_class_install_property (object_class,
                                     PROP_NAME,
                                     g_param_spec_string ("name",
                                                          "name",
                                                          "Name",
                                                          NULL,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    g_object_class_install_property (object_class,
                                     PROP_STATE,
                                     g_param_spec_string ("state",
                                                          "state",
                                                          "State",
                                                          NULL,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

void
mp_instance_init (MpInstance *instance)
{
}

const gchar *
mp_instance_get_name (MpInstance *instance)
{
    g_return_val_if_fail (MP_IS_INSTANCE (instance), NULL);
    return instance->name;
}

const gchar *
mp_instance_get_state (MpInstance *instance)
{
    g_return_val_if_fail (MP_IS_INSTANCE (instance), NULL);
    return instance->state;
}
