#pragma once

#define AS_CONST_REF(TYPE, V)		*(const TYPE*)(&V)

#define NULLABLE(CLS, MEM)			(CLS ? CLS->MEM : nullptr)
#define IF_AND_OR(COND, AND, OR)	(COND ? AND : OR)
#define IF_AND_NULL(COND, AND)		IF_AND_OR(COND, AND, nullptr)
#define IF_OR(COND, OR)				IF_AND_OR(COND, COND, OR)

#define VECTOR_AT_OR_NULL(VECTOR, POS) IF_AND_OR(POS < VECTOR.size(), VECTOR[POS], nullptr)