#pragma once

namespace mir {

enum CompareFunc
{
	kCompareUnkown = 0,
	kCompareNever = 1,
	kCompareLess = 2,
	kCompareEqual = 3,
	kCompareLessEqual = 4,
	kCompareGreater = 5,
	kCompareNotEqual = 6,
	kCompareGreaterEqual = 7,
	kCompareAlways = 8
};

}