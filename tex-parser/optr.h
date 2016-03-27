#include "gen-token.h"
#include "gen-symbol.h"
#include "trans.h"

#include "tree/tree.h"
#include "stdbool.h"

#define WC_NORMAL_LEAF      0
#define WC_WILDCD_LEAF      1
#define WC_NONCOM_OPERATOR  0
#define WC_COMMUT_OPERATOR  1

struct optr_node {
	union {
		bool         commutative;
		bool         wildcard;
	};
	enum symbol_id   symbol_id;
	enum token_id    token_id;
	uint32_t         sons;
	uint32_t         rank;
	uint8_t          br_hash;
	struct tree_node tnd;
};

struct optr_node* optr_alloc(enum symbol_id, enum token_id, bool);

struct optr_node* optr_attach(struct optr_node*, struct optr_node*);

void optr_print(struct optr_node*, FILE*);

void optr_release(struct optr_node*);

/*
uint32_t tex_tr_assign(struct tex_tr*);

uint32_t tex_tr_prune(struct tex_tr*);

void tex_tr_group(struct tex_tr*);

uint32_t tex_tr_update(struct tex_tr*);

void tex_tr_print(struct tex_tr*, FILE*);

void tex_tr_release();

struct list_it tex_tr_subpaths(struct tex_tr*, int*);

void subpaths_print(struct list_it*, FILE*);

LIST_DECL_FREE_FUN(subpaths_free);
*/
