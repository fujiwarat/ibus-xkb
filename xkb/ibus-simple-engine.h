#ifndef __IBUS_SIMPLE_ENGINE_H__
#define __IBUS_SIMPLE_ENGINE_H__

#include <ibus.h>

#define IBUS_TYPE_SIMPLE_ENGINE (ibus_simple_engine_get_type ())
#define IBUS_IS_SIMPLE_ENGINE(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_SIMPLE_ENGINE))
#define IBUS_SIMPLE_ENGINE_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_SIMPLE_ENGINE, IBusSimpleEngineClass))

GType   ibus_simple_engine_get_type    (void);

#endif // __IBUS_SIMPLE_ENGINE_H__
