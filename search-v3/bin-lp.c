#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "common/common.h"
#include "bin-lp.h"
#include "config.h"

/*
 * Printing functions
 */
void bin_lp_brief_print(struct bin_lp blp)
{
	for (int column = 0; column < blp.n_po; column++) {
		if (column < blp.zero_pivot) printf(C_BLUE);
		else if (column >= blp.one_pivot) printf(C_GREEN);
		printf("%u ", blp.po[column]);
		printf(C_RST);
	}
	printf("\n");
}

void bin_lp_print(struct bin_lp blp, int level)
{
	printf("[binary linear programming table]  ");
	printf("(uncolored are feasible area) \n");

	printf("%*c", level * 2, ' ');
	for (int column = 0; column < blp.n_po; column++)
		printf(" po-%-3u|", blp.po[column]);
	printf("\n");

	printf("%*c", level * 2, ' ');
	for (int column = 0; column < blp.n_po; column++)
		printf("w=%-5.2f|", blp.weight[column]);
	printf("\n");

	for (int row = 0; row < blp.n_nodes; row++) {
		printf("%*c", level * 2, ' ');
		for (int column = 0; column < blp.n_po; column++) {
			if (column < blp.zero_pivot) printf(C_BLUE);
			else if (column >= blp.one_pivot) printf(C_GREEN);
			printf("%5.2f   ", blp.matrix[row * blp.width + column]);
			printf(C_RST);
		}
		printf(" (node-%d) \n", blp.node[row]);
	}
}

int bin_lp_assign(struct bin_lp *blp, int node, int po, float val)
{
	int old_val, modified = 0;
	int column = 0, row = 0;

	for (column = 0; column < blp->n_po; column++)
		if (blp->po[column] == po)
			break;

	if (column == blp->n_po) {
		blp->n_po += 1;
		modified = 2; /* new */
	}

	for (row = 0; row < blp->n_nodes; row++)
		if (blp->node[row] == node)
			break;

	if (row == blp->n_nodes) {
		blp->n_nodes += 1;
		modified = 2; /* new */
	}

	/* assignment */
	blp->po[column] = po;
	blp->node[row] = node;
	old_val = blp->matrix[row * blp->width + column];
	blp->matrix[row * blp->width + column] = val;

	if (!modified && old_val != val)
		modified = 1; /* change value */

	return modified;
}

/*
 * Allocation/Copy functions
 */
struct bin_lp bin_lp_alloc(int height, int width)
{
	struct bin_lp blp;
	blp.n_po = 0;
	blp.n_nodes = 0;
	blp.width = width;
	blp.height = height;
	blp.po   = malloc(sizeof(int) * width);
	blp.node = malloc(sizeof(int) * height);
	blp.weight = malloc(sizeof(float) * width);
	blp.matrix = malloc(sizeof(float) * height * width);
	return blp;
}

void bin_lp_free(struct bin_lp blp)
{
	free(blp.po);
	free(blp.node);
	free(blp.weight);
	free(blp.matrix);
}

void bin_lp_reset(struct bin_lp *blp)
{
	blp->n_po = 0;
	blp->n_nodes = 0;
	memset(blp->po, 0, sizeof(int) * blp->width);
	memset(blp->node, 0, sizeof(int) * blp->height);
	memset(blp->weight, 0, sizeof(float) * blp->width);
	memset(blp->matrix, 0, sizeof(float) * blp->height * blp->width);
}

static void bin_lp_copy(struct bin_lp *dst, struct bin_lp *src)
{
	dst->n_po = src->n_po;
	dst->n_nodes = src->n_nodes;
	dst->width = src->width;
	dst->height = src->height;
	dst->zero_pivot = src->zero_pivot;
	dst->one_pivot = src->one_pivot;
	memcpy(dst->po, src->po, sizeof(int) * src->width);
	memcpy(dst->node, src->node, sizeof(int) * src->height);
	memcpy(dst->weight, src->weight, sizeof(float) * src->width);
	memcpy(dst->matrix, src->matrix, sizeof(float) * src->height * src->width);
}

/*
 * Manipulation functions
 */
static float column_max(struct bin_lp *blp, int c)
{
	float max = 0;
	for (int row = 0; row < blp->n_nodes; row++)
		if (blp->matrix[row * blp->width + c] > max)
			max = blp->matrix[row * blp->width + c];
	return max;
}

static void column_swap(struct bin_lp *blp, int a, int b)
{
#ifdef DEBUG_BIN_LP
	printf("swap col %d and %d ...\n", a, b);
#endif
	float tmp;
	for (int row = 0; row < blp->n_nodes; row++) {
		tmp = blp->matrix[row * blp->width + a];
		blp->matrix[row * blp->width + a] = blp->matrix[row * blp->width + b];
		blp->matrix[row * blp->width + b] = tmp;
	}

	tmp = blp->po[a];
	blp->po[a] = blp->po[b];
	blp->po[b] = tmp;

	tmp = blp->weight[a];
	blp->weight[a] = blp->weight[b];
	blp->weight[b] = tmp;
}

