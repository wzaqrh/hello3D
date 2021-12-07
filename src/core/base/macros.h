#pragma once

#define NULLABLE(CLS, MEM) (CLS ? CLS->MEM : nullptr)