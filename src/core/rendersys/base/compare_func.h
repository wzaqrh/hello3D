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

inline CompareFunc GetReverseZCompareFunc(CompareFunc func) 
{
	switch (func)
	{
	case kCompareLess: return kCompareGreater;
	case kCompareLessEqual: return kCompareGreaterEqual;
	case kCompareGreater: return kCompareLess;
	case kCompareGreaterEqual: return kCompareLessEqual;
	case kCompareEqual:
	case kCompareNotEqual:
	case kCompareNever:
	case kCompareAlways:
	case kCompareUnkown:
	default:
		return func;
	}
}

}