static float row_weight_sum(struct bin_lp *blp, int r)
{
	float sum = 0;
	for (int column = blp->zero_pivot; column < blp->n_po; column++)
		sum += blp->matrix[r * blp->width + column];
	return sum;
}

static float objective_weight_sum(struct bin_lp *blp)
{
	float sum = 0;
	for (int column = blp->zero_pivot; column < blp->n_po; column++)
		sum += blp->weight[column];
	return sum;
}

static int row_max_column(struct bin_lp *blp, int r)
{
	int max_column = 0;
	float max = 0;
	for (int column = blp->zero_pivot; column < blp->one_pivot; column++)
		if (blp->matrix[r * blp->width + column] > max) {
			max = blp->matrix[r * blp->width + column];
			max_column = column;
		}
	return max_column;
}

static int row_min_objective_col(struct bin_lp *blp, int r)
{
	int min_column = 0;
	float min = FLT_MAX;
	for (int column = blp->zero_pivot; column < blp->one_pivot; column++)
		if (blp->matrix[r * blp->width + column] > 0 &&
		    blp->weight[column] < min) {
			min = blp->weight[column];
			min_column = column;
		}
	return min_column;
}

/*
 * Solver functions
 */
static void
bin_lp_solve_r(struct bin_lp* blp, float threshold,
               float *max, struct bin_lp *sol,
               bin_lp_upp_callbk upp, void* args, int level)
{
	float cur_max = objective_weight_sum(blp);

#ifdef DEBUG_BIN_LP
	printf("\n");
	printf("objective weight sum: %f, max=%f.\n", cur_max,
		(*max == -FLT_MAX) ? 0 : *max);
	bin_lp_print(*blp, level);
#endif

	if (cur_max <= *max) {
#ifdef DEBUG_BIN_LP
		printf("pruned.\n");
#endif
		return;
	} else if (blp->zero_pivot >= blp->one_pivot) {
#ifdef DEBUG_BIN_LP
		printf("infeasible.\n");
#endif
		return;
	}

	/* find out which row violates threshold constraints the most */
	int max_row = -1; float max_delta = 0.f;
	for (int row = 0; row < blp->n_nodes; row++) {
		float sum = row_weight_sum(blp, row);
		/* delta: more positive, more violation */
		float delta = upp(args, sum) - threshold;
#ifdef DEBUG_BIN_LP
		printf("row[%d] weight sum: %f (violate: %f)", row, sum, delta);
		printf(" upperbound=%f (threshold: %f)\n", upp(args, sum), threshold);
#endif
		/* find the greatest delta */
		if (delta > max_delta) {
			max_delta = delta;
			max_row = row;
		}
	}

	/* if this problem does not satisfy all of our constraints */
	if (max_row >= 0) {
		/* solve sub-problems */

		/* Find a column in that row to take out, heuristically, 2 methods: */
		/* [1] find a column that violates the most of our constraints */
		// int max_col = row_max_column(blp, max_row);

		/* [2] find a column that contributes the least to our objective */
		int max_col = row_min_objective_col(blp, max_row);

#ifdef DEBUG_BIN_LP
		printf("most violating row = %d, col = %d\n", max_row, max_col);
#endif
		struct bin_lp lp1, lp2;
		lp1 = bin_lp_alloc(blp->height, blp->width);
		lp2 = bin_lp_alloc(blp->height, blp->width);

		bin_lp_copy(&lp1, blp);
		bin_lp_copy(&lp2, blp);

		column_swap(&lp1, lp1.zero_pivot ++, max_col);
		bin_lp_solve_r(&lp1, threshold, max, sol, upp, args, level + 1);

		/* uncomment to track all BP branches */
//		column_swap(&lp2, -- lp2.one_pivot, max_col);
//		bin_lp_solve_r(&lp2, threshold, max, sol, upp, args, level + 1);

		bin_lp_free(lp1);
		bin_lp_free(lp2);
		return;
	}

#if 0
	printf("feasible solution.\n");
	bin_lp_brief_print(*blp);
#endif

#ifdef DEBUG_BIN_LP
	printf(C_RED "%*cUpdate result, max: %f\n" C_RST, level * 2, ' ', cur_max);
#endif
	*max = cur_max;
	bin_lp_copy(sol, blp);
}

int
bin_lp_solve(struct bin_lp* blp, float threshold,
             bin_lp_upp_callbk upp, void* args)
{
	/* initialize */
	blp->zero_pivot = 0;
	blp->one_pivot = blp->n_po;

	for (int column = 0; column < blp->n_po; column++) {
		float max = column_max(blp, column);

		/* violates constraints anyway, take it out */
		if (upp(args, max) > threshold) {
			column_swap(blp, blp->zero_pivot ++, column);
#ifdef DEBUG_BIN_LP
			bin_lp_print(*blp, 0);
#endif
		}
	}

	float max = -FLT_MAX; /* branch-and-cut upperbound */
	bin_lp_solve_r(blp, threshold, &max, blp, upp, args, 0);
	return blp->zero_pivot;
}
