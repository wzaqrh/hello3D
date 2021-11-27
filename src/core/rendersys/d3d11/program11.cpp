#include "core/rendersys/d3d11/program11.h"

namespace mir {

Program11::Program11()
{
}

void Program11::SetVertex(VertexShader11Ptr pVertex)
{
	mVertex = pVertex;
}

void Program11::SetPixel(PixelShader11Ptr pPixel)
{
	mPixel = pPixel;
}

}