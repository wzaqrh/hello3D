#pragma once

namespace mir {

enum PrimitiveTopology 
{
	kPrimTopologyUnkown = 0,
	kPrimTopologyPointList = 1,
	kPrimTopologyLineList = 2,
	kPrimTopologyLineStrip = 3,
	kPrimTopologyTriangleList = 4,
	kPrimTopologyTriangleStrip = 5,
	kPrimTopologyLineListAdj = 10,
	kPrimTopologyLineStripAdj = 11,
	kPrimTopologyTriangleListAdj = 12,
	kPrimTopologyTriangleStripAdj = 13,
};

}