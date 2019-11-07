#include <math.h>
#include "common/common.h"
#include "math-score.h"

const float eta = MATH_SCORE_ETA;

#define DOC_LEN_PENALTY ((1.f - eta) + eta * (1.f / logf(1.f + dn)))

void math_score_precalc(struct math_score_factors *msf)
{
	float dn;

	dn = 1.f;
	msf->upp_sf = 1.f * DOC_LEN_PENALTY;

	dn = (float)MAX_MATCHED_PATHS;
	msf->low_sf = .5f * DOC_LEN_PENALTY;
}

float math_score_ipf(float N, float pf)
{
	return logf(N / pf);
}

float math_score_calc(struct math_score_factors *msf)
{
	float dn = msf->doc_lr_paths;
	float sf = msf->symbol_sim * DOC_LEN_PENALTY;

	return msf->struct_sim * sf;
}

float math_score_upp(void *msf_, float sum_ipf)
{
	P_CAST(msf, struct math_score_factors, msf_);
	return sum_ipf * msf->upp_sf;
}

float math_score_low(void *msf_, float sum_ipf)
{
	P_CAST(msf, struct math_score_factors, msf_);
	return sum_ipf * msf->low_sf;
}
