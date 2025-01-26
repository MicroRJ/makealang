
int getkey(int key);
int random();
int *array(int size, int count);
int set(int x, int y, int value);
int get(int x, int y);

// paddle1 	:= 0
// paddle2 	:= 0
// x  := 0
// y  := 0
// xv := 0
// yv := 0

// while {
// 	set(0,paddle1+0,1)
// 	set(0,paddle1+1,1)
// 	set(0,paddle1+2,1)
// 	if get(x+xv,y+yv)==1 ? {
// 		xv=-xv
// 	} else {
// 		x=x+xv
// 		y=y+yv
// 	}
// 	set(x,y,3)
// }


// snake  		:= new_array(100,4)
// snakel 		:= 0
// applx  		:= -1
// spawn_apple := 1
// apply  		:= 0
// dirx   		:= 0
// diry   		:= 1
// movetimer=30
// while {
// 	keyx := is_key_pressed('D')-is_key_pressed('A')
// 	keyy := is_key_pressed('S')-is_key_pressed('W')
// 	if keyx!=0 ? {
// 		dirx=keyx
// 	}
// 	if keyy!=0 ? {
// 		diry=keyy
// 	}
// 	if movetimer==0 ? {
// 		grid[snake[(snakel-1)*4+0]+snake[(snakel-1)*4+1]*16]
// 		// shift array to the left
// 		i := 1
// 		while i < snakel ? {
// 			snake[i*4+0]=snake[(i-1)*4+0]
// 			snake[i*4+1]=snake[(i-1)*4+1]
// 		}
// 		snake[0]=snake[0]+dirx
// 		snake[1]=snake[1]+diry
// 	}else{
// 		movetimer=movetimer-1
// 	}
// 	if snake[0]==applx ? {
// 		if snake[1]==apply ? {
// 			spawn_apple=1
// 			snakel=snakel+1
// 		}
// 	}
// 	if spawn_apple ? {
// 		applx=random()%16
// 		apply=random()%16
// 	}
// 	i := 0
// 	while i < snakel ? {
// 		x := snake[i*4 + 0]
// 		y := snake[i*4 + 1]
// 		draw(x,y,1)
// 	}
// }



typedef treeT *treeID;

typedef enum{
	BC_LINT,
	BC_MOVE,
	// '+','\\','*'
} byteKi;
typedef struct {
	short o,x,y,_;
} byteC;

typedef enum{
	NONE=0,
	SETMEM,
}treeKi;

typedef char *treeSL;

struct treeT {
	treeKi kind;
	treeSL text;
	treeID prox;
	union{
		struct{
			treeID x,y,z;
		};
		char   *expr_str;
		elf_i64 expr_int;
		elf_f64 expr_num;
	};
	int _mem;
};

typedef struct{
	char *name;
	char *text;
	treeID  ti;
	int  flags;
}entityT;

entityT entity_stack[128];
entityT entity_index;
int 	  scope;


enum{tok_none=0,tok_integer,tok_word};
typedef struct{
	int 		  type;
	char 		 *text;
	union{
		char *string;
		int  integer;
	};
}tokenT;

char *schr;
tokenT tok;
tokenT tok2;
char general_memory[10000];
int general_index;
byteC bytec_memory[1000];
int bytec_index;
treeID tree_memory[1000];
int tree_index;


void *gnew(int size){
	assert(general_index+size<=sizeof(general_memory));
	void*ptr=general_memory+general_index;
	general_index+=size;
	return ptr;
}
void gget(){ return general_memory+general_index; }
void gput(int chr){ *gnew(1)=chr; }
void gend(int chr){ *gnew(1)=0;   }


void gettok(){
	tokenT t={};
	t.text=schr;
	switch(*schr){
		case '0'...'9':{
			t.type=tok_integer;
			t.integer=0;
			int base=10;
			do {
				t.integer=t.integer*base+*schr++;
			} while(*schr>='0'&&*schr<='9');
		}break;
		case 'a'...'z': case 'A'...'Z': case '_': {
			t.type=tok_word;
			t.string=gget();
			do {
				gput(*schr++);
			}while((*schr>='a'&&*schr<='z')
			||		 (*schr>='A'&&*schr<='Z')
			||     (*schr>='0'&&*schr<='9')
			||     (*schr=='_'));
			gend();
		}break;
		default:{
			t.type=*schr;
		}break;
	}
	tok=tok2;
	tok2=t;
}

int find(char *name){
	int id=-1;
	for(int i=entity_index-1;i>=0;i--){
		if(!strcmp(entities[i].name,name)){
			id=i;
			break;
		}
	}
	return id;
}

treeID tnew(treeKi kind,char *text){
	ASSERT(tree_index<_countof(tree_memory));
	treeID tree = tree_memory[tree_index++];
	tree->kind=kind;
	tree->text=text;
	return tree;
}

treeID parse_unary(){
	treeID v=0;
	switch(tok.type){
		case tok_integer:{
			v=tnew('i',tok.text);
			v->expr_int=tok.integer;
		}break;
		case tok_word:{
			int i=find(tok.string);
			if(i!=-1){
				v=entities[i].ti;
			}else{
				v=tnew('g',tok.text);
			}
		}break;
	}
	return v;
}
treeID parse_subexpr(int rank){
	treeID v;
	v=parse_unary();
	int prec=0;
	do{
		switch(tok.type){
			case '+': prec=1; break;
			case '*': prec=2; break;
		}
		v=parse_subexpr(prec);
	}while(prec>rank);
	return v;
}
treeID parse_expr(){
	treeID v=0;
	switch(tok.type){
		case ',': case ')': {
			goto esc;
		} break;
		default: {
			parse_subexpr(0);
		} break;
	}
	esc:
	return v;
}
treeID parse_stat(){
	switch(tok.type){
		case '}': {
			return 0;
		}
		default: {
			if (tok.type==tok_word && tok2.type==':') {
				entityT *ent = entity_stack[entity_index++];
				// homework! look for already declared entities
				ent->name=tok.string;
				take_tok('=');
				ent->ti=parse_expr();
			} else {
				parse_expr();
			}
		} break;
	}
}
treeID parse_body(){
	int scope=entity_index;
	while (parse_stat());
	entity_index=scope;
}
treeID parse_block(){
	take_tok('{');
	parse_body();
	take_tok('}');
}
treeID parse(){
	return parse_block();
}


int memstate;
void setmem(treeID ti){ ti->_mem=1+memstate++; }
void getmem(treeID ti){ return ti->_mem;   }

void codegen(treeID ti){
	switch(ti->kind){
		case SETMEM:{
			setmem(ti);
		}break;
		case '*': case '+':{
			int rx,ry,ro;

			int mem=memstate;
			rx=codegen(ti->x);
			ry=codegen(ti->y);
			memstate=mem;

			ro=setmem(ti);
			geno(ti->kind,ro,rx,ry);
		}break;
	}
}


void run(){
	for(;;){
		switch(iptr->o){
			case '+':{
				stack[iptr->z]=stack[iptr->x].as_int+stack[iptr->y].as_int;
			}break;
			case '*':{
				stack[iptr->z]=stack[iptr->x].as_int*stack[iptr->y].as_int;
			}break;
			default:{
				goto esc;
			};
		}
	}

	esc: ;
}