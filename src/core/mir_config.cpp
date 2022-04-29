#include "mir_config.h"
#include "core/mir_config_macros.h"

namespace mir {

Configure::Configure()
{
	_SHADOW_MODE = SHADOW_MODE;
}

bool Configure::IsShadowVSM() const 
{ 
	return _SHADOW_MODE == SHADOW_VSM; 
}

}