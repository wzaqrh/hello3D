#include "mir_config.h"
#include "core/mir_config_macros.h"

namespace mir {

Configure::Configure()
{
	_SHADOW_MODE = SHADOW_MODE;
	_REVERSE_Z = REVERSE_Z;
	_COLORSPACE = COLORSPACE;
}

bool Configure::IsShadowVSM() const 
{ 
	return _SHADOW_MODE == SHADOW_VSM; 
}

bool Configure::IsGammaSpace() const
{
	return _COLORSPACE == COLORSPACE_GAMMA;
}

}