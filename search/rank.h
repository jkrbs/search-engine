#pragma once
#include <stdint.h>
#include "minheap.h"
struct rank_hit {
	doc_id_t    docID;
	float       score;
